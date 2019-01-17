#include "FixedVelocity.h"

AFixedVelocity::AFixedVelocity() {
	VolumeType = EVolumeType::FixedVelocity;
	Velocity = { 0, 0 ,0 };

	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));

	ArrowComponent->SetupAttachment(RootComponent);
	ArrowComponent->bHiddenInGame = false;
}

AFixedVelocity::~AFixedVelocity()
{
}

void AFixedVelocity::ApplyBeforeIntegration(UParticleContext & particleContext, double timestep, double simulationTime)
{
	if (IsEnabled) {
		for (UFluid * fluid : particleContext.GetFluids()) {
			ParallelFor(fluid->Particles->size(), [&](int32 i) {
				Particle& particle = fluid->Particles->at(i);

				if (IsParticleAffected(particle)) {
					particle.IsScripted = true;
					particle.Acceleration = Vector3D(0.0);
					particle.Velocity = Velocity;
					if (Integrate) {
						particle.Position += timestep * particle.Velocity;
					}
				}
			});
		}
	}
}

double AFixedVelocity::MaxTimeStep(double timestepFactor, const UParticleContext & particleContext) const
{
	return particleContext.GetParticleDistance() * timestepFactor / Velocity.Size();
}

void AFixedVelocity::OnConstruction(const FTransform & Transform)
{
	Super::OnConstruction(Transform);

	if (Velocity.Size() == 0) {
		ArrowComponent->SetVisibility(false);
	}
	else {
		ArrowComponent->SetVisibility(true);
		ArrowComponent->SetWorldRotation(Velocity.Rotation());
	}
}



