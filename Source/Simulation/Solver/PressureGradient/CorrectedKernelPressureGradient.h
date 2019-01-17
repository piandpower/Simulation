#pragma once

#include "PressureGradient.h"
#include "CoreMinimal.h"
#include "Solver/BoundaryPressure/MLSExtrapolation.h"

#include "CorrectedKernelPressureGradient.generated.h"

// the SPH way of computing pressure gradient with corrected kernel 
UCLASS()
class UCorrectedKernelPressureGradient : public UPressureGradient {
	GENERATED_BODY()

public:

	Vector3D ComputePressureGradient(Particle& f, int particleIndex) const override;

	void PrecomputeAllGeometryData(const UParticleContext& particleContext) override;

	UFUNCTION(BlueprintPure)
	static UCorrectedKernelPressureGradient * CreateCorrectedKernelPressureGradient(FEpsilons nanoEpsilons);

protected:

	double Epsilon1D;
	double Epsilon2D;
	double Epsilon3D;

	std::vector<std::vector<Matrix3D>> InvertedCorrectionMatrices;
};
