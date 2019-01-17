#pragma once

#include <vector>
#include <algorithm>

#include "DataStructures/Vector3D.h"
#include "DataStructures/Matrix3D.h"

#include "Solver/Solver.h"

#include "CoreMinimal.h"

#include "DFSPH.generated.h"

struct DFSPHParticleAttributes {
	double Pressure;
	double SourceTerm;
	double Ap;
	double Aff;
	double IntermediateDensity;
	Vector3D IntermediateVelocity;
};

UCLASS(BlueprintType)
class UDFSPHSolver : public USolver {
	GENERATED_BODY()

public:

	~UDFSPHSolver() override;

	UFUNCTION(BlueprintPure, Category = "Solver")
	static UDFSPHSolver * CreateDFSPHSolver(TArray<UAcceleration*> accelerations,
		float desiredAverageVelocityDivergenceError = 0.1,
		float desiredIndividualVelocityDivergenceError = 1.0,
		float desiredAverageDensityError = 0.001,
		float desiredIndividualDensityError = 0.01,
		int minIterationVelocity = 1,
		int maxIterationVelocity = 100,
		int minIterationDensity = 1,
		int maxIterationDensity = 100,
		float jacobiFactor = 0.5,
		UBoundaryPressure * boundaryPressure = nullptr,
		UPressureGradient * pressureGradient = nullptr);

	void Step();
	void ComputeSolverStatistics(bool computeSolverStats = true);

protected:

	// Fits the attribute arrays required by the solver
	void FitSolverAttributeArray();

	// determines the maximum possible timestep the simulation can take
	double MaxTimeStep();

	// Computes all non pressure accelerations given in vector Accelerations
	void ComputeNonPressureAccelerations();
	void ComputeIntermediateVelocities(bool overrideOldIntermediateVelocity);
	void ComputeSourceTermsVelocityDivergence();
	void ComputeSourceTermsDensity();
	void ComputeDiagonalElement(bool clampAtZero);
	void InitializePressureValues(bool clampAtZero);
	void UpdatePressureAcceleration();
	void ComputePressureAccelerationCorrection(bool clampToZeroIncompleteNeighborhood);

	// One relaxed Jacobi step to update pressure values
	void RelaxedJacobiUpdatePressure(bool clampAtZero);

	// Computes predicted densities using intermeidate velocities and positions
	void ComputePredictedDensities();

	// Apply final pressure acceleration from density invariance solving to velocity and position
	void IntegrateEulerCromerWithPressureAcceleration();

	// returns true if average density error is smaller than desired density error
	bool CheckAveragePredictedDensityError();

	// returns true if the average divergence error is smaller than the desired velocity divergence error
	bool CheckAveragePredictedVelocityDivergenceError();

	bool HasEnoughNeighbors(const Particle& f) const;

	double DesiredAverageVelocityDivergenceError;
	double DesiredIndividualVelocityDivergenceError;
	double DesiredAverageDensityError;
	double DesiredIndividualDensityError;
	double JacobiFactor;
	int MinIterationVelocity;
	int MaxIterationVelocity;
	int MinIterationDensity;
	int MaxIterationDensity;

	std::vector<std::vector<DFSPHParticleAttributes>> Attributes;

};
