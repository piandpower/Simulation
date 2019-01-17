// Fill out your copyright notice in the Description page of Project Settings.

#include "FluidSpawner.h"


// Sets default values
AFluidSpawner::AFluidSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	RestDensity = 1;
	Viscosity = 10;
	ColorVisual = EColorVisualisation::Normal;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RootComponent"));
	RootComponent = MeshComponent;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> defaultMesh(TEXT("StaticMesh'/Game/Geometry/Structures/Cube.Cube'"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> defaultMaterial(TEXT("Material'/Game/Geometry/Materials/Blue.Blue'"));

	
	if (defaultMesh.Object) {
		Mesh = defaultMesh.Object;
		MeshComponent->SetStaticMesh(Mesh);
	}
	if (defaultMaterial.Object) {
		MeshComponent->SetMaterial(0, defaultMaterial.Object);
	}

}

void AFluidSpawner::PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (Mesh != nullptr) {
		MeshComponent->SetStaticMesh(Mesh);
	}

}
