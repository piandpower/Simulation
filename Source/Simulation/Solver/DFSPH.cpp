#include "DFSPH.h"
#include "Simulator.h"

UDFSPHSolver::~UDFSPHSolver()
{
}

UDFSPHSolver * UDFSPHSolver::CreateDFSPHSolver(TArray<UAcceleration*> accelerations,
	float desiredAverageVelocityDivergenceError,
	float desiredIndividualVelocityDivergenceError,
	float desiredAverageDensityError,
	float desiredIndividualDensityError,
	int minIterationVelocity,
	int maxIterationVelocity,
	int minIterationDensity,
	int maxIterationDensity,
	float jacobiFactor,
	UBoundaryPressure * boundaryPressure)
{
	UDFSPHSolver * dfsphsolver = NewObject<UDFSPHSolver>();
	dfsphsolver->SolverType = ESolverMethod::DFSPH;

	dfsphsolver->DesiredAverageVelocityDivergenceError = desiredAverageVelocityDivergenceError;
	dfsphsolver->DesiredIndividualVelocityDivergenceError = desiredIndividualVelocityDivergenceError;
	dfsphsolver->DesiredAverageDensityError = desiredAverageDensityError;
	dfsphsolver->DesiredIndividualDensityError = desiredIndividualDensityError;
	dfsphsolver->MinIterationVelocity = minIterationVelocity;
	dfsphsolver->MaxIterationVelocity = maxIterationVelocity;
	dfsphsolver->MinIterationDensity = minIterationDensity;
	dfsphsolver->MaxIterationDensity = maxIterationDensity;
	dfsphsolver->JacobiFactor = jacobiFactor;
	dfsphsolver->Accelerations = accelerations;
	dfsphsolver->BoundaryPressureComputer = boundaryPressure;

	// prevent garbage collection
	dfsphsolver->AddToRoot();

	return dfsphsolver;
}

void UDFSPHSolver::Step()
{
	FDateTime totalStartTime = FDateTime::UtcNow();

	InitializePeriodicCondition();

	// Reserve space for all particle attirbutes required by the solver
	FitSolverAttributeArray();

	// reset Accelerations to zero
	ClearAcceleration();

	// Find Neighbors with specified NeighborhoodSearch method
	FindNeighbors();

	// Compute Densities
	FDateTime densityCalculationStartTime = FDateTime::UtcNow();
	ComputeDensitiesExplicit();
	ComputeAverageDensityError();
	ComputationTimes.DensityComputationTime = (FDateTime::UtcNow() - densityCalculationStartTime).GetTotalSeconds();

	// Compute the maximum possible time step at current velocities
	MaxTimeStep();

	// Applies all behaviour defined by scripted volumes before the solver starts. Some of them block the solvers accelerations
	ApplyScriptedVolumesBeforeIntegration();

	FDateTime pressureStartTime = FDateTime::UtcNow();

	// Compute pressure accelerations based on velocity divergence
	ComputeDiagonalElement(false);
	ComputeSourceTermsVelocityDivergence();
	InitializePressureValues(false);
	UpdatePressureAcceleration();

	int iterationCount = 1;
	// Do at least 1 iteration, max maxIterationVelocity iterations
	while (!CheckAveragePredictedVelocityDivergenceError() && iterationCount < MaxIterationVelocity || iterationCount < MinIterationVelocity) {
		ComputePressureAccelerationCorrection(false);
		RelaxedJacobiUpdatePressure(false);
		UpdatePressureAcceleration();
		iterationCount++;
	}
	LastIterationCount = iterationCount;

	ComputationTimes.PressureComputationTime = (FDateTime::UtcNow() - pressureStartTime).GetTotalSeconds();

	// Compute intermediate velocities, considering only pressure accelerations from divergence-free solving
	ComputeIntermediateVelocities(true);

	// Set accelerations to 0
	ClearAcceleration();

	// Compute all non-pressure accelerations
	ComputeNonPressureAccelerations();

	// compute v** (Intermediate velocities)
	ComputeIntermediateVelocities(false);

	// compute rho** (Intermediate densities)
	ComputePredictedDensities();

	pressureStartTime = FDateTime::UtcNow();

	// Compute pressure accelerations based on density error
	ComputeSourceTermsDensity();
	InitializePressureValues(true);
	UpdatePressureAcceleration();
	iterationCount = 0;
	// Do at least one iteration, max maxIterationDensity iterations
	while (!CheckAveragePredictedDensityError() && iterationCount < MaxIterationDensity || iterationCount < MinIterationDensity) {
		ComputePressureAccelerationCorrection(false);
		RelaxedJacobiUpdatePressure(true);
		UpdatePressureAcceleration();
		iterationCount++;
	}
	LastIterationCount += iterationCount;

	ComputationTimes.PressureComputationTime += (FDateTime::UtcNow() - pressureStartTime).GetTotalSeconds();

	// Add the pressure acceleration and integrate
	FDateTime integrationStartTime = FDateTime::UtcNow();

	// Applies the pressure values from the density invariance term to the intermediate velocities resulting in final velocities and performing a position update
	IntegrateEulerCromerWithPressureAcceleration();

	ComputationTimes.IntegrationTime = (FDateTime::UtcNow() - integrationStartTime).GetTotalSeconds();

	// Applies all behaviour defined by scripted volumes
	ApplyScriptedVolumesAfterIntegration();

	// measure time the whole step took
	ComputationTimes.TotalTime = (FDateTime::UtcNow() - totalStartTime).GetTotalSeconds();
}

