#pragma once
#include "ScriptedVolume.h"
#include "Classes/Components/SphereComponent.h"
#include "CoreMinimal.h"

#include "PointGravityVolume.generated.h"

UCLASS()
class APointGravityVolume : public AScriptedVolume {
	GENERATED_BODY()
public:
	APointGravityVolume();

	~APointGravityVolume() override;

	void ApplyBeforeIntegration(UParticleContext & particleContext, double timestep, double simulationTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Midpoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Magnitude;

protected:
	USphereComponent * SphereComponent;

	void OnConstruction(const FTransform& Transform) override;

};
