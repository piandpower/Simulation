#include "CubicSplineKernel.h"
#include "Simulator.h"

UCubicSplineKernel::~UCubicSplineKernel()
{
}

UCubicSplineKernel * UCubicSplineKernel::CreateCubicSplineKernel(float supportRange)
{
	UCubicSplineKernel * cubicSplineKernel = NewObject<UCubicSplineKernel>();
	cubicSplineKernel->SupportRange = supportRange;

	// prevent garbage collection
	cubicSplineKernel->AddToRoot();
	return cubicSplineKernel;
}

double UCubicSplineKernel::ComputeKernel(const Particle & particle1, const Particle & particle2) const
{
	return UCubicSplineKernel::ComputeKernel(particle1.Position, particle2.Position);
}

double UCubicSplineKernel::ComputeKernel(const Vector3D& position1, const Vector3D& position2) const
{
	double q = 2 * (position1 - position2).Size() / (SupportRange * ParticleSpacing);

	double prefactor = 0.0;

	switch (Dimensionality) {
	case EDimensionality::One:
		prefactor = 1 / (6 * (ParticleSpacing));
		break;
	case EDimensionality::Two:
		prefactor = 5 / (14 * PI * pow(ParticleSpacing, 2));
		break;
	case EDimensionality::Three:
		prefactor = 1 / (4 * PI * pow(ParticleSpacing, 3));
		break;
	}

	if (0 <= q && q < 1) {
		return (pow((2 - q), 3) - 4 * pow((1 - q), 3)) * prefactor;
	}
	if (1 <= q && q < 2) {
		return pow((2 - q), 3) * prefactor;
	}
	if (2 <= q) {
		return 0;
	}
	else {
		// Never ever should this be called
		throw("Distance between Particles is negative? The Kernel Function is broken.");
		return -1;
	}
	
}

Vector3D UCubicSplineKernel::ComputeKernelDerivative(const Particle & particle1, const Particle & particle2) const
{
	return UCubicSplineKernel::ComputeKernelDerivative(particle1.Position, particle2.Position);
}

Vector3D UCubicSplineKernel::ComputeKernelDerivative(const Vector3D& position1, const Vector3D& position2) const
{
	Vector3D PositionDifference = position1 - position2;
	double q = 2 * PositionDifference.Size() / (SupportRange * ParticleSpacing);
	Vector3D gradientq = PositionDifference / (PositionDifference.Size() * ParticleSpacing);

	// No pressure direction if they're at the same location 
	if (q == 0) {
		return { 0, 0, 0 };
	}

	double prefactor = 0.0;
	switch (Dimensionality) {
	case EDimensionality::One:
		prefactor = 1 / (6 * (ParticleSpacing));
		break;
	case EDimensionality::Two:
		prefactor = 5 / (14 * PI * pow(ParticleSpacing, 2));
		break;
	case EDimensionality::Three:
		prefactor = 1 / (4 * PI * pow(ParticleSpacing, 3));
		break;
	}


	if (0 <= q && q < 1) {
		return prefactor * gradientq * (-3 * pow(2 - q, 2) + 12 * pow(1 - q, 2));
	}
	if (1 <= q && q < 2) {
		return prefactor * gradientq * -3 * pow(2 - q, 2);
	}
	if (2 <= q) {
		return { 0, 0, 0 };
	}
	else {
		// Never ever should this be called
		throw("Distance between Particles is negative? The Kernel Function Derivative is broken.");
		return { -1, -1, -1 };
	}
}
