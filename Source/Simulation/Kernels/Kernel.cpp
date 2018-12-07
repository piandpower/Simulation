#include "Kernel.h"

UKernel::~UKernel()
{
}

double UKernel::GetParticleSpacing() const
{
	return ParticleSpacing;
}

double UKernel::GetSupportRange() const
{
	return SupportRange;
}

void UKernel::Build(double particleSpacing, EDimensionality dimensionality)
{
	ParticleSpacing = particleSpacing;
	Dimensionality = dimensionality;
}

double UKernel::ComputeKernel(const Particle & particle1, const Particle & particle2) const
{
	throw("This is an abstract base class and should never be called!");
	return 0.0;
}

double UKernel::ComputeKernel(const Vector3D& position1, const Vector3D& position2) const
{
	throw("This is an abstract base class and should never be called!");
	return 0.0;
}

Vector3D UKernel::ComputeKernelDerivative(const Particle & particle1, const Particle & particle2) const
{
	throw("This is an abstract base class and should never be called!");
	return Vector3D();
}

Vector3D UKernel::ComputeKernelDerivative(const Vector3D & position1, const Vector3D & position2) const
{
	throw("This is an abstract base class and should never be called!");
	return Vector3D();
}
