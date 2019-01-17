#include "IISPH.h"
#include "Simulator.h"

UIISPHSolver::~UIISPHSolver()
{
}

UIISPHSolver * UIISPHSolver::CreateIISPHSolver(TArray<UAcceleration*> accelerations,
	float desiredAverageDensityError,
	float desiredIndividualDensityError,
	int minIteration,
	int maxIteration,
	float jacobiFactor,
	UBoundaryPressure * boundaryPressure,
	UPressureGradient * pressureGradient)
{
	UIISPHSolver * iisphsolver = NewObject<UIISPHSolver>();
	iisphsolver->SolverType = ESolverMethod::IISPH;

	iisphsolver->DesiredAverageDensityError = desiredAverageDensityError;
	iisphsolver->DesiredIndividualDensityError = desiredIndividualDensityError;
	iisphsolver->MinIteration = minIteration;
	iisphsolver->MaxIteration = maxIteration;
	iisphsolver->JacobiFactor = jacobiFactor;
	iisphsolver->Accelerations = accelerations;
	iisphsolver->BoundaryPressureComputer = boundaryPressure;
	iisphsolver->PressureGradientComputer = pressureGradient;


	// prevent garbage collection
	iisphsolver->AddToRoot();

	return iisphsolver;
}

void UIISPHSolver::Step()
{
	FDateTime totalStartTime = FDateTime::UtcNow();
		
	FitSolverAttributeArray();
	InitializePeriodicCondition();
	ClearAcceleration();
	FindNeighbors();

	FDateTime densityCalculationStartTime = FDateTime::UtcNow();
	ComputeDensitiesExplicit();
	ComputeAverageDensityError();
	ComputationTimes.DensityComputationTime = (FDateTime::UtcNow() - densityCalculationStartTime).GetTotalSeconds();

	ComputeNonPressureAccelerations();

	MaxTimeStep();

	ApplyScriptedVolumesBeforeIntegration();

	// Compute velocities, considering only non-pressure forces
	ComputeIntermediateVelocities();
	
	// Compute pressure accelerations
	FDateTime pressureStartTime = FDateTime::UtcNow();
	ComputeSourceTerms();
	ComputeDiagonalElement();
	InitializePressureValues(true);
	UpdatePressureAcceleration();
	int iterationCount = 1;

	// Do at least 2 iterations, max maxIteration iterations
	while (!CheckAveragePredictedDensityError() && iterationCount < MaxIteration || iterationCount < MinIteration) {
		ComputePressureAccelerationCorrection();
		RelaxedJacobiUpdatePressure();
		UpdatePressureAcceleration();
		iterationCount++;
	}
	LastIterationCount = iterationCount;

	ComputationTimes.PressureComputationTime = (FDateTime::UtcNow() - pressureStartTime).GetTotalSeconds();

	// Add the pressure acceleration and integrate
	FDateTime integrationStartTime = FDateTime::UtcNow();
	IntegrateEulerCromerWithPressureAcceleration();
	ComputationTimes.IntegrationTime = (FDateTime::UtcNow() - integrationStartTime).GetTotalSeconds();

	// Applies all behaviour defined by scripted volumes
	ApplyScriptedVolumesAfterIntegration();

	// measure time the whole step took
	ComputationTimes.TotalTime = (FDateTime::UtcNow() - totalStartTime).GetTotalSeconds();
}

void UIISPHSolver::ComputeSolverStatistics(bool computeSolverStats)
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

void UIISPHSolver::FitSolverAttributeArray()
{
	Attributes.resize(GetParticleContext()->GetFluids().size());

	for (int i = 0; i < GetParticleContext()->GetFluids().size(); i++) {
		UFluid* fluid = GetParticleContext()->GetFluids()[i];
		Attributes[i].resize(fluid->Particles->size());
	}
}

double UIISPHSolver::MaxTimeStep()
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

void UIISPHSolver::ComputeNonPressureAccelerations()
{
	FDateTime startTime = FDateTime::UtcNow();
	for (UAcceleration * acceleration : Accelerations) {
		acceleration->ApplyAcceleration(GetSimulator()->GetParticleContext());
	}

	ComputationTimes.AccelerationComputationTime = (FDateTime::UtcNow() - startTime).GetTotalSeconds();

	if (GetParticleContext()->GetPeriodicCondition() != nullptr) {
		GetParticleContext()->GetPeriodicCondition()->UpdateGhostParticleAcceleration();
	}
}

