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

double UCubicSplineKernel::ComputeValue(const Particle & particle1, const Particle & particle2) const
{
	return UCubicSplineKernel::ComputeValue(particle1.Position, particle2.Position);
}

double UCubicSplineKernel::ComputeValue(const Vector3D& position1, const Vector3D& position2) const
{
	double q = 2 * (position1 - position2).Size() / (SupportRange * ParticleSpacing);

	if (0 <= q && q < 1) {
		return (pow((2 - q), 3) - 4 * pow((1 - q), 3)) * Prefactor;
	}
	if (1 <= q && q < 2) {
		return pow((2 - q), 3) * Prefactor;
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

Vector3D UCubicSplineKernel::ComputeGradient(const Particle & particle1, const Particle & particle2) const
{
	return UCubicSplineKernel::ComputeGradient(particle1.Position, particle2.Position);
}

Vector3D UCubicSplineKernel::ComputeGradient(const Vector3D& position1, const Vector3D& position2) const
{
	Vector3D PositionDifference = position1 - position2;
	double q = 2 * (position1 - position2).Size() / (SupportRange * ParticleSpacing);
	Vector3D gradientq = 2 * PositionDifference / (PositionDifference.Size() * ParticleSpacing * SupportRange);

	// No pressure direction if they're at the same location 
	if (q == 0) {
		return { 0, 0, 0 };
	}

	if (0 <= q && q < 1) {
		return Prefactor * gradientq * (-3 * pow(2 - q, 2) + 12 * pow(1 - q, 2));
	}
	if (1 <= q && q < 2) {
		return Prefactor * gradientq * -3 * pow(2 - q, 2);
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

void UCubicSplineKernel::ComputePrefactor()
{
	switch (Dimensionality) {
	case EDimensionality::One:
		Prefactor = 1 / (6 * (ParticleSpacing)) * 2 / SupportRange;
		break;
	case EDimensionality::Two:
		Prefactor = 5 / (14 * PI * pow(ParticleSpacing, 2)) * 4 / (SupportRange * SupportRange);
		break;
	case EDimensionality::Three:
		Prefactor = 1 / (4 * PI * pow(ParticleSpacing, 3)) * 8 / pow(SupportRange, 3);
		break;
	}
}
