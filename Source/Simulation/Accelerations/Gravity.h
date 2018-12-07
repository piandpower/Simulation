#pragma once

#include "DataStructures/Vector3D.h"

#include "Accelerations/Acceleration.h"

#include "CoreMinimal.h"

#include "Gravity.generated.h"

UCLASS()
class UGravity : public UAcceleration {
	GENERATED_BODY()

public:

	~UGravity() override;

	UFUNCTION(BlueprintPure, Category = "Accelerations")
		static UGravity * CreateGravity(FVector acceleration = FVector( 0, 0, -9.81 ));

	void ApplyAcceleration(UParticleContext * particleContext) override;

	Vector3D Acceleration;
};
