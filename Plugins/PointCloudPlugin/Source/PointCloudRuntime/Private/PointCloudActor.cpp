// Copyright 2018 Michal Cieciura. All Rights Reserved.

#include "PointCloudActor.h"
#include "PointCloudComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "PointCloudSection.h"
#include "PointCloud.h"


#if WITH_EDITOR
#include "EditorViewportClient.h"
#include "Editor.h"
#endif

APointCloudActor::APointCloudActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	RootComponent = Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void APointCloudActor::BeginPlay()
{
	Super::BeginPlay();
	RebuildComponents();
}

void APointCloudActor::SetPointCloud(UPointCloud *InPointCloud)
{
	if (PointCloud && PointCloud->IsValidLowLevel())
	{
		PointCloud->OnPointCloudRebuilt().RemoveAll(this);
	}

	PointCloud = InPointCloud;

	if (PointCloud)
	{
		PointCloud->OnPointCloudRebuilt().AddUObject(this, &APointCloudActor::OnPointCloudRebuilt);
	}

	RebuildComponents();
}

UPointCloud* APointCloudActor::GetPointCloud() const
{
	return PointCloud;
}

void APointCloudActor::SetCastDynamicShadow(bool NewCastShadow)
{
	bCastDynamicShadow = NewCastShadow;

	for (UPointCloudComponent* Component : PCCs)
	{
		if (IsValid(Component))
		{
			Component->SetCastShadow(bCastDynamicShadow);
		}
	}
}

#if WITH_EDITOR
void APointCloudActor::OnConstruction(const FTransform& Transform)
{
	// We only want the rebuild to happen on initial load
	if (PointCloud && !PointCloud->IsDirty() && PCCs.Num() == 0)
	{
		RebuildComponents();
	}
}

void APointCloudActor::PostLoad()
{
	Super::PostLoad();

	if (PointCloud)
	{
		PointCloud->OnPointCloudRebuilt().AddUObject(this, &APointCloudActor::OnPointCloudRebuilt);
	}
}

void APointCloudActor::PreEditChange(UProperty* PropertyThatWillChange)
{
	Super::PreEditChange(PropertyThatWillChange);

	if (PropertyThatWillChange && PropertyThatWillChange->GetName().Equals("PointCloud"))
	{
		if (PointCloud)
		{
			PointCloud->OnPointCloudRebuilt().RemoveAll(this);
		}
	}
}
void APointCloudActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.MemberProperty)
	{
		if (PropertyChangedEvent.MemberProperty->GetName().Equals("PointCloud"))
		{
			if (PointCloud)
			{
				PointCloud->OnPointCloudRebuilt().AddUObject(this, &APointCloudActor::OnPointCloudRebuilt);
			}

			RebuildComponents();
		}

		if (PropertyChangedEvent.MemberProperty->GetName().Equals("bCastDynamicShadow"))
		{
			SetCastDynamicShadow(bCastDynamicShadow);
		}
	}
}
#endif

void APointCloudActor::RebuildComponents()
{
	for (UPointCloudComponent* Component : PCCs)
	{
		if (IsValid(Component))
		{
			Component->DestroyComponent();
		}
	}
	PCCs.Empty();

	if (PointCloud && !PointCloud->IsDirty())
	{
		for (int i = 0; i < PointCloud->Sections.Num(); i++)
		{
			UPointCloudComponent *Component = NewObject<UPointCloudComponent>(this);

			if (IsValid(Component))
			{
				Component->RegisterComponent();
				Component->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
				Component->CastShadow = bCastDynamicShadow;
				Component->SetSection(PointCloud->Sections[i]);

				PCCs.Add(Component);
			}
		}
	}
}
void APointCloudActor::OnPointCloudRebuilt()
{
	RebuildComponents();
}