// Fill out your copyright notice in the Description page of Project Settings.

#include "LineComponent.h"


// Sets default values for this component's properties
ULineComponent::ULineComponent()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryComponentTick.bCanEverTick = false;
	

	LineParticleSystemComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("RootComponent"));

	static ConstructorHelpers::FObjectFinder<UParticleSystem> particleSystem(TEXT("ParticleSystem'/Game/Geometry/Line/Line.Line'"));


	if (particleSystem.Object) {
		LineParticleSystemComponent->Template = particleSystem.Object;
		SetStartEnd({ 0, 0, 0 }, { 1, 1, 1 });
		SetLineColor(FLinearColor(1.0, 0.f, 0.f));
		SetLineThickness(0.5f);
	}

}

void ULineComponent::SetStartEnd(FVector start, FVector end)
{
	LineParticleSystemComponent->SetVectorParameter("Source", start);
	LineParticleSystemComponent->SetVectorParameter("Target", end);
}

void ULineComponent::SetLineColor(FLinearColor color)
{
	LineParticleSystemComponent->SetColorParameter("Color", color);
}

void ULineComponent::SetLineThickness(float thickness)
{
	LineParticleSystemComponent->SetVectorParameter("Thickness", FVector(thickness, thickness, thickness));
}
