#pragma once

#include <unordered_map>
#include <unordered_set>
#include <math.h>

#include "Runtime/Core/Public/Async/Future.h"
#include "Runtime/Core/Public/Async/AsyncWork.h"

#include "NeighborsFinder.h"

#include "CoreMinimal.h"

#include "HashNeighborsFinder.generated.h"

UCLASS()
class UHashNeighborsFinder : public UNeighborsFinder {
	GENERATED_BODY()
public:

	~UHashNeighborsFinder() override;

	UFUNCTION(BlueprintPure, Category="NeighborhoodSearch")
	static UHashNeighborsFinder * CreateHashNeighborsFinder();

	// Adds static particles to the static hashtable
	void AddStaticParticles(TArray<UStaticBorder *>& borders, double supportLength) override;
	void AddStaticParticles(std::vector<UStaticBorder *>& borders, double supportLength) override;
	void AddStaticParticles(UStaticBorder * border, double supportLength) override;


	// Finds neighbors for particles
	void FindNeighbors(const UParticleContext& particleContext, double particleDistance, FNeighborsSearchRelations searchRelations = FNeighborsSearchRelations()) override;

	// FInds neighbors at a specified position
	FNeighborhood NeighborsOfPosition(const Vector3D& position, const UParticleContext& particleContext) const override;

	// Finds neighbors for border particles
	void FindBorderNeighbors(UStaticBorder * border, double particleDistance);

	// Inefficiently adds the ghost particles from the periodic condition to the particles neighborhoods
	void FindPeriodicNeighbors(const UParticleContext& particleContext, FNeighborsSearchRelations searchRelations);

	static int GetHash(const Vector3D& vector, double supportLength);
	static int GetHash(const Particle& particle, double supportLength);
	static int GetHash(int x, int y, int z);

	std::unordered_map<int, std::vector<FluidNeighbor>> DynamicHashtable;
	std::unordered_map<int, std::vector<StaticBorderNeighbor>> StaticHashtable;

private:

	void FillHashtableDynamic(const UParticleContext& particleContext, double supportLength);

	void RegisterNeighborsFluids(const std::vector<UFluid*>& fluids, double supportLength, FNeighborsSearchRelations searchRelations);
	void RegisterNeighborsStaticBorders(const std::vector<UStaticBorder*>& borders, double supportLength, FNeighborsSearchRelations searchRelations);
};

