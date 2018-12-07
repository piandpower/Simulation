#pragma once

#include "DataStructures/Vector3D.h"

#include "Accelerations/Acceleration.h"

#include "CoreMinimal.h"

#include "Damping.generated.h"

UCLASS()
class UDamping : public UAcceleration {
	GENERATED_BODY()

public:

	~UDamping() override;

	UFUNCTION(BlueprintPure, Category = "Accelerations")
		static UDamping * CreateDamping(float dampingFactor);

	void ApplyAcceleration(UParticleContext * particleContext) override;

	double DampingFactor;
};
