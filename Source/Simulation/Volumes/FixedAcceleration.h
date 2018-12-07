#pragma once

#include <unordered_map>

#include "ScriptedVolume.h"
#include "Classes/Components/ArrowComponent.h"
#include "CoreMinimal.h"

#include "FixedAcceleration.generated.h"


UCLASS()
class AFixedAcceleration : public AScriptedVolume {
	GENERATED_BODY()
public:
	AFixedAcceleration();

	~AFixedAcceleration() override;

	void ApplyBeforeIntegration(UParticleContext & particleContext, double timestep, double simulationTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Acceleration;

protected:
	UArrowComponent * ArrowComponent;

	void OnConstruction(const FTransform& Transform) override;

};
