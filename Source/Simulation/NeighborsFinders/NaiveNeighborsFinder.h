#pragma once

#include "NeighborsFinder.h"

#include "CoreMinimal.h"

#include "NaiveNeighborsFinder.generated.h"

UCLASS()
class UNaiveNeighborsFinder : public UNeighborsFinder {
	GENERATED_BODY()
public:

	UNaiveNeighborsFinder();

	~UNaiveNeighborsFinder() override;

	UFUNCTION(BlueprintPure, Category = "NeighborhoodSearch")
	static UNaiveNeighborsFinder * CreateNaiveNeighborsFinder();
	
	void FindNeighbors(const UParticleContext& particleContext, double supportLength, FNeighborsSearchRelations searchRelations = FNeighborsSearchRelations()) override;
	FNeighborhood NeighborsOfPosition(const Vector3D& position, const UParticleContext& particleContext) const override;

	void AddStaticParticles(UStaticBorder * borders, double supportLength) override;

	static UNaiveNeighborsFinder * CreateNaiveNeighborhoodSearch();
};