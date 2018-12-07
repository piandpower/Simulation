#pragma once

#include <vector>

#include "CoreMinimal.h"

#include "DataStructures/Utility.h"
#include "ParticleContext/ParticleContext.h"
#include "Accelerations/Acceleration.h"
#include "BoundaryPressure/BoundaryPressure.h"
#include "SceneComponents/Fluid.h"
#include "Kernels/Kernel.h"
#include "NeighborsFinders/NeighborsFinder.h"
#include "Volumes/ScriptedVolume.h"

#include "Runtime/Core/Public/Async/ParallelFor.h"

#include "Solver.generated.h"

class ASimulator;

UENUM(BlueprintType)
enum ESolverMethod {
	SESPH,
	IISPH,
	DFSPH,
	Dummy
};


USTRUCT(BlueprintType)
struct FComputationTimesPerStep {
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
		float TotalTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
		float NeighborhoodSearchTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
		float DensityComputationTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
		float PressureComputationTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
		float AccelerationComputationTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
		float IntegrationTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
		float ScriptedTime;

	FComputationTimesPerStep(float totalTime, float neighborhoodSearchTime, float densityComputationTime, float pressureComputationTime, float accelerationComputationTime, float integrationTime, float scriptedTime) :
		TotalTime(totalTime),
		NeighborhoodSearchTime(neighborhoodSearchTime),
		DensityComputationTime(densityComputationTime),
		PressureComputationTime(pressureComputationTime),
		AccelerationComputationTime(accelerationComputationTime),
		IntegrationTime(integrationTime),
		ScriptedTime(scriptedTime)
	{
	}

	FComputationTimesPerStep() :
		TotalTime(0.0f),
		NeighborhoodSearchTime(0.0f),
		DensityComputationTime(0.0f),
		PressureComputationTime(0.0f),
		AccelerationComputationTime(0.0f),
		IntegrationTime(0.0f),
		ScriptedTime(0.0f)
	{
	}
};


// Abstract parent class of all solvers. Contains some functions which are used in all solvers
UCLASS(BlueprintType)
class USolver : public UObject{
	GENERATED_BODY()
public:

	virtual ~USolver();

	virtual void Step();

	void Build(ASimulator * simulator, TArray<AScriptedVolume*> volumes);

	double ComputeAverageDensityError();

	virtual void ComputeSolverStatistics(bool computeSolverStats);



	ESolverMethod GetSolverType() const;

	// Timestep the simulation took in last iteration
	double GetCurrentTimestep() const;

	// Computation Times the last iterationstep took
	FComputationTimesPerStep GetComputationTimes() const;

	UFUNCTION(BlueprintPure)
	float GetLastAverageDensityError() const;

	UFUNCTION(BlueprintPure)
	int GetLastIterationCount() const;

	std::vector<double> OldTimesteps;
	std::vector<double> OldComputationTimesPerStep;
	std::vector<int> OldIterationCounts;
	std::vector<double> OldAverageDensityErrors;
	std::vector<double> OldKineticEnergies;

	ASimulator * GetSimulator() const;
	UParticleContext * GetParticleContext() const;
	UKernel * GetKernel() const;
	const std::vector<UFluid*>& GetFluids() const;
	const std::vector<UStaticBorder*>& GetBorders() const;

	UNeighborsFinder * GetNeighborsFinder() const;
	UBoundaryPressure * GetBoundaryPressure() const;

	UFUNCTION(BlueprintPure)
	TArray<UAcceleration*> GetAccelerations() const;

	UFUNCTION(BlueprintPure)
	TArray<AScriptedVolume*> GetScriptedVolumes() const;
protected:

	ESolverMethod SolverType;

	// Find all neighbors
	void FindNeighbors();

	// Initialize periodic condition
	void InitializePeriodicCondition();

	// Reset Accelerations to zero
	void ClearAcceleration();

	// Computes the density at each particle using positions and current neighborhoods
	void ComputeDensitiesExplicit();

	// Applies current acceleration to velocity and performs a position update with the new velocity
	void IntegrateEulerCromer();

	// Applies the effect of all scripted volumes
	void ApplyScriptedVolumesBeforeIntegration();

	// Appllies effects of volumes like spawning or deleting particles
	void ApplyScriptedVolumesAfterIntegration();

	double CurrentTimestep;

	FComputationTimesPerStep ComputationTimes;

	ASimulator * Simulator;

	UBoundaryPressure * BoundaryPressureComputer;

	bool FixedNextTimestep = false;

	TArray<UAcceleration*> Accelerations;
	TArray<AScriptedVolume*> Volumes;

	// stores the last computed density error for one simulation step so multiple uses don't need to recalculate it
	double LastAverageDensityError;

	// stores the last iteration count of the solver. Always 1 for SESPH
	int LastIterationCount;
};