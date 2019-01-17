// Fill out your copyright notice in the Description page of Project Settings.
#include "Simulator.h"

// Sets default values
ASimulator::ASimulator()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.001;

	CFLNumber = 0.01;
	SimulatedTime = 0;
	SimulationStatus = Paused;
}

ASimulator::~ASimulator()
{
}

void ASimulator::Initialize(UParticleContext * particleContext,
	USolver * solver,
	UKernel * kernelFunction,
	UNeighborsFinder * neighborsFinder,
	URecordManager * recordManager,
	TArray<AScriptedVolume*> volumes,
	float timestepFactor,
	float minTimestep,
	float maxTimestep,
	FString simulationName,
	bool adaptTimestepToFrameRecording,
	EDimensionality dimensionality)
{
	CFLNumber = timestepFactor;
	MinTimestep = minTimestep;
	MaxTimestep = maxTimestep;
	SimulationName = simulationName;
	AdaptTimestepToFrameRecording = adaptTimestepToFrameRecording;
	Dimensionality = dimensionality;

	Kernel = kernelFunction;
	NeighborsFinder = neighborsFinder;
	ParticleContext = particleContext;
	Solver = solver;
	RecordManager = recordManager;

	Kernel->Build(GetParticleContext()->GetParticleDistance(), dimensionality);
	NeighborsFinder->Build(GetKernel()->GetSupportRange());
	ParticleContext->Build(GetWorld(), this, dimensionality);
	Solver->Build(this, volumes);
	RecordManager->Build(GetWorld(), this);

	Volumes = volumes;
	for (AScriptedVolume * volume : Volumes) {
		volume->Build(GetWorld(), *ParticleContext);
	}
}

void ASimulator::StartPauseSimulation()
{
	if (SimulationStatus != Running) {
		SimulationStatus = Running;
		LastSimulationStart = FDateTime::UtcNow();

		// show the fluids
		GetParticleContext()->GetParticleVisualiser()->SetActorHiddenInGame(false);
		// hide the playback
		GetRecordManager()->GetParticleCloudActor()->SetActorHiddenInGame(true);

	}
	else {
		if (SimulationStatus == Running) {
			ElapsedTimeWhileSimulating += FDateTime::UtcNow() - LastSimulationStart;
		}
		SimulationStatus = Paused;
	}
}

void ASimulator::PauseSimulation() {
	if (SimulationStatus == Running) {
		ElapsedTimeWhileSimulating += FDateTime::UtcNow() - LastSimulationStart;
	}
	SimulationStatus = Paused;
}

void ASimulator::StartPauseReplay()
{
	if (SimulationStatus == Running) {
		ElapsedTimeWhileSimulating += FDateTime::UtcNow() - LastSimulationStart;
	}

	if (SimulationStatus != Replaying) {
		SimulationStatus = Replaying;

		// show the fluids
		GetParticleContext()->GetParticleVisualiser()->SetActorHiddenInGame(true);
		// show the playback
		GetRecordManager()->GetParticleCloudActor()->SetActorHiddenInGame(false);

		if (GetRecordManager()->ReplayIsAtEnd()) {
			GetRecordManager()->RewindReplay();
		}
	}
	else {
		SimulationStatus = ReplayPaused;
	}
}

ESimulationState ASimulator::GetSimulationStatus() const
{
	return SimulationStatus;
}

UParticleContext * ASimulator::GetParticleContext() const
{
	return ParticleContext;
}

void ASimulator::WriteSimulationToFile(std::string file)
{
	// Write simulation informations
	std::ofstream simulationFile;
	simulationFile.open(file);

	// test if we can write
	if (!simulationFile.is_open()) {
		throw("Could't write in file: " + file);
	}

	simulationFile << "Simulation Name:\t" << TCHAR_TO_UTF8(*SimulationName) << std::endl;
	simulationFile << "TimestepFactor:\t" << CFLNumber << std::endl;
	simulationFile << "MinTimestep:\t" << MinTimestep << std::endl;
	simulationFile << "MaxTimestep:\t" << MaxTimestep << std::endl;
	simulationFile << "Neighborhood Search Method:\t" << GetNeighborsFinder()->GetNeighborsFinderType() << std::endl;
	simulationFile << "Solver Methode:\t" << GetSolver()->GetSolverType() << std::endl;
	simulationFile << "Smoothing Length:\t" << GetParticleContext()->GetParticleDistance() << std::endl;
	simulationFile.close();
}

