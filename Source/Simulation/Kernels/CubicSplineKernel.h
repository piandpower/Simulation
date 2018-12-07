#pragma once

#include "Kernel.h"

#include "CubicSplineKernel.generated.h"

UCLASS()
class UCubicSplineKernel : public UKernel {
	GENERATED_BODY()
public:
	~UCubicSplineKernel() override;

	UFUNCTION(BlueprintPure)
	static UCubicSplineKernel * CreateCubicSplineKernel(float supportRange = 2.f);

	double ComputeKernel(const Particle& particle1, const Particle& particle2) const override;
	double ComputeKernel(const Vector3D& position1, const Vector3D& position2) const override;

	Vector3D ComputeKernelDerivative(const Particle& particle1, const Particle& particle2) const override;
	Vector3D ComputeKernelDerivative(const Vector3D& position1, const Vector3D& position2) const override;
};