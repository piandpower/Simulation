// Copyright 2018 Michal Cieciura.All Rights Reserved.

#include "ActorFactoryPointCloud.h"
#include "PointCloudActor.h"
#include "PointCloud.h"

#define LOCTEXT_NAMESPACE "ActorFactory"

UActorFactoryPointCloud::UActorFactoryPointCloud(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DisplayName = LOCTEXT("PointCloudDisplayName", "Point Cloud");
	NewActorClass = APointCloudActor::StaticClass();
	bUseSurfaceOrientation = true;
}

bool UActorFactoryPointCloud::CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg)
{
	if (!AssetData.IsValid() || !AssetData.GetClass()->IsChildOf(UPointCloud::StaticClass()))
	{
		OutErrorMsg = NSLOCTEXT("CanCreateActor", "NoPointCloud", "A valid point cloud must be specified.");
		return false;
	}

	return true;
}

void UActorFactoryPointCloud::PostSpawnActor(UObject* Asset, AActor* NewActor)
{
	Super::PostSpawnActor(Asset, NewActor);

	UPointCloud* PointCloud = CastChecked<UPointCloud>(Asset);

	UE_LOG(LogActorFactory, Log, TEXT("Actor Factory created %s"), *PointCloud->GetName());

	// Change properties
	APointCloudActor* PointCloudActor = CastChecked<APointCloudActor>(NewActor);
	PointCloudActor->SetPointCloud(PointCloud);
}

UObject* UActorFactoryPointCloud::GetAssetFromActorInstance(AActor* Instance)
{
	check(Instance->IsA(NewActorClass));
	APointCloudActor* PCA = CastChecked<APointCloudActor>(Instance);

	return PCA->GetPointCloud();
}

void UActorFactoryPointCloud::PostCreateBlueprint(UObject* Asset, AActor* CDO)
{
	if (Asset != NULL && CDO != NULL)
	{
		UPointCloud* PointCloud = CastChecked<UPointCloud>(Asset);
		APointCloudActor* PointCloudActor = CastChecked<APointCloudActor>(CDO);

		PointCloudActor->SetPointCloud(PointCloud);
	}
}

FQuat UActorFactoryPointCloud::AlignObjectToSurfaceNormal(const FVector& InSurfaceNormal, const FQuat& ActorRotation) const
{
	// Meshes align the Z (up) axis with the surface normal
	return FindActorAlignmentRotation(ActorRotation, FVector(0.f, 0.f, 1.f), InSurfaceNormal);
}

#undef LOCTEXT_NAMESPACE