void UDFSPHSolver::ComputeSolverStatistics(bool computeSolverStats)
{
	if (computeSolverStats) {
		OldTimesteps.push_back(CurrentTimestep);
		OldComputationTimesPerStep.push_back(ComputationTimes.TotalTime);
		OldIterationCounts.push_back(GetLastIterationCount());
		OldAverageDensityErrors.push_back(GetLastAverageDensityError());
		double kineticEnergy = 0.0;
		for (UFluid * fluid : GetParticleContext()->GetFluids()) {
			kineticEnergy += fluid->CalculateKineticEnergy();
		}
		OldKineticEnergies.push_back(kineticEnergy);
	}
}

void UDFSPHSolver::FitSolverAttributeArray()
{
	Attributes.resize(GetParticleContext()->GetFluids().size());

	for (int i = 0; i < GetParticleContext()->GetFluids().size(); i++) {
		UFluid* fluid = GetParticleContext()->GetFluids()[i];
		Attributes[i].resize(fluid->Particles->size());
	}
}

double UDFSPHSolver::MaxTimeStep()
{
	// if there is a fixed next timestep because of incoming frame-recording, then simply take last timestep
	if (FixedNextTimestep) {
		FixedNextTimestep = false;
		return CurrentTimestep;
	}

	// get the max velocity of all particles in parallel
	double maxVelocity = 0.0;

	for (UFluid * fluid : GetParticleContext()->GetFluids()) {
		if (fluid->Particles->size() > 0)
			maxVelocity = ParallelMax<Particle, double>(*fluid->Particles, [](const Particle & particle) -> double {return particle.Velocity.Size(); });

	}

	// Calculate timestep as conditioned by the CFL number (timestep factor)
	if (maxVelocity <= 0) {
		CurrentTimestep = Simulator->GetMaxTimestep();
	}
	else {
		CurrentTimestep = std::min(Simulator->GetCFLNumber() * Simulator->GetParticleContext()->GetParticleDistance() / maxVelocity, Simulator->GetMaxTimestep());
	}

	// sometimes volumes need to restrict the timestep
	for (AScriptedVolume * scriptedVolume : Volumes) {
		CurrentTimestep = std::min(CurrentTimestep, scriptedVolume->MaxTimeStep(Simulator->GetCFLNumber(), *Simulator->GetParticleContext()));
	}

	CurrentTimestep = std::max(CurrentTimestep, Simulator->GetMinTimestep());

	// Check if CurrentTimestep needs to be smaller because of incoming new recording-frame
	if (Simulator->IsTimestepAdaptiveToFramerate() && CurrentTimestep * 2 + Simulator->GetSimulatedTime() > Simulator->GetRecordManager()->GetNextRecordTime()) {
		// next timestep should be the same as this one
		FixedNextTimestep = true;
		CurrentTimestep = (Simulator->GetRecordManager()->GetNextRecordTime() - Simulator->GetSimulatedTime()) / 2;
	}

	return CurrentTimestep;
}