void UIISPHSolver::ComputeIntermediateVelocities()
{
	for (UFluid * fluid : GetParticleContext()->GetFluids()) {
		ParallelFor(fluid->Particles->size(), [&](int32 i) {
			Particle& f = fluid->Particles->at(i);
			Attributes[*fluid][i].IntermediateVelocity = f.Velocity + CurrentTimestep * f.Acceleration;
		});
	}
}

void UIISPHSolver::ComputeSourceTerms()
{
	for (UFluid * fluid : GetParticleContext()->GetFluids()) {
		ParallelFor(fluid->Particles->size(), [&](int32 i) {
			Particle& f = fluid->Particles->at(i);
			double velocityDivergence = 0;

			for (FluidNeighbor& ff : f.FluidNeighbors) {
				velocityDivergence += ff.GetParticle()->Mass * (Attributes[*ff.GetFluid()][ff].IntermediateVelocity - Attributes[*fluid][i].IntermediateVelocity) * GetKernel()->ComputeGradient(f, ff);
			}
			for (const Particle& fb : f.StaticBorderNeighbors) {
				// neighbor velocity should be { 0, 0, 0 } for static borders
				velocityDivergence += fb.Border->BorderDensityFactor * fb.Mass * (fb.Velocity - Attributes[*fluid][i].IntermediateVelocity) * GetKernel()->ComputeGradient(f, fb);
			}

			Attributes[*fluid][i].SourceTerm = f.Fluid->GetRestDensity() - (f.Density - CurrentTimestep * velocityDivergence);
		});
	}
}

void UIISPHSolver::ComputeDiagonalElement() {

	for (UFluid * fluid : GetParticleContext()->GetFluids()) {
		ParallelFor(fluid->Particles->size(), [&](int32 i) {
			Particle& f = fluid->Particles->at(i);
			// we don't need the diagonal element for particles with no predicted density error
			if (Attributes[*fluid][i].SourceTerm >= 0) {
				// return only exits Parallel call
				return;
			}

			Vector3D innersum = { 0, 0, 0 };

			for (const Particle& ff : f.FluidNeighbors) {
				innersum -= ff.Mass / pow(f.Fluid->GetRestDensity(), 2) * GetKernel()->ComputeGradient(f, ff);
			}

			for (const Particle& fb : f.StaticBorderNeighbors) {
				innersum -= 2 * fb.Border->BorderStiffness * fb.Mass / pow(f.Fluid->GetRestDensity(), 2) * GetKernel()->ComputeGradient(f, fb);
			}


			// first row of equation
			double firstline = 0;
			for (const Particle& ff : f.FluidNeighbors) {
				firstline += ff.Mass * innersum * GetKernel()->ComputeGradient(f, ff);
			}

			// second row of equation
			double secondline = 0;
			for (const Particle& ff : f.FluidNeighbors) {
				secondline += ff.Mass * f.Mass / pow(f.Fluid->GetRestDensity(), 2) * GetKernel()->ComputeGradient(ff, f) * GetKernel()->ComputeGradient(f, ff);
			}

			// third row of equation
			double thirdline = 0;
			for (const Particle& fb : f.StaticBorderNeighbors) {
				thirdline += fb.Mass * innersum * GetKernel()->ComputeGradient(f, fb);
			}

			Attributes[*fluid][i].Aff = pow(CurrentTimestep, 2) * (firstline + secondline + thirdline);
		});
	}
}

void UIISPHSolver::InitializePressureValues(bool clampAtZero) {

	// Fluid pressure
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

	// Static-border pressure
	GetBoundaryPressure()->ComputeAllPressureValues(GetSimulator()->GetParticleContext(), GetKernel());

	// In the special case of a periodic setting update pressure of ghost particles
	if (GetParticleContext()->GetPeriodicCondition() != nullptr) {
		GetParticleContext()->GetPeriodicCondition()->UpdateGhostParticlePressure();
	}
}


void UIISPHSolver::UpdatePressureAcceleration() {
	for (UFluid * fluid : GetParticleContext()->GetFluids()) {
		ParallelFor(fluid->Particles->size(), [&](int32 i) {
			Particle& f = fluid->Particles->at(i);
			f.Acceleration = -GetPressureGradient()->ComputePressureGradient(f, i) / f.Density;
		});
	}

	if (GetParticleContext()->GetPeriodicCondition() != nullptr) {
		GetParticleContext()->GetPeriodicCondition()->UpdateGhostParticleAcceleration();
	}
}

