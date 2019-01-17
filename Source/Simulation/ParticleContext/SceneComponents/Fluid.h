// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <vector>
#include <algorithm>
#include <functional>

#include "Kismet/KismetMathLibrary.h"
#include "CoreMinimal.h"
#include "Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"
#include "Particles/Particle.h"
#include "DataStructures/Vector3D.h"
#include "UnrealComponents/ParticleCloudActor.h"
#include "Classes/Engine/StaticMesh.h"

#include "Fluid.generated.h"

enum EDimensionality;

UCLASS(BlueprintType)
class SIMULATION_API UFluid : public UObject
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	UFluid();

	~UFluid();

	void InitializeBoxFluid(float particleDistance, FTransform transform, FVector initialVelocity, float fluidDensity, float viscosity, float massFactor, EDimensionality dimensionality, bool displaceParticles = true);
	void InitializeFluidFromPositionsAndVelocities(TArray<Vector3D>& positions, TArray<Vector3D>& velocities, float fluidDensity, float viscosity, EDimensionality dimensionality);
	void InitializeFluidFromPositionsVelocitiesAndMasses(TArray<Vector3D>& positions, TArray<Vector3D>& velocities, TArray<double> masses, float fluidDensity, float viscosity);
	
	UFUNCTION(BlueprintPure)
	static UFluid * CreateFluidFromSpawner(AFluidSpawner * fluidSpawner);

	UFUNCTION(BlueprintPure)
	static UFluid * CreateFluidFromFile(FString fluidFile);

	UFUNCTION(BlueprintPure)
	static UFluid * CreateFluidFromPositionsAndVelocities(TArray<FVector> positions, TArray<FVector> velocities, float restDensity, float viscosity, float massFactor = 1.0f);

	void Build(UParticleContext * particleContext, EDimensionality dimensionality);

	UFUNCTION(BlueprintCallable)
	static UFluid * ReadFluidFromFileBlueprint(FString file, UFluid * fluid);
	static UFluid * ReadFluidFromFile(std::string file, UFluid * fluid);

	
		void WriteFluidToFile(const std::string file, int IterationCount = 0);
		
	UFUNCTION(BlueprintCallable)
		TArray<FVector> GetAllParticlePositions();

	void AddParticle(const Particle& particle);

	UFUNCTION(BlueprintCallable)
		void AddParticle(FVector position, FVector velocity);

	void AddParticles(const std::vector<FVector>& positions, const std::vector<FVector>& velocities);

	bool RemoveParticle(int index); 
	bool RemoveParticle(Particle & particle);
	int RemoveParticles(std::function<bool(const Particle&)> criteria);

	int GetNumParticles() const;

	// Computes masses of fluid particles based on the particle distance, fluid density and the mass factor.
	void ComputeMasses(EDimensionality dimensionality, double particleDistance, double fluidDensity);

protected:

	friend class UParticleContext;
	UParticleContext * ParticleContext;

	// spawn parameters
	// wheather the fluid comes from a file or a box spawner
	enum class SpawnedFrom {
		Box,
		File,
		PositionsAndVelocities
	};
	SpawnedFrom SpawnSource;
	FString FluidFile;
	// The fluid spawner from which the fluid is created
	AFluidSpawner * FluidSpawner;
	TArray<Vector3D> SpawnPositions;
	TArray<Vector3D> SpawnVelocities;


	double CurrentAverageDensityError;
	double CurrentPotentialEnergy;
	double CurrentKineticEnergy;

	double Viscosity;
	double RestDensity;

	// Used to scale the mass of the particles. This results in more particles in a neighborhood
	double MassFactor;

public:

	UFUNCTION(BlueprintPure)
		UParticleContext * GetParticleContext() const;

	// The position the fluid is located in the fluid vector of the particle context. Required to locate particle entries in the solvers by fluid and particle index
	int Index;

	operator int();

	std::unique_ptr<std::vector<Particle>> Particles;

	double GetRestVolume() const;
	double GetRestDensity() const;
	double GetMassFactor() const;
	double GetViscosity() const;
	
	void CalculateEnergies(bool calculateEnergies = true);
	double CalculatePotentialEnergy();
	double CalculateKineticEnergy();	
	
	double GetPotentialEnergy() const;
	double GetKineticEnergy() const;
	double GetTotalEnergy() const;

	UFUNCTION(BlueprintPure)
		float GetPotentialEnergyBlueprint() const;

	UFUNCTION(BlueprintPure)
		float GetKineticEnergyBlueprint() const;

	UFUNCTION(BlueprintPure)
		float GetTotalEnergyBlueprint() const;
};
