#include "KillVolume.h"

AKillVolume::AKillVolume() {
	VolumeType = EVolumeType::Kill;
	SetActorHiddenInGame(true);
}

AKillVolume::~AKillVolume()
{
}

void AKillVolume::ApplyAfterIntegration(UParticleContext & particleContext, double timestep, double simulationTime)
{
	if (IsEnabled) {
		for (UFluid * fluid : particleContext.GetFluids()) {
			std::function<bool(const Particle&)> func = std::bind(&AScriptedVolume::IsParticleAffected, this, std::placeholders::_1);
			fluid->RemoveParticles(func);
		}
	}
}

