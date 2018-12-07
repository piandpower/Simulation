#pragma once

#include "CoreMinimal.h"
#include "SceneComponents/Particle.h"

#include "Kernel.generated.h"

enum EDimensionality;

UCLASS()
class UKernel : public UObject{
	GENERATED_BODY()
public:

	virtual ~UKernel();

	void Build(double particleSpacing, EDimensionality dimensionality);

	virtual double ComputeKernel(const Particle & particle1, const Particle & particle2) const;
	virtual double ComputeKernel(const Vector3D& position1, const Vector3D& position2) const;

	virtual Vector3D ComputeKernelDerivative(const Particle & particle1, const Particle & particle2) const;
	virtual Vector3D ComputeKernelDerivative(const Vector3D& position1, const Vector3D& position2) const;

	double GetParticleSpacing() const;
	double GetSupportRange() const;

protected:

	double ParticleSpacing;

	// Specifies the range the kernel function is > 0. Support range is always multiplied with particle spacing
	double SupportRange;

	EDimensionality Dimensionality;
};