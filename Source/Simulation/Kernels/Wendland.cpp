#include "Wendland.h"
#include "Simulator.h"

UWendland::~UWendland()
{
}

UWendland * UWendland::CreateWendlandKernel(float supportRange)
{
	UWendland * wendlandKernel = NewObject<UWendland>();
	wendlandKernel->SupportRange = supportRange;

	// prevent garbage collection
	wendlandKernel->AddToRoot();
	return wendlandKernel;
}

double UWendland::ComputeValue(const Particle & particle1, const Particle & particle2) const
{
	return UWendland::ComputeValue(particle1.Position, particle2.Position);
}

double UWendland::ComputeValue(const Vector3D& position1, const Vector3D& position2) const
{
	double q = 2 * (position1 - position2).Size() / (SupportRange * ParticleSpacing);
	return pow(1 - q / 2, 4) * (2 * q + 1) * Prefactor;
}

Vector3D UWendland::ComputeGradient(const Particle & particle1, const Particle & particle2) const
{
	return UWendland::ComputeGradient(particle1.Position, particle2.Position);
}

Vector3D UWendland::ComputeGradient(const Vector3D& position1, const Vector3D& position2) const
{
	Vector3D PositionDifference = position1 - position2;
	double q = 2 * (position1 - position2).Size() / (SupportRange * ParticleSpacing);
	Vector3D gradientq = 2 * PositionDifference / (PositionDifference.Size() * ParticleSpacing * SupportRange);

	if (0 < q && q < 2) {
		return Prefactor * -5 * gradientq * q * pow(1 - 0.5 * q, 3);
	}
	// No pressure direction if they're at the same location 
	return Vector3D::Zero;
}

void UWendland::ComputePrefactor()
{
	switch (Dimensionality) {
	case EDimensionality::One:
		throw("One Dimension is not supported!");
		break;
	case EDimensionality::Two:
		Prefactor = 7.0 / (4.0 * PI * ParticleSpacing * ParticleSpacing) * 4.0 / (SupportRange * SupportRange);
		break;
	case EDimensionality::Three:
		Prefactor = 21.0 / (16.0 * PI * pow(ParticleSpacing, 3)) * 8.0 / pow(SupportRange, 3);
		break;
	}
}