void UDFSPHSolver::ComputeNonPressureAccelerations()
{
	FDateTime startTime = FDateTime::UtcNow();
	for (UAcceleration * acceleration : Accelerations) {
		acceleration->ApplyAcceleration(GetSimulator()->GetParticleContext());
	}

	// In the special case of a periodic setting update pressure of ghost particles
	if (GetParticleContext()->GetPeriodicCondition() != nullptr) {
		GetParticleContext()->GetPeriodicCondition()->UpdateGhostParticleAcceleration();
	}

	ComputationTimes.AccelerationComputationTime = (FDateTime::UtcNow() - startTime).GetTotalSeconds();
}

void UDFSPHSolver::ComputeIntermediateVelocities(bool overrideOldIntermediateVelocity)
{
	if (overrideOldIntermediateVelocity) {
		for (UFluid * fluid : GetParticleContext()->GetFluids()) {
			ParallelFor(fluid->Particles->size(), [&](int32 i) {
				Particle& f = fluid->Particles->at(i);
				Attributes[*fluid][i].IntermediateVelocity = f.Velocity + CurrentTimestep * f.Acceleration;
			});
		}
	}
	else {
		for (UFluid * fluid : GetParticleContext()->GetFluids()) {
			ParallelFor(fluid->Particles->size(), [&](int32 i) {
				Particle& f = fluid->Particles->at(i);
				Attributes[*fluid][i].IntermediateVelocity += CurrentTimestep * f.Acceleration;
			});
		}
	}

}

void UDFSPHSolver::ComputeSourceTermsVelocityDivergence()
{
	for (UFluid * fluid : GetParticleContext()->GetFluids()) {
		ParallelFor(fluid->Particles->size(), [&](int32 i) {
			Particle& f = fluid->Particles->at(i);

			// in DFSPH velocity divergence is only solved if neighborhood is full enough
			if (!HasEnoughNeighbors(f)) {
				Attributes[*fluid][i].SourceTerm = 0.0;
				return;
			}

			double velocityDivergence = 0;

			for (const Particle& ff : f.FluidNeighbors) {
				velocityDivergence += ff.Mass / f.Density * (ff.Velocity - f.Velocity) * GetKernel()->ComputeKernelDerivative(f, ff);
			}
			for (const Particle& fb : f.StaticBorderNeighbors) {
				// neighbor velocity should be { 0, 0, 0 } for static borders
				velocityDivergence += fb.Mass / f.Density * (-f.Velocity) * GetKernel()->ComputeKernelDerivative(f, fb);
			}

			Attributes[*fluid][i].SourceTerm = velocityDivergence * GetCurrentTimestep();
		});
	}

}

void UDFSPHSolver::ComputeSourceTermsDensity()
{
	for (UFluid * fluid : GetParticleContext()->GetFluids()) {
		ParallelFor(fluid->Particles->size(), [&](int32 i) {
			Particle& f = fluid->Particles->at(i);
			Attributes[*fluid][i].SourceTerm = f.Fluid->GetRestDensity() - Attributes[*fluid][i].IntermediateDensity;
		});
	}
}

