#include "Damping.h"

UDamping::~UDamping()
{
}


UDamping * UDamping::CreateDamping(float dampingFactor)
{
	UDamping * damping = NewObject<UDamping>();
	damping->DampingFactor = dampingFactor;

	// prevent garbage collection
	damping->AddToRoot();

	return damping;
}

void UDamping::ApplyAcceleration(UParticleContext * particleContext)
{
	for (UFluid * fluid : particleContext->GetFluids()) {
		ParallelFor(fluid->Particles->size(), [&](int32 i) {
			Particle& particle = fluid->Particles->at(i);
			particle.Acceleration += -DampingFactor * particle.Velocity / particle.Mass;
		});
	}
}
