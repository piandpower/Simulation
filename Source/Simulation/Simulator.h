// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iterator>

#include "Kismet/KismetMathLibrary.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ParticleContext/SceneComponents/Fluid.h"
#include "ParticleContext/SceneComponents/StaticBorder.h"
#include "NeighborsFinders/NeighborsFinder.h"
#include "Kernels/Kernel.h"
#include "Solver/Solver.h"
#include "Recording/RecordManager.h"
#include "Volumes/ScriptedVolume.h"
#include "Recording/Sensors/Sensor.h"

#include "Simulator.generated.h"



UENUM(BlueprintType)
enum ESimulationState {
	Running,
	Paused,
	Replaying,
	ReplayPaused
};

USTRUCT(BlueprintType)
struct FSimulationInformation {
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
	TEnumAsByte<ESimulationState> SimulationStatus;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
	float SimulatedTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
	float Timestep;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
	float CFLNumber;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
	int Iteration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
	float ElapsedTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
	FComputationTimesPerStep ComputationTimes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
	float AverageDensityError;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
	int NumberOfFluidParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
	int NumberOfStaticParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
	int NumberOfRigidParticles;

	FSimulationInformation() :
		SimulationStatus(ESimulationState::Paused),
		SimulatedTime(0.0f),
		Timestep(0.0f),
		CFLNumber(0.f),
		Iteration(0),
		AverageDensityError(0.0f),
		NumberOfFluidParticles(0),
		NumberOfRigidParticles(0),
		NumberOfStaticParticles(0),
		ElapsedTime(0.0f),
		ComputationTimes(FComputationTimesPerStep())
	{
	}

	FSimulationInformation(TEnumAsByte<ESimulationState> simulationStatus,
		float simulatedTime,
		float timestep,
		float cflNumber,
		int iteration,
		float averageDensityError,
		float elapsedTime,
		int numberOfFluidParticles,
		int numberOfStaticParticles,
		int numberOfRigidParticles,
		FComputationTimesPerStep computationTimes)
		:
		SimulationStatus(simulationStatus),
		SimulatedTime(simulatedTime),
		Timestep(timestep),
		CFLNumber(cflNumber),
		Iteration(iteration),
		AverageDensityError(averageDensityError),
		NumberOfFluidParticles(numberOfFluidParticles),
		NumberOfRigidParticles(numberOfRigidParticles),
		NumberOfStaticParticles(numberOfStaticParticles),
		ElapsedTime(elapsedTime),
		ComputationTimes(computationTimes)
	{
	}
};

UENUM(BlueprintType)
enum EDimensionality {
	One,
	Two,
	Three
};

UCLASS()
class SIMULATION_API ASimulator : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASimulator();

	~ASimulator();

	UFUNCTION(BlueprintCallable)
		void Initialize(UParticleContext * particleContext,
			USolver * solver,
			UKernel * kernelFunction,
			UNeighborsFinder * neighborsFinder,
			URecordManager * recordManager,
			TArray<AScriptedVolume*> volumes,
			float timestepFactor = 0.1,
			float minTimestep = 0.0001,
			float maxTimestep = 0.005,
			FString simulationName = "Fluid Simulation",
			bool adaptTimestepToFrameRecording = true,
			EDimensionality dimensionality = EDimensionality::Three);

	UFUNCTION(BlueprintCallable)
		void StartPauseSimulation();

	UFUNCTION(BlueprintCallable)
		void PauseSimulation();

	UFUNCTION(BlueprintCallable)
		void StartPauseReplay();

	UFUNCTION(BlueprintPure)
		ESimulationState GetSimulationStatus() const;

	UFUNCTION(BlueprintPure)
		UParticleContext * GetParticleContext() const;

	void WriteSimulationToFile(std::string file);

	UFUNCTION(BlueprintCallable)
		static ASimulator * ReadSimulationStateFromFile(FString simulationName, int iteration = 0);

	UPROPERTY(BlueprintReadWrite)
		bool RecordParticles = true;
	UPROPERTY(BlueprintReadWrite)
		bool TakeScreenshots = false;
	UPROPERTY(BlueprintReadWrite)
		bool SaveSimulationState = false;
	UPROPERTY(BlueprintReadWrite)
		bool ComputeSolverStats = false;
	UPROPERTY(BlueprintReadWrite)
		bool ComputeEnergies = false;

	UFUNCTION(BlueprintPure)
		const FSimulationInformation SimulationInformation() const;
	
	UFUNCTION(BlueprintPure)
		FString GetSimulationName() const;

	UFUNCTION(BlueprintPure)
		FString GetSimulationPath() const;

	const double GetElapsedTimeWhileSimulating() const;

	UFUNCTION(BlueprintPure)
	UKernel * GetKernel() const;

	UFUNCTION(BlueprintPure)
	USolver * GetSolver() const;

	UFUNCTION(BlueprintPure)
	UNeighborsFinder * GetNeighborsFinder() const;

	UFUNCTION(BlueprintPure)
	URecordManager * GetRecordManager() const;

	UFUNCTION(BlueprintPure)
	UWorld * GetWorldBlueprint() const;

	UFUNCTION(BlueprintPure)
	TEnumAsByte<EDimensionality> GetDimensionality() const;

protected:
	FString SimulationName;

	TEnumAsByte<ESimulationState> SimulationStatus;

	TEnumAsByte<EDimensionality> Dimensionality;

	UNeighborsFinder * NeighborsFinder;
	UKernel * Kernel;
	USolver * Solver;
	URecordManager * RecordManager;

	// Contains all particles including fluids and borders in a simulation domain
	UParticleContext * ParticleContext = nullptr;

	// Contains all scripted behavior of the simulation
	TArray<AScriptedVolume*> Volumes;

public:

	// Performs various tests of the simulation. Attention, totally kills performance!
	void TestSimulation();

	double GetSimulatedTime() const;
	int GetIterationCount() const;

	double GetCFLNumber() const;
	double GetMinTimestep() const;
	double GetMaxTimestep() const;
	bool IsTimestepAdaptiveToFramerate() const;


protected:

	double MinTimestep;
	double MaxTimestep;
	double CFLNumber;

	double SimulatedTime;
	int IterationCount;

	bool AdaptTimestepToFrameRecording = false;


	FDateTime LastSimulationStart;
	FTimespan ElapsedTimeWhileSimulating;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
