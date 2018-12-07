// Copyright 2018 Michal Cieciura. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
//#include "UObject/ObjectMacros.h"
#include "Components/MeshComponent.h"
#include "PointCloudComponent.generated.h"

class FPointCloudSceneProxy;
class FPointCloudSection;

/** Component that allows you to render specified point cloud section */
UCLASS(ClassGroup=Rendering, hidecategories = (Object, LOD, Physics, Collision, Materials))
class POINTCLOUDRUNTIME_API UPointCloudComponent : public UMeshComponent
{
	GENERATED_UCLASS_BODY()
		
private:
	FPointCloudSection *Section;

public:
	FPointCloudSection* GetSection() const { return Section; }
	void SetSection(FPointCloudSection *InSection);

	// Begin UActorComponent Interface
	virtual void DestroyComponent(bool bPromoteChildren = false) override;
	// End UActorComponent Interface

	// Begin UObject Interface.
	static void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector);
	// End UObject Interface.

private:
	// Begin UPrimitiveComponent Interface
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	virtual int32 GetNumMaterials() const override { return 1; }
	virtual UMaterialInterface* GetMaterial(int32 ElementIndex) const override;
	// End UMeshComponent Interface

	// Begin USceneComponent Interface
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	// End USceneComponent Interface

	friend class FPointCloudSceneProxy;
};


