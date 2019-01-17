#pragma once

#include "PressureGradient.h"
#include "CoreMinimal.h"

#include "SPHPressureGradient.generated.h"

// The SPH way of computing pressure gradient
UCLASS()
class USPHPressureGradient : public UPressureGradient {
	GENERATED_BODY()

public:

	Vector3D ComputePressureGradient(Particle& f, int particleIndex) const override;

	UFUNCTION(BlueprintPure)
	static USPHPressureGradient * CreateSPHPressureGradient();

};
