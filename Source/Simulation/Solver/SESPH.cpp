#include "SESPH.h"
#include "Simulator.h"



USESPHSolver::~USESPHSolver()
{
}

USESPHSolver * USESPHSolver::CreateSESPHSolver(TArray<UAcceleration*> accelerations, float fluidStiffness, UBoundaryPressure * boundaryPressure)
{
	USESPHSolver * sesphsolver = NewObject<USESPHSolver>();
	sesphsolver->SolverType = ESolverMethod::SESPH;

	sesphsolver->Accelerations = accelerations;
	sesphsolver->FluidStiffness = fluidStiffness;
	sesphsolver->LastIterationCount = 1;

	sesphsolver->BoundaryPressureComputer = boundaryPressure;

	// prevent garbage collection
	sesphsolver->AddToRoot();
	return sesphsolver;
}

void USESPHSolver::Step()
{
	FDateTime startTime = FDateTime::UtcNow();

	InitializePeriodicCondition();

	// Find all neighbors using the specified neighborhood search method
	FindNeighbors();

	// Calculate density errors
	ComputeDensitiesExplicit();
	ComputeAverageDensityError();

	// Computes pressure using a state equation
	ComputePressure();


	FDateTime accelerationStartTime = FDateTime::UtcNow();
	ClearAcceleration();

	// Non-pressure accelerations
	for (UAcceleration * acceleration : Accelerations) {
		acceleration->ApplyAcceleration(GetSimulator()->GetParticleContext());
	}

	ApplyPressureAcceleration();

	ComputationTimes.AccelerationComputationTime = (FDateTime::UtcNow() - accelerationStartTime).GetTotalSeconds();

	FDateTime integrationStartTime = FDateTime::UtcNow();
	MaxTimeStep();

	// Applies all behaviour defined by scripted volumes
	ApplyScriptedVolumesBeforeIntegration();

	IntegrateEulerCromer();

	// Applies all behaviour defined by scripted volumes
	ApplyScriptedVolumesAfterIntegration();

	ComputationTimes.IntegrationTime = (FDateTime::UtcNow() - integrationStartTime).GetTotalSeconds();

	ComputationTimes.TotalTime = (FDateTime::UtcNow() - startTime).GetTotalSeconds();
}

void USESPHSolver::ComputeSolverStatistics(bool computeSolverStats)
{
	if (computeSolverStats) {
		OldTimesteps.push_back(CurrentTimestep);
		OldComputationTimesPerStep.push_back(ComputationTimes.TotalTime);
		OldIterationCounts.push_back(1);
		OldAverageDensityErrors.push_back(GetLastAverageDensityError());
		double kineticEnergy = 0.0;
		for (UFluid * fluid : GetParticleContext()->GetFluids()) {
			kineticEnergy += fluid->CalculateKineticEnergy();
		}
		OldKineticEnergies.push_back(kineticEnergy);
	}
}


void USESPHSolver::ComputePressure()
{
	// fluid pressure
	for (UFluid * fluid : GetSimulator()->GetParticleContext()->GetFluids()) {
		ParallelFor(fluid->Particles->size(), [fluid, this](int32 i) {
			Particle& particle = fluid->Particles->at(i);
			particle.Pressure = FluidStiffness * std::max(pow(particle.Density / particle.Fluid->GetRestDensity(), 7) - 1, 0.0);
		});
	}

	// boundary pressure
	BoundaryPressureComputer->ComputeAllPressureValues(GetSimulator()->GetParticleContext(), GetKernel());

	// In the special case of a periodic setting update pressure of ghost particles
	if (GetParticleContext()->GetPeriodicCondition() != nullptr) {
		GetParticleContext()->GetPeriodicCondition()->UpdateGhostParticlePressure();
	}
}

void USESPHSolver::ApplyPressureAcceleration()
{
	for (UFluid * fluid : GetSimulator()->GetParticleContext()->GetFluids()) {
		ParallelFor(fluid->Particles->size(), [fluid, this](int32 i) {
			Particle& f = fluid->Particles->at(i);

			Vector3D fluidsum = { 0, 0, 0 };
			for (const Particle& ff : f.FluidNeighbors) {
				fluidsum -= (f.Pressure / pow(f.Fluid->GetRestDensity(), 2) + ff.Pressure / pow(ff.Fluid->GetRestDensity(), 2)) * GetKernel()->ComputeKernelDerivative(f, ff);
			}
			fluidsum *= f.Mass;

			Vector3D solidsum = { 0, 0, 0 };
			for (const Particle& fb : f.StaticBorderNeighbors) {
				solidsum -= fb.Border->BorderStiffness * fb.Mass * f.Fluid->GetRestDensity() *
					(f.Pressure / pow(f.Fluid->GetRestDensity(), 2) +
						BoundaryPressureComputer->GetPressureValue(fb, f) / pow(f.Fluid->GetRestDensity(), 2))
					* GetKernel()->ComputeKernelDerivative(f, fb);
			}
			f.Acceleration += fluidsum + solidsum;
		});
	}

	if (GetParticleContext()->GetPeriodicCondition() != nullptr) {
		GetParticleContext()->GetPeriodicCondition()->UpdateGhostParticleAcceleration();
	}
}

double USESPHSolver::MaxTimeStep()
{
	// if there is a fixed next timestep because of incoming frame-recording, then simply take last timestep
	if (FixedNextTimestep) {
		FixedNextTimestep = false;
		return CurrentTimestep;
	}

	double maxVelocity = 0.0;
	
	for (UFluid * fluid : GetParticleContext()->GetFluids()) {
		if (fluid->Particles->size() > 0)
			maxVelocity = std::max(maxVelocity, ParallelMax<Particle, double>(*fluid->Particles, [](const Particle & particle) -> double {return particle.Velocity.Size(); }));
	}

	if (maxVelocity <= 0) {

		CurrentTimestep = Simulator->GetMaxTimestep();
	}
	else {
		CurrentTimestep = FMath::Max(FMath::Min(Simulator->GetCFLNumber() * Simulator->GetParticleContext()->GetParticleDistance() / maxVelocity, Simulator->GetMaxTimestep()), Simulator->GetMinTimestep());
	}

	// sometimes volumes need to restrict the timestep
	for (AScriptedVolume * scriptedVolume : Volumes) {
		std::min(CurrentTimestep, scriptedVolume->MaxTimeStep(Simulator->GetCFLNumber(), *Simulator->GetParticleContext()));
	}

	// Check if CurrentTimestep needs to be smaller because of incoming new recording-frame
	if (Simulator->IsTimestepAdaptiveToFramerate() && CurrentTimestep * 2 + Simulator->GetSimulatedTime() > Simulator->GetRecordManager()->GetNextRecordTime()) {
		// next timestep should be the same as this one
		FixedNextTimestep = true;
		CurrentTimestep = (Simulator->GetRecordManager()->GetNextRecordTime() - Simulator->GetSimulatedTime()) / 2;
	}

	return CurrentTimestep;
}
