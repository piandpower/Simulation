#pragma once

#include "DataStructures/Matrix3D.h"

#include "BoundaryPressure.h"
#include "CoreMinimal.h"

#include "MLSExtrapolation.generated.h"

USTRUCT(BlueprintType)
struct FEpsilons {
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
	float OneDimension = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
	float TwoDimensions = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
	float ThreeDimensions = 0.001f;
};

UCLASS()
class UMLSExtrapolation : public UBoundaryPressure {
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure, Category = "BoundaryPressure")
	static UMLSExtrapolation * CreateBoundaryMLSExtrapolationPressureComputation(FEpsilons nanoEpsilon);

	~UMLSExtrapolation() override;

	// Does nothing if pressure mirroring is used 
	void ComputeAllPressureValues(UParticleContext * particleContext, UKernel * kernel) override;

	// Concept of mirroring: returns the same pressure value as the neighboring fluid particle
	double GetPressureValue(const Particle& staticBoundaryParticle, const Particle& neighboringFluidParticle) const override;

	FNeighborsSearchRelations GetRequiredNeighborhoods() const override;

protected:
	double Epsilon1D;
	double Epsilon2D;
	double Epsilon3D;

};
