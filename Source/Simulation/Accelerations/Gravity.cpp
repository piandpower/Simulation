#include "Gravity.h"

UGravity::~UGravity()
{
}


UGravity * UGravity::CreateGravity(FVector acceleration)
{
	UGravity * gravity = NewObject<UGravity>();
	gravity->Acceleration = acceleration;

	// prevent garbage collection
	gravity->AddToRoot();

	return gravity;
}

void UGravity::ApplyAcceleration(UParticleContext * particleContext)
{
	for (UFluid * fluid : particleContext->GetFluids()) {
		ParallelFor(fluid->Particles->size(), [&](int32 i) {
			Particle& particle = fluid->Particles->at(i);
			particle.Acceleration += Acceleration;
		});
	}
}
