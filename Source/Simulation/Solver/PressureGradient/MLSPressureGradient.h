#pragma once

#include "PressureGradient.h"
#include "CoreMinimal.h"
#include "Solver/BoundaryPressure/MLSExtrapolation.h"

#include "MLSPressureGradient.generated.h"

// The MLS way of computing pressure gradient
UCLASS()
class UMLSPressureGradient : public UPressureGradient {
	GENERATED_BODY()

public:

	Vector3D ComputePressureGradient(Particle& f, int particleIndex) const override;

	UFUNCTION(BlueprintPure)
	static UMLSPressureGradient * CreateMLSPressureGradient(FEpsilons nanoEpsilons);

protected:

	double Epsilon1D;
	double Epsilon2D;
	double Epsilon3D;

};
