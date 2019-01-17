#include "Mirroring.h"

UMirroring * UMirroring::CreateMirroringPressureComputation() {
	UMirroring * mirroring = NewObject<UMirroring>();

	// prevent garbage collection
	mirroring->AddToRoot();

	return mirroring;
}


UMirroring::~UMirroring()
{
}

void UMirroring::ComputeAllPressureValues(UParticleContext * particleContext, UKernel * kernel)
{
}

double UMirroring::GetPressureValue(const Particle & staticBoundaryParticle, const Particle & neighboringFluidParticle) const
{
	return neighboringFluidParticle.Pressure;
}
