#pragma once

#include "DataStructures/Vector3D.h"

#include "Accelerations/Acceleration.h"

#include "CoreMinimal.h"

#include "SurfaceTension.generated.h"


// Applies damping to fluid particles that have equal or less neighbors than specified by the treshold
UCLASS()
class USurfaceTension : public UAcceleration {
	GENERATED_BODY()

public:

	~USurfaceTension() override;

	UFUNCTION(BlueprintPure, Category = "Accelerations")
		static USurfaceTension * CreateSurfaceTension(float surfaceTensionFactor = 0.01);

	void ApplyAcceleration(UParticleContext * particleContext) override;

	double SurfaceTensionFactor;

};
