#pragma once
#include "ScriptedVolume.h"
#include "Classes/Components/ArrowComponent.h"
#include "CoreMinimal.h"

#include "ShearWaveVelocity.generated.h"

UCLASS()
class AShearWaveVelocity : public AScriptedVolume {
	GENERATED_BODY()
public:
	AShearWaveVelocity();

	~AShearWaveVelocity() override;

	void ApplyBeforeIntegration(UParticleContext & particleContext, double timestep, double simulationTime) override;

	double MaxTimeStep(double timestepFactor, const UParticleContext & particleContext) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxVelocity;

protected:
	UArrowComponent * PositiveArrowComponent;
	UArrowComponent * NegativeArrowComponent;
};
