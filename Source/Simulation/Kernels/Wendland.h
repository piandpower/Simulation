#pragma once

#include "Kernel.h"

#include "Wendland.generated.h"

UCLASS()
class UWendland : public UKernel {
	GENERATED_BODY()
public:
	~UWendland() override;

	UFUNCTION(BlueprintPure)
	static UWendland * CreateWendlandKernel(float supportRange = 2.f);

	double ComputeValue(const Particle& particle1, const Particle& particle2) const override;
	double ComputeValue(const Vector3D& position1, const Vector3D& position2) const override;

	Vector3D ComputeGradient(const Particle& particle1, const Particle& particle2) const override;
	Vector3D ComputeGradient(const Vector3D& position1, const Vector3D& position2) const override;

protected:
	void ComputePrefactor() override;

	double Prefactor;
};