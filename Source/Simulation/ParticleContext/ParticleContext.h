// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <vector>

#include "ParticleContext/SceneComponents/Fluid.h"
#include "ParticleContext/SceneComponents/StaticBorder.h"

#include "Periodic/PeriodicCondition.h"

#include "UnrealComponents/ParticleCloudActor.h"
#include "CoreMinimal.h"

#include "ParticleContext.generated.h"

enum EDimensionality;
class UPeriodicCondition;

UCLASS(BlueprintType)
class UParticleContext : public UObject
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintPure, Category = "Particles")
	static UParticleContext * CreateParticleContext(float particleDistance, TArray<UFluid*> fluids, TArray<UStaticBorder*> staticBorders, FVisualisationInformation visualisationInformation, UPeriodicCondition * periodicCondition = nullptr);
	static UParticleContext * CreateParticleContext(double particleDistance, std::vector<UFluid*> fluids, std::vector<UStaticBorder*> staticBorders, FVisualisationInformation visualisationInformation, UPeriodicCondition * periodicCondition = nullptr);

	void Build(UWorld * world, ASimulator * simulator, EDimensionality dimensionality);

	const std::vector<UFluid*>& GetFluids() const;
	const std::vector<UStaticBorder*>& GetStaticBorders() const;

	UFUNCTION(BlueprintPure)
	TArray<UFluid*> GetFluidsBlueprint() const;
	UFUNCTION(BlueprintPure)
	TArray<UStaticBorder*> GetStaticBordersBlueprint() const;

	UFUNCTION(BlueprintCallable)
	void AddFluid(UFluid* fluid);

	UFUNCTION(BlueprintCallable)
	void AddStaticBorder(UStaticBorder* staticBorder);

	UFUNCTION(BlueprintCallable)
	void RemoveFluid(UFluid* fluid);
	void RemoveFluid(int index);

	UFUNCTION(BlueprintCallable)
	void RemoveStaticBorder(UStaticBorder* staticBorder);
	void RemoveStaticBorder(int index);

	double GetParticleDistance() const;

	UFUNCTION(BlueprintPure)
	float GetParticleDistanceBlueprint() const;

	void UpdateVisual();

	ASimulator * GetSimulator() const;

	AParticleCloudActor * GetParticleVisualiser() const;

	UPeriodicCondition * GetPeriodicCondition() const;

	UFUNCTION(BlueprintCallable)
	void SetVisualisationInformation(FVisualisationInformation visualisationInformation);

	UFUNCTION(BlueprintPure)
	FVisualisationInformation GetVisualisationInformation() const;

protected:
	std::vector<UFluid*> Fluids;
	std::vector<UStaticBorder*> StaticBorders;

	double ParticleDistance;

	UPeriodicCondition * PeriodicCondition = nullptr;

	AParticleCloudActor * ParticleVisualiser = nullptr;

	// User specified informations about how the particles should be colored
	FVisualisationInformation VisualisationInformation;
	ASimulator * Simulator;
};
