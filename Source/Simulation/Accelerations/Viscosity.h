#pragma once

#include <vector>
#include <algorithm>
#include "DataStructures/Vector3D.h"

#include "Accelerations/Acceleration.h"

#include "CoreMinimal.h"

#include "Viscosity.generated.h"

UCLASS()
class UViscosity : public UAcceleration {
	GENERATED_BODY()

public:

	~UViscosity() override;

	UFUNCTION(BlueprintPure, Category = "Accelerations")
		static UViscosity * CreateViscosity();

	void ApplyAcceleration(UParticleContext * particleContext) override;

};
