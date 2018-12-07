#include "PointGravityVolume.h"

APointGravityVolume::APointGravityVolume() {
	VolumeType = EVolumeType::AddedAcceleration;
	Midpoint = { 0, 0 ,0 };
	Magnitude = 10.0;

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));

	SphereComponent->SetWorldLocation(Midpoint * 10);
	SphereComponent->SetWorldScale3D({ 0.1, 0.1, 0.1 });
	SphereComponent->SetupAttachment(RootComponent);
	SphereComponent->bHiddenInGame = false;
}

APointGravityVolume::~APointGravityVolume() {
}

void APointGravityVolume::ApplyBeforeIntegration(UParticleContext & particleContext, double timestep, double simulationTime)
{
	if (IsEnabled) {
		for (UFluid * fluid : particleContext.GetFluids()) {
			ParallelFor(fluid->Particles->size(), [&](int32 i) {
				Particle& particle = fluid->Particles->at(i);

				if (IsParticleAffected(particle)) {
					particle.Velocity += timestep * (static_cast<Vector3D>(Midpoint) - particle.Position).Normalized() * Magnitude;
					particle.Position += pow(timestep, 2) * (static_cast<Vector3D>(Midpoint) - particle.Position).Normalized() * Magnitude;
				}
			});
		}
	}
}

void APointGravityVolume::OnConstruction(const FTransform & Transform)
{
	Super::OnConstruction(Transform);
	SphereComponent->SetWorldLocation(Midpoint * 10);
	SphereComponent->SetWorldScale3D({ 0.1, 0.1, 0.1 });
}



