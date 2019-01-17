#pragma once

#include <vector>
#include "ParticleContext/ParticleContext.h"

#include "CoreMinimal.h"
#include "Runtime/Core/Public/Async/ParallelFor.h"

#include "NeighborsFinder.generated.h"

UENUM(BlueprintType)
enum ENeighborhoodSearch {
	Naive,
	SpatialHash
};

USTRUCT(BlueprintType)
struct FNeighborsSearchRelations {
	GENERATED_BODY()
	bool FluidNeighborsOfFluidRequired;
	bool StaticBorderNeighborsOfFluidRequired;

	bool FluidNeighborsOfStaticBorderRequired;
	bool StaticBorderNeighborsOfStaticBorderRequired;

	FNeighborsSearchRelations() :
		FluidNeighborsOfFluidRequired(true),
		StaticBorderNeighborsOfFluidRequired(true),
		FluidNeighborsOfStaticBorderRequired(false),
		StaticBorderNeighborsOfStaticBorderRequired(false)
	{
	}

	FNeighborsSearchRelations(bool fluidNeighborsOfFluid, bool staticBorderNeighborsOfFluid, bool fluidNeighborsOfStaticBorder, bool staticBorderNeighborsOfStaticBorder) :
		FluidNeighborsOfFluidRequired(fluidNeighborsOfFluid),
		StaticBorderNeighborsOfFluidRequired(staticBorderNeighborsOfFluid),
		FluidNeighborsOfStaticBorderRequired(fluidNeighborsOfStaticBorder),
		StaticBorderNeighborsOfStaticBorderRequired(staticBorderNeighborsOfStaticBorder)
	{
	}

	bool FluidNeedsNeighbors() const {
		return FluidNeighborsOfFluidRequired || StaticBorderNeighborsOfFluidRequired;
	};

	bool StaticBorderNeedsNeighbors() const {
		return FluidNeighborsOfStaticBorderRequired || StaticBorderNeighborsOfStaticBorderRequired;
	};

	bool FluidNeighborsRequired() const {
		return FluidNeighborsOfFluidRequired || FluidNeighborsOfStaticBorderRequired;
	};

	bool StaticBorderNeighborsRequired() const {
		return StaticBorderNeighborsOfFluidRequired || FluidNeighborsOfStaticBorderRequired;
	}

};

struct FNeighborhood {
	std::vector<FluidNeighbor> FluidNeighbors;
	std::vector<StaticBorderNeighbor> StaticBorderNeighbors;
};

UCLASS()
class UNeighborsFinder : public UObject {
	GENERATED_BODY()
public:

	virtual ~UNeighborsFinder();

	void Build(double supportRange);

	virtual void FindNeighbors(const UParticleContext& particleContext, double supportLength, FNeighborsSearchRelations searchRelations = FNeighborsSearchRelations());
	virtual FNeighborhood NeighborsOfPosition(const Vector3D& position, const UParticleContext& particleContext) const;
	virtual void AddStaticParticles(UStaticBorder * borders, double supportRange);
	virtual void AddStaticParticles(TArray<UStaticBorder*>& borders, double supportRange);
	virtual void AddStaticParticles(std::vector<UStaticBorder*>& borders, double supportRange);


	ENeighborhoodSearch GetNeighborsFinderType();

protected:

	// Support range in particle Units. Scales how far the neighborhoos is computed
	double SupportRange;

	ENeighborhoodSearch NeighborsFinderType;
};