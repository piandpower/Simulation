#pragma once

#include "DataStructures/Vector3D.h"

#include "Accelerations/Acceleration.h"

#include "CoreMinimal.h"

#include "PointGravity.generated.h"

UCLASS()
class UPointGravity : public UAcceleration {
	GENERATED_BODY()

public:

	~UPointGravity() override;

	UFUNCTION(BlueprintPure, Category = "Accelerations")
		static UPointGravity * CreatePointGravity(FVector midpoint = FVector( 0, 0, -9.81 ), float magnitude = 10.0);

	void ApplyAcceleration(UParticleContext * particleContext) override;

	Vector3D Midpoint;
	double Magnitude;
};