ASimulator * ASimulator::ReadSimulationStateFromFile(FString simulationName, int iteration)
{
	ASimulator * simulator = NewObject<ASimulator>();

	std::string path = TCHAR_TO_UTF8(*(FPaths::ProjectDir() + "Simulation Saves/" + simulationName + "/"));
	

	std::ifstream simulationFile;
	simulationFile.open(path + "Simulation.simulation");

	/*std::string simulationNameFromFile;
	double timestepFactor;
	double minTimestep;
	double maxTimestep;
	int neighborhoodSearchMethod;
	int kernelFunction;
	int solverMethod;
	int saveEachNthIteration;
	double smoothingLength;

	if (simulationFile.is_open()) {
		std::string name;
		std::string value;
		simulationFile >> name >> value;
		if (name != "Simulation Name:" || value != TCHAR_TO_UTF8(*simulationName)) {
			throw("Simulation name doesn't match!");
		}

	}
	else {
		throw("No such simulation found");
	}*/

	//simulator->Initialize()

	return simulator;
}

const FSimulationInformation ASimulator::SimulationInformation() const
{
	int numStaticParticles = 0;

	for (UStaticBorder * border : GetParticleContext()->GetStaticBorders()) {
		numStaticParticles += border->Particles->size();
	}

	int numFluidParticles = 0;
	for (UFluid * fluid : GetParticleContext()->GetFluids()) {
		numFluidParticles += fluid->Particles->size();
	}


	return FSimulationInformation(SimulationStatus,
		SimulatedTime,
		static_cast<float>(Solver->GetCurrentTimestep()),
		static_cast<float>(CFLNumber),
		GetIterationCount(),
		GetSolver()->GetLastAverageDensityError(),
		GetElapsedTimeWhileSimulating(),
		numFluidParticles,
		numStaticParticles,
		0,
		Solver->GetComputationTimes());
}

FString ASimulator::GetSimulationName() const
{
	return SimulationName;
}

FString ASimulator::GetSimulationPath() const
{
	return FPaths::ProjectDir();
}

const double ASimulator::GetElapsedTimeWhileSimulating() const
{
	if (SimulationStatus == Running) {
		return (ElapsedTimeWhileSimulating + (FDateTime::UtcNow() - LastSimulationStart)).GetTotalSeconds();
	}
	return ElapsedTimeWhileSimulating.GetTotalSeconds();
}

UKernel * ASimulator::GetKernel() const
{
	if (Kernel == nullptr) {
		throw("No Kernel Set");
	}
	return Kernel;
}

USolver * ASimulator::GetSolver() const
{
	return Solver;
}

UNeighborsFinder * ASimulator::GetNeighborsFinder() const
{
	return NeighborsFinder;
}

URecordManager * ASimulator::GetRecordManager() const
{
	return RecordManager;
}