void UDFSPHSolver::ComputeDiagonalElement(bool clampAtZero) {
	for (UFluid * fluid : GetParticleContext()->GetFluids()) {
		ParallelFor(fluid->Particles->size(), [&](int32 i) {
			Particle& f = fluid->Particles->at(i);
			// we don't need the diagonal element for particles with no predicted density error
			if (clampAtZero && Attributes[*fluid][i].SourceTerm >= 0) {
				// return only exits Parallel call
				return;
			}

			Vector3D innersum = { 0, 0, 0 };

			for (const Particle& ff : f.FluidNeighbors) {
				innersum -= ff.Mass / pow(f.Fluid->GetRestDensity(), 2) * GetKernel()->ComputeKernelDerivative(f, ff);
			}

			for (const Particle& fb : f.StaticBorderNeighbors) {
				innersum -= 2 * fb.Border->BorderStiffness * fb.Mass / pow(f.Fluid->GetRestDensity(), 2) * GetKernel()->ComputeKernelDerivative(f, fb);
			}


			// first row of equation
			double firstline = 0;
			for (const Particle& ff : f.FluidNeighbors) {
				firstline += ff.Mass * innersum * GetKernel()->ComputeKernelDerivative(f, ff);
			}

			// second row of equation
			double secondline = 0;
			for (const Particle& ff : f.FluidNeighbors) {
				secondline += ff.Mass * f.Mass / pow(f.Fluid->GetRestDensity(), 2) * GetKernel()->ComputeKernelDerivative(ff, f) * GetKernel()->ComputeKernelDerivative(f, ff);
			}

			// third row of equation
			double thirdline = 0;
			for (const Particle& fb : f.StaticBorderNeighbors) {
				thirdline += fb.Mass * innersum * GetKernel()->ComputeKernelDerivative(f, fb);
			}

			Attributes[*fluid][i].Aff = pow(CurrentTimestep, 2) * (firstline + secondline + thirdline);
		});
	}
}

void UDFSPHSolver::InitializePressureValues(bool clampAtZero) {
	// fluid pressure
	for (UFluid * fluid : GetParticleContext()->GetFluids()) {
		ParallelFor(fluid->Particles->size(), [&](int32 i) {
			Particle& f = fluid->Particles->at(i);

			if (std::abs(Attributes[*fluid][i].Aff) > DBL_EPSILON) {
				// this corresponds to a jacobi-step with pressure = 0
				if (clampAtZero) {
					f.Pressure = std::max(JacobiFactor * Attributes[*fluid][i].SourceTerm / Attributes[*fluid][i].Aff, 0.0);
				}
				else {
					f.Pressure = JacobiFactor * Attributes[*fluid][i].SourceTerm / Attributes[*fluid][i].Aff;
				}
			}
			else {
				f.Pressure = 0.0;
			}


		});
	}

	// boundary pressure
	GetBoundaryPressure()->ComputeAllPressureValues(GetSimulator()->GetParticleContext(), GetKernel());

	// In the special case of a periodic setting update pressure of ghost particles
	if (GetParticleContext()->GetPeriodicCondition() != nullptr) {
		GetParticleContext()->GetPeriodicCondition()->UpdateGhostParticlePressure();
	}
}

void UDFSPHSolver::UpdatePressureAcceleration() {
	for (UFluid * fluid : GetParticleContext()->GetFluids()) {
		ParallelFor(fluid->Particles->size(), [&](int32 i) {
			Particle& f = fluid->Particles->at(i);
			Vector3D sum = { 0, 0, 0 };
			for (const Particle& ff : f.FluidNeighbors) {
				sum -= ff.Mass * (f.Pressure / pow(f.Fluid->GetRestDensity(), 2) + ff.Pressure / pow(ff.Fluid->GetRestDensity(), 2)) * GetKernel()->ComputeKernelDerivative(f, ff);
			}
			for (const Particle& fb : f.StaticBorderNeighbors) {
				// Pressure values are mirrored from fluid particles to boundary particles
				sum -= fb.Border->BorderStiffness * fb.Mass *
					(f.Pressure / pow(f.Fluid->GetRestDensity(), 2) +
						BoundaryPressureComputer->GetPressureValue(fb, f) / pow(f.Fluid->GetRestDensity(), 2))
					* GetKernel()->ComputeKernelDerivative(f, fb);
			}
			f.Acceleration = sum;
		});
	}

	// In the special case of a periodic setting update acceleration of ghost particles
	if (GetParticleContext()->GetPeriodicCondition() != nullptr) {
		GetParticleContext()->GetPeriodicCondition()->UpdateGhostParticleAcceleration();
	}
}


