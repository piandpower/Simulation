#pragma once

#include "Particles/Particle.h"
#include "CoreMinimal.h"

#include "Solver/BoundaryPressure/BoundaryPressure.h"

#include "PressureGradient.generated.h"

enum EDimensionality;
class USolver;
class UKernel;

// Interface for all pressure gradient computation methods
UCLASS()
class UPressureGradient : public UObject {
	GENERATED_BODY()

public:

	virtual Vector3D ComputePressureGradient(Particle& f, int particleIndex) const;

	virtual void PrecomputeAllGeometryData(const UParticleContext& particleContext);

	void Build(const USolver * solver, const EDimensionality& dimensionality);

protected:

	const UKernel& GetKernel() const;
	const UBoundaryPressure& GetBoundaryPressure() const;

	const USolver* Solver;

	EDimensionality Dimensionality;
};
