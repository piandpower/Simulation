#include "Dummy.h"
#include "Simulator.h"



UDummySolver::~UDummySolver()
{
}

UDummySolver * UDummySolver::CreateDummySolver(TArray<UAcceleration*> accelerations, UBoundaryPressure * boundaryPressure, bool computeNeighborhoods, bool computeNonPressureAccelerations, bool computeScriptedVolumes)
{
	UDummySolver * dummySolver = NewObject<UDummySolver>();
	dummySolver->SolverType = ESolverMethod::Dummy;

	dummySolver->ComputeAccelerations = computeNonPressureAccelerations;
	dummySolver->ComputeNeighborhoods = computeNeighborhoods;
	dummySolver->ComputeScriptedVolumes = computeScriptedVolumes;
	dummySolver->Accelerations = accelerations;
	dummySolver->LastIterationCount = 1;

	dummySolver->BoundaryPressureComputer = boundaryPressure;

	// prevent garbage collection
	dummySolver->AddToRoot();
	return dummySolver;
}

void UDummySolver::Step()
{
	if (Simulator == nullptr) {
		throw("No Simulator specified");
	}

	FDateTime startTime = FDateTime::UtcNow();


	switch (GetSimulator()->GetDimensionality()) {
	case One:


		break;
	case Two: {

		GetSimulator()->GetParticleContext()->GetFluids()[0]->Particles->at(0).Position = Vector3D(0.0, 0.0, 0.0);
		GetSimulator()->GetParticleContext()->GetFluids()[0]->Particles->at(1).Position = Vector3D(1.0, 0.0, 0.0);
		GetSimulator()->GetParticleContext()->GetFluids()[0]->Particles->at(2).Position = Vector3D(0.0, 0.0, 1.0);

		GetSimulator()->GetParticleContext()->GetFluids()[0]->Particles->at(0).Pressure = 100;
		GetSimulator()->GetParticleContext()->GetFluids()[0]->Particles->at(1).Pressure = 200;
		GetSimulator()->GetParticleContext()->GetFluids()[0]->Particles->at(2).Pressure = 200;

		GetSimulator()->GetParticleContext()->GetFluids()[0]->Particles->at(0).Density = 1;
		GetSimulator()->GetParticleContext()->GetFluids()[0]->Particles->at(1).Density = 1;
		GetSimulator()->GetParticleContext()->GetFluids()[0]->Particles->at(2).Density = 1;

		GetSimulator()->GetParticleContext()->GetStaticBorders().at(1)->Particles->at(0).Position = Vector3D(1.0, 0.0, 1.0);
	}
		break;
	case Three:
		break;
	}

	if (ComputeNeighborhoods) {
		// Find all neighbors using the specified neighborhood search method
		FindNeighbors();
	}

	// Calculate density errors
	ComputeDensitiesExplicit();
	ComputeAverageDensityError();

	// ComputePressure();
	BoundaryPressureComputer->ComputeAllPressureValues(GetSimulator()->GetParticleContext(), GetKernel());


	if (ComputeAccelerations) {
		FDateTime accelerationStartTime = FDateTime::UtcNow();

		ClearAcceleration();

		// Non-pressure accelerations
		for (UAcceleration * acceleration : Accelerations) {
			acceleration->ApplyAcceleration(GetSimulator()->GetParticleContext());
		}


		ComputationTimes.AccelerationComputationTime = (FDateTime::UtcNow() - accelerationStartTime).GetTotalSeconds();


		FDateTime integrationStartTime = FDateTime::UtcNow();
		MaxTimeStep();

		// Applies all behaviour defined by scripted volumes
		if (ComputeScriptedVolumes)
			ApplyScriptedVolumesBeforeIntegration();

		IntegrateEulerCromer();

		// Applies all behaviour defined by scripted volumes
		if (ComputeScriptedVolumes)
			ApplyScriptedVolumesAfterIntegration();

		ComputationTimes.IntegrationTime = (FDateTime::UtcNow() - integrationStartTime).GetTotalSeconds();

		ComputationTimes.TotalTime = (FDateTime::UtcNow() - startTime).GetTotalSeconds();
	}


}

void UDummySolver::ComputeSolverStatistics(bool computeSolverStats)
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


void UDummySolver::ComputePressure()
{
	for (UFluid * fluid : GetSimulator()->GetParticleContext()->GetFluids()) {
		ParallelFor(fluid->Particles->size(), [fluid, this](int32 i) {
			Particle& particle = fluid->Particles->at(i);
			particle.Pressure = FluidStiffness * std::max(pow(particle.Density / particle.Fluid->GetRestDensity(), 7) - 1, 0.0);
		});
	}
}


double UDummySolver::MaxTimeStep()
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
