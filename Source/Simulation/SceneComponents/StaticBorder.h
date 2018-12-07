// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <cmath>
#include <vector>
#include <unordered_set>
#include <set>


#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"
#include "Runtime/Core/Public/Async/ParallelFor.h"


#include "SceneComponents/Particle.h"
#include "UnrealComponents/ParticleCloudActor.h"

#include "../Plugins/Runtime/ProceduralMeshComponent/Source/ProceduralMeshComponent/Public/KismetProceduralMeshLibrary.h"
#include "StaticBorder.generated.h"

class ASimulator;


enum class ESpawnSource {
	Line,
	Positions,
	Mesh,
	MultipleMeshes,
};

UCLASS(BlueprintType)
class SIMULATION_API UStaticBorder : public UObject
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	UStaticBorder();

	~UStaticBorder();

	void Build(UParticleContext * particleContext, ASimulator * simulator);

	UFUNCTION(BlueprintPure)
		static UStaticBorder * CreateStaticBorderFromLine(FVector start, FVector end, float borderDensityFactor = 1.0, float borderStiffness = 1.0, float borderVolumeFactor = 1.0, float borderViscosity = 0.04, FVector ghostVelocity = FVector( 0, 0, 0 ));

	UFUNCTION(BlueprintPure)
		static UStaticBorder * CreateStaticBorderFromPositions(TArray<FVector> positions, FTransform transform, float borderDensityFactor = 1.0, float borderStiffness = 1.0, float borderVolumeFactor = 1.0, float borderViscosity = 0.04);

	UFUNCTION(BlueprintPure)
		static UStaticBorder * CreateStaticBorderFromMesh(UStaticMesh * mesh, FTransform transform, float borderDensityFactor = 1.0, float borderStiffness = 1.0, float borderVolumeFactor = 1.0, float borderViscosity = 0.04);

	UFUNCTION(BlueprintPure)
		static UStaticBorder * CreateStaticBorderFromMultipleMeshes(TArray<UStaticMesh *> meshes, TArray<FTransform> transforms, float borderDensityFactor = 1.0, float borderStiffness = 1.0, float borderVolumeFactor = 1.0, float borderViscosity = 0.04);

	UFUNCTION(BlueprintPure)
		static UStaticBorder * CreateStaticBorderFromStaticMeshActors(TArray<AStaticMeshActor *> meshActors, float borderDensityFactor = 1.0, float borderStiffness = 1.0, float borderVolumeFactor = 1.0, float borderViscosity = 0.04);

	void WriteStaticBorderToFile(std::string file);


	// Calculate masses only depending on the own borderparticles
	// Masses are scaled with restDensity = 1. Other Densities need scaling!
	void CalculateMasses();

	ASimulator * Simulator;

	UFUNCTION(BlueprintPure)
	UParticleContext * GetParticleContext() const;
	
	UFUNCTION(BlueprintPure)
	int GetNumParticles() const;

	ASimulator * GetSimulator() const;

protected:

	UParticleContext * ParticleContext;

	void AddPositionsFromTriangle(std::unordered_set<FVector> & positions, FVector a, FVector b, FVector c, float particleDistance, bool doubleThickness = false);

	void RemoveCloseNeighbors(std::unordered_set<FVector> & positions, double minDistance);

	ESpawnSource SpawnSource;

	// parameters and functions for Building
	FVector Start, End, GhostVelocity;
	TArray<FVector> Positions;
	TArray<FTransform> Transforms;
	TArray<UStaticMesh *> Meshes;
	void BuildStaticBorderFromMultipleMeshes();
	void BuildStaticBorderFromPositions();
	void BuildStaticBorderFromLine();


public:

	int Index;
	operator int();

	UPROPERTY(BlueprintReadWrite)
		TEnumAsByte<EColorVisualisation> ColorCode;

	std::unique_ptr<std::vector<Particle>> Particles;

	double BorderDensityFactor;

	double BorderStiffness;

	double BorderViscosity;

	double BorderVolumeFactor;

};