void UDFSPHSolver::ComputePressureAccelerationCorrection(bool clampToZeroIncompleteNeighborhood)
{
		if (clampToZeroIncompleteNeighborhood) {
		for (UFluid * fluid : GetParticleContext()->GetFluids()) {
			ParallelFor(fluid->Particles->size(), [&](int32 i) {
				Particle& f = fluid->Particles->at(i);
				Attributes[*fluid][i].Ap = 0;
				for (const Particle& ff : f.FluidNeighbors) {
					Attributes[*fluid][i].Ap += pow(CurrentTimestep, 2) * ff.Mass * (f.Acceleration - ff.Acceleration) * GetKernel()->ComputeKernelDerivative(f, ff);
				}
				for (const Particle& fb : f.StaticBorderNeighbors) {
					Attributes[*fluid][i].Ap += pow(CurrentTimestep, 2) * fb.Mass * f.Acceleration * GetKernel()->ComputeKernelDerivative(f, fb);
				}
			});
		}
	}
	// if incomplete neighborhood doesn't matter (for example at density error pressure computation) we compute Ap the normal way
	else {
		for (UFluid * fluid : GetParticleContext()->GetFluids()) {
			ParallelFor(fluid->Particles->size(), [&](int32 i) {
				Particle& f = fluid->Particles->at(i);

				Attributes[*fluid][i].Ap = 0;
				for (const Particle& ff : f.FluidNeighbors) {
					Attributes[*fluid][i].Ap += pow(CurrentTimestep, 2) * ff.Mass * (f.Acceleration - ff.Acceleration) * GetKernel()->ComputeKernelDerivative(f, ff);
				}
				for (const Particle& fb : f.StaticBorderNeighbors) {
					Attributes[*fluid][i].Ap += pow(CurrentTimestep, 2) * fb.Mass * f.Acceleration * GetKernel()->ComputeKernelDerivative(f, fb);
				}
			});
		}
	}

}

void UDFSPHSolver::RelaxedJacobiUpdatePressure(bool clampAtZero)
{
	// fluid pressure
	for (UFluid * fluid : GetParticleContext()->GetFluids()) {
		ParallelFor(fluid->Particles->size(), [&](int32 i) {
			Particle& f = fluid->Particles->at(i);

			// in DFSPH velocity divergence is only solved if neighborhood is full enough
			if (!HasEnoughNeighbors(f)) {
				return;
			}

			if (std::abs(Attributes[*fluid][i].Aff) > DBL_EPSILON) {

				// update pressure only if Aff is not 0 (if Aff = 0, then there are no neighbors)
				if (clampAtZero) {
					if (Attributes[*fluid][i].SourceTerm < 0)
						f.Pressure = std::max(f.Pressure + JacobiFactor * (Attributes[*fluid][i].SourceTerm - Attributes[*fluid][i].Ap) / Attributes[*fluid][i].Aff, 0.0);
				}
				else {
					f.Pressure = f.Pressure + JacobiFactor * (Attributes[*fluid][i].SourceTerm - Attributes[*fluid][i].Ap) / Attributes[*fluid][i].Aff;
				}
			}
			else {
				f.Pressure = 0.0;
			}
		});
	}

	// boundary pressure
	GetBoundaryPressure()->ComputeAllPressureValues(GetSimulator()->GetParticleContext(), GetKernel());

	// In the special case of a periodic setting update pressure of ghost particles
	if (GetParticleContext()->GetPeriodicCondition() != nullptr) {
		GetParticleContext()->GetPeriodicCondition()->UpdateGhostParticlePressure();
	}
}

