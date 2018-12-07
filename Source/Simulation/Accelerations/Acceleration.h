#pragma once


#include "SceneComponents/Particle.h"
#include "SceneComponents/Fluid.h"
#include "Kernels/Kernel.h"
#include "NeighborsFinders/NeighborsFinder.h"
#include "ParticleContext/ParticleContext.h"

#include "Runtime/Core/Public/Async/ParallelFor.h"

#include "Acceleration.generated.h"

class USolver;

// Abstract parent class of all forces
UCLASS(BlueprintType)
class UAcceleration : public UObject {
	GENERATED_BODY()
public:

	virtual void ApplyAcceleration(UParticleContext * particleContext);

	void Build(USolver * solver);

protected:
	UKernel * GetKernel();

	USolver * Solver;
};