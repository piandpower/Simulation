#include "AddedAcceleration.h"

AAddedAcceleration::AAddedAcceleration() {
	VolumeType = EVolumeType::AddedAcceleration;
	Acceleration = { 0, 0 ,0 };

	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));

	ArrowComponent->SetupAttachment(RootComponent);
	ArrowComponent->bHiddenInGame = false;
}

AAddedAcceleration::~AAddedAcceleration()
{
}

void AAddedAcceleration::ApplyBeforeIntegration(UParticleContext & particleContext, double timestep, double simulationTime)
{
	if (IsEnabled) {
		for (UFluid * fluid : particleContext.GetFluids()) {
			ParallelFor(fluid->Particles->size(), [&](int32 i) {
				Particle& particle = fluid->Particles->at(i);

				if (IsParticleAffected(particle)) {
					particle.Velocity += timestep * static_cast<Vector3D>(Acceleration);
					particle.Position += pow(timestep, 2) * static_cast<Vector3D>(Acceleration);
				}
			});
		}
	}
}

void AAddedAcceleration::OnConstruction(const FTransform & Transform)
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