void UDFSPHSolver::ComputePredictedDensities()
{
	for (UFluid * fluid : GetParticleContext()->GetFluids()) {
		ParallelFor(fluid->Particles->size(), [&](int32 i) {
			Particle& f = fluid->Particles->at(i);

			double velocityDivergence = 0.0;

			for (FluidNeighbor& ff : f.FluidNeighbors) {
				velocityDivergence += ff.GetParticle()->Mass * (Attributes[*ff.GetFluid()][ff].IntermediateVelocity - Attributes[*fluid][i].IntermediateVelocity) * GetKernel()->ComputeKernelDerivative(f.Position, ff.GetParticle()->Position);
			}
			for (const Particle& fb : f.StaticBorderNeighbors) {
				velocityDivergence += fb.Border->BorderDensityFactor * fb.Mass * (fb.Velocity - Attributes[*fluid][i].IntermediateVelocity) * GetKernel()->ComputeKernelDerivative(f, fb);
			}
			Attributes[*fluid][i].IntermediateDensity = f.Density - GetCurrentTimestep() * velocityDivergence;
		});
	}
}


void UDFSPHSolver::IntegrateEulerCromerWithPressureAcceleration() {
	for (UFluid * fluid : GetParticleContext()->GetFluids()) {
		ParallelFor(fluid->Particles->size(), [&](int32 i) {
			Particle& f = fluid->Particles->at(i);

			if (!f.IsScripted) {
				// acceleration should only contain pressure acceleration
				f.Velocity = Attributes[*fluid][i].IntermediateVelocity + CurrentTimestep * f.Acceleration;
				f.Position = f.Position + CurrentTimestep * f.Velocity;
			}
		});
	}

	// In the special case of a periodic setting update positions
	if (GetParticleContext()->GetPeriodicCondition() != nullptr) {
		GetParticleContext()->GetPeriodicCondition()->UpdateGhostParticleAttributes(true);
	}
}

bool UDFSPHSolver::CheckAveragePredictedVelocityDivergenceError()
{
	// compute predicted density errors and sum them
	double divergenceSum = 0.0;
	int numParticles = 0;
	for (UFluid * fluid : GetParticleContext()->GetFluids()) {

		// check individual particle velocity error
		if (ParallelExists<DFSPHParticleAttributes>(Attributes[*fluid], [this](const DFSPHParticleAttributes& attributes) {return (attributes.Ap - attributes.SourceTerm) > DesiredIndividualVelocityDivergenceError; })) {
			return false;
		}

		// check average particle velocity error
		divergenceSum += ParallelSum<DFSPHParticleAttributes, double>(Attributes[*fluid], [](const DFSPHParticleAttributes& attributes) { return std::abs(attributes.Ap - attributes.SourceTerm); });
		numParticles += fluid->Particles->size();
	}

	double averageDivergenceError = divergenceSum / numParticles;

	return averageDivergenceError <= DesiredAverageVelocityDivergenceError;
}

bool UDFSPHSolver::HasEnoughNeighbors(const Particle & f) const
{
	switch (GetSimulator()->GetDimensionality()) {
	case One:
		if (f.FluidNeighbors.size() + f.StaticBorderNeighbors.size() < 1) {
			return false;
		}
		break;
	case Two:
		if (f.FluidNeighbors.size() + f.StaticBorderNeighbors.size() < 9) {
			return false ;
		}
		break;
	case Three:
		if (f.FluidNeighbors.size() + f.StaticBorderNeighbors.size() < 20) {
			return false;
		}
		break;
	}
	return true;
}

bool UDFSPHSolver::CheckAveragePredictedDensityError()
{
	// compute predicted density errors and sum them
	double densitySum = 0.0;
	int numParticles = 0;
	for (UFluid * fluid : GetParticleContext()->GetFluids()) {

		// check individual particle density error
		if (ParallelExists<DFSPHParticleAttributes>(Attributes[*fluid], [this](const DFSPHParticleAttributes& attributes) {return (attributes.Ap - attributes.SourceTerm) > DesiredIndividualDensityError; })) {
			return false;
		}

		densitySum += ParallelSum<DFSPHParticleAttributes, double>(Attributes[*fluid], [](const DFSPHParticleAttributes& attributes) { return std::max(attributes.Ap - attributes.SourceTerm, 0.0); }) / fluid->GetRestDensity();
		numParticles += fluid->Particles->size();
	}

	double averageDensityError = densitySum / numParticles;

	return averageDensityError <= DesiredAverageDensityError;
}