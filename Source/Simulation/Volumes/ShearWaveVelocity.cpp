#include "ShearWaveVelocity.h"

AShearWaveVelocity::AShearWaveVelocity() {
	VolumeType = EVolumeType::ShearWaveVelocity;
	MaxVelocity = 10.0f;

	PositiveArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("PosArrow"));
	NegativeArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("NegArrow"));

	PositiveArrowComponent->RelativeLocation = { 0, 0, 50.f };
	NegativeArrowComponent->RelativeLocation = { 0, 0, -50.f };
	NegativeArrowComponent->RelativeRotation = FRotator(180, 0, 0);

	PositiveArrowComponent->SetupAttachment(RootComponent);
	NegativeArrowComponent->SetupAttachment(RootComponent);
	PositiveArrowComponent->bHiddenInGame = false;
	NegativeArrowComponent->bHiddenInGame = false;
}

AShearWaveVelocity::~AShearWaveVelocity()
{
}

void AShearWaveVelocity::ApplyBeforeIntegration(UParticleContext & particleContext, double timestep, double simulationTime)
{
	if (IsEnabled) {
		for (UFluid * fluid : particleContext.GetFluids()) {
			ParallelFor(fluid->Particles->size(), [&](int32 i) {
				Particle& particle = fluid->Particles->at(i);

				if (IsParticleAffected(particle)) {
					particle.IsScripted = true;

					// Project particle on the up vector to determine magnitude
					double projected = Vector3D::ProjectScalar(static_cast<Vector3D>(GetActorUpVector()), particle.Position - static_cast<Vector3D>(GetActorLocation()) * 0.1);

					double velocityMagnitude = sin(projected * 0.1 * PI) * MaxVelocity;
					particle.Acceleration = Vector3D(0.0);
					particle.Velocity = GetActorForwardVector() * velocityMagnitude;
					particle.Position += timestep * particle.Velocity;
				}
			});
		}
	}
}

double AShearWaveVelocity::MaxTimeStep(double timestepFactor, const UParticleContext & particleContext) const
{
	return particleContext.GetParticleDistance() * timestepFactor / MaxVelocity;
}




