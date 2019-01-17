#pragma once

#include <vector>
#include <algorithm>
#include <unordered_map>


#include "DataStructures/Vector3D.h"

#include "Solver/Solver.h"

#include "CoreMinimal.h"

#include "IISPH.generated.h"

struct IISPHParticleAttributes {
	double Pressure;
	double SourceTerm;
	double Ap;
	double Aff;
	double IntermediateDensity;
	Vector3D IntermediateVelocity;
};

UCLASS(BlueprintType)
class UIISPHSolver : public USolver {
	GENERATED_BODY()

public:

	~UIISPHSolver() override;

	UFUNCTION(BlueprintPure, Category = "Solver")
	static UIISPHSolver * CreateIISPHSolver(TArray<UAcceleration*> accelerations,
		float desiredDensityError = 0.001,
		float desiredIndividualDensityError = 0.01,
		int minIteration = 2,
		int maxIteration = 100,
		float jacobiFactor = 0.5,
		UBoundaryPressure * boundaryPressure = nullptr,
		UPressureGradient * pressureGradient = nullptr);

	void Step();
	void ComputeSolverStatistics(bool computeSolverStats = true);

protected:

	// Fits the attribute arrays required by the solver
	void FitSolverAttributeArray();

	double MaxTimeStep();
	void ComputeNonPressureAccelerations();
	void ComputeIntermediateVelocities();
	void ComputeSourceTerms();
	void ComputeDiagonalElement();
	void InitializePressureValues(bool clampAtZero);
	void UpdatePressureAcceleration();
	void IntegrateEulerCromerWithPressureAcceleration();
	void ComputePressureAccelerationCorrection();
	void RelaxedJacobiUpdatePressure();

	// returns true if average density error is smaller than desired density error
	bool CheckAveragePredictedDensityError();

	double DesiredAverageDensityError;
	double DesiredIndividualDensityError;
	double JacobiFactor;
	int MaxIteration;
	int MinIteration;


	std::vector<std::vector<IISPHParticleAttributes>> Attributes;
};
