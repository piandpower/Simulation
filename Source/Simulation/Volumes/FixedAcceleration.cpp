#include "FixedAcceleration.h"

AFixedAcceleration::AFixedAcceleration() {
	VolumeType = EVolumeType::FixedAcceleration;
	Acceleration = { 0, 0 ,0 };

	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));

	ArrowComponent->SetupAttachment(RootComponent);
	ArrowComponent->bHiddenInGame = false;
}

AFixedAcceleration::~AFixedAcceleration()
{
}

void AFixedAcceleration::ApplyBeforeIntegration(UParticleContext & particleContext, double timestep, double simulationTime)
{
	if (IsEnabled) {
		for (UFluid * fluid : particleContext.GetFluids()) {
			ParallelFor(fluid->Particles->size(), [&](int32 i) {
				Particle& particle = fluid->Particles->at(i);

				if (IsParticleAffected(particle)) {
					particle.IsScripted = true;
					particle.Acceleration = Acceleration;
					if (Integrate) {
						particle.Velocity += timestep * static_cast<Vector3D>(Acceleration);
						particle.Position += timestep * particle.Velocity;
					}
				}
			});
		}
	}
}

void AFixedAcceleration::OnConstruction(const FTransform & Transform)
{
	Super::OnConstruction(Transform);

	if (Acceleration.Size() == 0) {
		ArrowComponent->SetVisibility(false);
	}
	else {
		ArrowComponent->SetVisibility(true);
		ArrowComponent->SetWorldRotation(Acceleration.Rotation());
	}
}



