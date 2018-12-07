#include "PointGravity.h"

UPointGravity::~UPointGravity()
{
}


UPointGravity * UPointGravity::CreatePointGravity(FVector midpoint, float magnitude)
{
	UPointGravity * pointGravity = NewObject<UPointGravity>();
	pointGravity->Midpoint = midpoint;
	pointGravity->Magnitude = magnitude;

	// prevent garbage collection
	pointGravity->AddToRoot();

	return pointGravity;
}

void UPointGravity::ApplyAcceleration(UParticleContext * particleContext)
{
	for (UFluid * fluid : particleContext->GetFluids()) {
		ParallelFor(fluid->Particles->size(), [&](int32 i) {
			Particle& particle = fluid->Particles->at(i);
			particle.Acceleration += (Midpoint - particle.Position).Normalized() * Magnitude;
		});
	}
}
