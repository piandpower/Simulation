#pragma once

#include "BoundaryPressure.h"
#include "CoreMinimal.h"

#include "Mirroring.generated.h"

UCLASS()
class UMirroring : public UBoundaryPressure {
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure, Category = "BoundaryPressure")
	static UMirroring * CreateMirroringPressureComputation();

	~UMirroring() override;

	// Does nothing if pressure mirroring is used 
	void ComputeAllPressureValues(UParticleContext * particleContext, UKernel * kernel) override;

	// Concept of mirroring: returns the same pressure value as the neighboring fluid particle
	double GetPressureValue(const Particle& staticBoundaryParticle, const Particle& neighboringFluidParticle) const override;

};
