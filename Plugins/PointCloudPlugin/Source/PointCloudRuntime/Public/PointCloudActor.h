// Copyright 2018 Michal Cieciura. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "PointCloudActor.generated.h"

class UPointCloud;

UCLASS(BlueprintType)
class POINTCLOUDRUNTIME_API APointCloudActor : public AActor
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Components")
	USceneComponent *Root;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Point Cloud", meta = (AllowPrivateAccess = "true"))
	UPointCloud *PointCloud;

public:
	/** Controls whether the cloud should cast shadows in the case of non precomputed shadowing. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Lighting, meta=(DisplayName = "Dynamic Shadow"))
	bool bCastDynamicShadow;

private:
	TArray<class UPointCloudComponent*> PCCs;

public:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Point Cloud")
	void SetPointCloud(UPointCloud *InPointCloud);

	UFUNCTION(BlueprintCallable, Category = "Point Cloud")
	UPointCloud* GetPointCloud() const;

	UFUNCTION(BlueprintCallable, Category = "Point Cloud")
	void SetCastDynamicShadow(bool NewCastShadow);

#if WITH_EDITOR
	virtual bool ShouldTickIfViewportsOnly() const override { return true; }

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void PostLoad() override;

	virtual void PreEditChange(UProperty* PropertyThatWillChange) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
	void RebuildComponents();
	void OnPointCloudRebuilt();
};