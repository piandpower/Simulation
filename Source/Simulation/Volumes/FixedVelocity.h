#pragma once
#include "ScriptedVolume.h"
#include "Classes/Components/ArrowComponent.h"
#include "CoreMinimal.h"

#include "FixedVelocity.generated.h"

UCLASS()
class AFixedVelocity : public AScriptedVolume {
	GENERATED_BODY()
public:
	AFixedVelocity();

	~AFixedVelocity() override;

	void ApplyBeforeIntegration(UParticleContext & particleContext, double timestep, double simulationTime) override;

	double MaxTimeStep(double timestepFactor, const UParticleContext & particleContext) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Velocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool Integrate = true;

protected:
	UArrowComponent * ArrowComponent;

	void OnConstruction(const FTransform& Transform) override;

};
