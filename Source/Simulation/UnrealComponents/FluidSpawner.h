// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"
#include "GameFramework/Actor.h"
#include "SceneComponents/Fluid.h"

#include "FluidSpawner.generated.h"

UCLASS()
class SIMULATION_API AFluidSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFluidSpawner();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector InitialVelocity;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float RestDensity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float MassFactor = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float Viscosity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool IsTwoDimensional;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool DisplaceFluidParticles;

	// for later, if we support different meshes to spawn
		UStaticMesh * Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TEnumAsByte<EColorVisualisation> ColorVisual;

	UStaticMeshComponent * MeshComponent;

	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

};
