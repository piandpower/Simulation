#pragma once
#include <algorithm>
#include "Solver/Solver.h"
#include "Accelerations/Acceleration.h"

#include "CoreMinimal.h"

#include "SESPH.generated.h"

// a simple State Equation SPH solver
UCLASS(BlueprintType)
class USESPHSolver : public USolver {
	GENERATED_BODY()
public:

	~USESPHSolver() override;

	UFUNCTION(BlueprintPure, Category = "Solver")
		static USESPHSolver * CreateSESPHSolver(TArray<UAcceleration*> accelerations, float fluidStiffness = 6400.0f, UBoundaryPressure * boundaryPressure = nullptr);

	void ComputePressure();

	virtual void Step() override;
	void ComputeSolverStatistics(bool computeSolverStats = true);

	void ApplyPressureAcceleration();
	double MaxTimeStep();

	double FluidStiffness;

};