void ASimulator::TestSimulation()
{
	for (UFluid * fluid : GetParticleContext()->GetFluids()) {
		for (Particle & particle : *fluid->Particles) {

			// Skip particle if there aren't enough neighbor particles
			if (particle.FluidNeighbors.size() < 48) {
				continue;
			}

			// Test density computation with masses. If all fluid partticles have the same mass
			// densityi should be near densityj should be near the computed particle density

			double densityi = 0;
			double densityj = 0;

			for (const Particle& ff : particle.FluidNeighbors) {
				densityj += ff.Mass * Kernel->ComputeValue(particle.Position, ff.Position);
				densityi += Kernel->ComputeValue(particle, ff);
			}

			densityi *= particle.Mass;

			// Test kernelsum over all neighbors. Should be inverse to the volume of the particle
			// Also Mass / Volume should be near density

			double kernelSum = 0;
			double volume = pow(fluid->GetParticleContext()->GetParticleDistance(), 3);

			for (const Particle& ff : particle.FluidNeighbors) {
				kernelSum += Kernel->ComputeValue(particle, ff);
			}

			double testedDensity = particle.Mass / volume;
			double inverseVolume = 1 / volume;


			// Test if KernelGradient is negated for opposite direction

			for (const Particle& ff : particle.FluidNeighbors) {
				Vector3D Gradient = Kernel->ComputeGradient(particle, ff);
				Vector3D NegatedOppositeGradient = -Kernel->ComputeGradient(ff, particle);
			}

			// Test KernelGradient Sum, should be near {0, 0, 0} for particles in fluid

			Vector3D kernelgradientSum = Vector3D(0, 0, 0);
			for (const Particle& ff : particle.FluidNeighbors) {
				kernelgradientSum += Kernel->ComputeGradient(particle, ff);
			}


			// Test KernelGradient Length, columns should form Identity matrix for particles in fluid

			Vector3D column1;
			Vector3D column2;
			Vector3D column3;

			for (const Particle& ff : particle.FluidNeighbors) {
				Vector3D positionDifference = (ff.Position - particle.Position);
				Vector3D kernelGradient = GetKernel()->ComputeGradient(particle, ff);

				column1 += positionDifference * kernelGradient.X;
				column2 += positionDifference * kernelGradient.Y;
				column3 += positionDifference * kernelGradient.Z;
			}

			column1 *= pow(fluid->GetParticleContext()->GetParticleDistance(), 3);
			column2 *= pow(fluid->GetParticleContext()->GetParticleDistance(), 3);
			column3 *= pow(fluid->GetParticleContext()->GetParticleDistance(), 3);

		}
	}
}

double ASimulator::GetSimulatedTime() const
{
	return SimulatedTime;
}

int ASimulator::GetIterationCount() const
{
	return IterationCount;
}

double ASimulator::GetCFLNumber() const
{
	return CFLNumber;
}

double ASimulator::GetMinTimestep() const
{
	return MinTimestep;
}

double ASimulator::GetMaxTimestep() const
{
	return MaxTimestep;
}

bool ASimulator::IsTimestepAdaptiveToFramerate() const
{
	return AdaptTimestepToFrameRecording;
}

UWorld * ASimulator::GetWorldBlueprint()  const
{
	return GetWorld();
}

TEnumAsByte<EDimensionality> ASimulator::GetDimensionality() const
{
	return Dimensionality;
}


// Called every frame
void ASimulator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// small simulations can do multiple substeps in one tick
	FDateTime tickStartTime = FDateTime::UtcNow();

	switch (SimulationStatus) {
	case Running:

		//TestSimulation();


		while ((FDateTime::UtcNow() - tickStartTime).GetTotalSeconds() < 0.01) {

			// Make an iteration with the selected solver. Default is SESPH
			Solver->Step();
			SimulatedTime += Solver->GetCurrentTimestep();

			// Calculate different infos about the simulation
			for (UFluid * fluid : GetParticleContext()->GetFluids()) {
				fluid->CalculateEnergies(ComputeEnergies);
			}
			Solver->ComputeSolverStatistics(ComputeSolverStats);

			// add one to the iteration count
			IterationCount++;

			// save the current state for recording. If frame needs to be captured exit the simulation loop
			if (RecordManager->UpdateTimeAndRecordings(DeltaTime, IterationCount, SimulatedTime))
				break;

		}
		
		// Update the visual part
		GetParticleContext()->UpdateVisual();

		break;

	case Paused:

		break;

	case Replaying:

		RecordManager->SetRecordedParticlePosition(DeltaTime);

		break;

	default:
		break;
	}

}

