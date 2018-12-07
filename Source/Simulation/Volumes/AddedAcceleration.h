#pragma once
#include "ScriptedVolume.h"
#include "Classes/Components/ArrowComponent.h"
#include "CoreMinimal.h"

#include "AddedAcceleration.generated.h"

UCLASS()
class AAddedAcceleration : public AScriptedVolume {
	GENERATED_BODY()
public:
	AAddedAcceleration();

	~AAddedAcceleration() override;

	void ApplyBeforeIntegration(UParticleContext & particleContext, double timestep, double simulationTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Acceleration;

protected:
	UArrowComponent * ArrowComponent;

	void OnConstruction(const FTransform& Transform) override;

};