void UIISPHSolver::ComputePressureAccelerationCorrection()
{
	for (UFluid * fluid : GetParticleContext()->GetFluids()) {
		ParallelFor(fluid->Particles->size(), [&](int32 i) {
			Particle& f = fluid->Particles->at(i);

			Attributes[*fluid][i].Ap = 0;
			for (const Particle& ff : f.FluidNeighbors) {
				Attributes[*fluid][i].Ap += pow(CurrentTimestep, 2) * ff.Mass * (f.Acceleration - ff.Acceleration) * GetKernel()->ComputeGradient(f, ff);
			}
			for (const Particle& fb : f.StaticBorderNeighbors) {
				Attributes[*fluid][i].Ap += pow(CurrentTimestep, 2) * fb.Mass * f.Acceleration * GetKernel()->ComputeGradient(f, fb);
			}
		});
	}
}

void UIISPHSolver::RelaxedJacobiUpdatePressure()
{
	// fluid pressure
	for (UFluid * fluid : GetParticleContext()->GetFluids()) {
		ParallelFor(fluid->Particles->size(), [&](int32 i) {
			Particle& f = fluid->Particles->at(i);
			// update pressure only if Aff is not 0 (if Aff = 0, then there are no neighbors)
			if (std::abs(Attributes[*fluid][i].Aff) > DBL_EPSILON) {
				if (Attributes[*fluid][i].SourceTerm < 0) {
					f.Pressure = std::max(f.Pressure + JacobiFactor * (Attributes[*fluid][i].SourceTerm - Attributes[*fluid][i].Ap) / Attributes[*fluid][i].Aff, 0.0);
				}
			}
			else {
				f.Pressure = 0.0;
			}
		});
	}

	// Boundary pressure
	GetBoundaryPressure()->ComputeAllPressureValues(GetSimulator()->GetParticleContext(), GetKernel());


	// In the special case of a periodic setting update pressure of ghost particles
	if (GetParticleContext()->GetPeriodicCondition() != nullptr) {
		GetParticleContext()->GetPeriodicCondition()->UpdateGhostParticlePressure();
	}
}

bool UIISPHSolver::CheckAveragePredictedDensityError()
{
	for (UFluid * fluid : GetParticleContext()->GetFluids()) {
		ParallelFor(fluid->Particles->size(), [&](int32 i) {
			Particle& f = fluid->Particles->at(i);

		});
	}
	// compute predicted density errors and sum them
	double densitySum = 0.0;
	int numParticles = 0;
	for (UFluid * fluid : GetParticleContext()->GetFluids()) {

		// check individual particle density error
		if (ParallelExists<IISPHParticleAttributes>(Attributes[*fluid], [this](const IISPHParticleAttributes& attributes) {return (attributes.Ap - attributes.SourceTerm) > DesiredIndividualDensityError; })) {
			return false;
		}
		
		// check average density error
		densitySum += ParallelSum<IISPHParticleAttributes, double>(Attributes[*fluid], [](const IISPHParticleAttributes& attributes) {return std::max(attributes.Ap - attributes.SourceTerm, 0.0); }) / fluid->GetRestDensity();
		numParticles += fluid->Particles->size();
	}

	double averageDensityError = densitySum / numParticles;

	return averageDensityError <= DesiredAverageDensityError;
}


void UIISPHSolver::IntegrateEulerCromerWithPressureAcceleration() {
	for (UFluid * fluid : GetParticleContext()->GetFluids()) {
		ParallelFor(fluid->Particles->size(), [&](int32 i) {
			Particle& particle = fluid->Particles->at(i);

			if (!particle.IsScripted) {
				// acceleration should only contain pressure acceleration
				particle.Velocity = Attributes[*fluid][i].IntermediateVelocity + CurrentTimestep * particle.Acceleration;
				particle.Position = particle.Position + CurrentTimestep * particle.Velocity;
			}
		});
	}

	// In the special case of a periodic setting new particles are generated at necessary positions
	if (GetParticleContext()->GetPeriodicCondition() != nullptr) {
		GetParticleContext()->GetPeriodicCondition()->MoveOutOfBoundsParticles();
		GetParticleContext()->GetPeriodicCondition()->UpdateGhostParticleAttributes(true);
	}
}