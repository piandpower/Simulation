#pragma once
#include <algorithm>
#include "Solver/Solver.h"
#include "Accelerations/Acceleration.h"

#include "CoreMinimal.h"

#include "Dummy.generated.h"

// a dummy solver for testing purposes
UCLASS(BlueprintType)
class UDummySolver : public USolver {
	GENERATED_BODY()
public:

	~UDummySolver() override;

	UFUNCTION(BlueprintPure, Category = "Solver")
		static UDummySolver * CreateDummySolver(TArray<UAcceleration*> accelerations, UBoundaryPressure * boundaryPressure = nullptr, bool computeNeighborhoods = true, bool computeNonPressureAccelerations = false, bool computeScriptedVolumes = true);

	void ComputePressure();

	virtual void Step() override;
	void ComputeSolverStatistics(bool computeSolverStats = true);

	double MaxTimeStep();

	double FluidStiffness;

protected:

	bool ComputeNeighborhoods;
	bool ComputeAccelerations;
	bool ComputeScriptedVolumes;

};
