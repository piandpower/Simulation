// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"

#include "LineComponent.generated.h"


UCLASS(BlueprintType)
class SIMULATION_API ULineComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	ULineComponent();

protected:


	UParticleSystemComponent * LineParticleSystemComponent;

public:	

	// Sets the start and the end of the beam in world space.
	UFUNCTION(BlueprintCallable)
	void SetStartEnd(FVector start, FVector end);

	UFUNCTION(BlueprintCallable)
	void SetLineColor(FLinearColor color);
	
	UFUNCTION(BlueprintCallable)
	void SetLineThickness(float thickness = 0.5f);
	
};
