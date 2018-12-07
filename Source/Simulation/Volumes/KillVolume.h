#pragma once
#include "ScriptedVolume.h"

#include "CoreMinimal.h"

#include "KillVolume.generated.h"

UCLASS()
class AKillVolume : public AScriptedVolume {
	GENERATED_BODY()
public:
	AKillVolume();

	~AKillVolume() override;

	void ApplyAfterIntegration(UParticleContext & particleContext, double timestep, double simulationTime) override;

};
