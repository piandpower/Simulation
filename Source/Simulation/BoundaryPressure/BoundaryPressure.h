#pragma once

#include "SceneComponents/Particle.h"
#include "ParticleContext/ParticleContext.h"
#include "Kernels/Kernel.h"
#include "NeighborsFinders/NeighborsFinder.h"

#include "CoreMinimal.h"

#include "BoundaryPressure.generated.h"

UCLASS()
class UBoundaryPressure : public UObject {
	GENERATED_BODY()

public:

	virtual void ComputeAllPressureValues(UParticleContext * particleContext, UKernel * kernel);
	virtual double GetPressureValue(const Particle& staticBoundaryParticle, const Particle& neighboringFluidParticle) const;

	void Build(const EDimensionality& dimensionality);

	// Some pressure computations for boundary like MLS requires the computation of static border neighbors
	virtual FNeighborsSearchRelations GetRequiredNeighborhoods() const;

protected:

	EDimensionality Dimensionality;
};
