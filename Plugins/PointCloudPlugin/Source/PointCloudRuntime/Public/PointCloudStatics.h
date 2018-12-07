// Copyright 2018 Michal Cieciura. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PointCloudStatics.generated.h"

class UPointCloud;
class UPointCloudImportSettings;

/**
 * Provides convenient set of Blueprint exposed functions
 */
UCLASS()
class POINTCLOUDRUNTIME_API UPointCloudStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Point Cloud")
	static UPointCloud* LoadPointCloudFromFile(FString Filename, int32 FirstLine, int32 LastLine, FVector2D RGBRange, FPointCloudImportSettingsColumns Columns);
	static UPointCloud* LoadPointCloudFromFile_Full(FString Filename, UPointCloudImportSettings *ImportSettings, UObject* InParent, FName InName, EObjectFlags Flags);

	/** Location prior to transformation applied */
	UFUNCTION(BlueprintPure, Category = "Point Cloud")
	static FORCEINLINE FVector GetOriginalLocation(FPointCloudPoint InPoint) { return InPoint.OriginalLocation; }

	/** Location after transformation applied */
	UFUNCTION(BlueprintPure, Category = "Point Cloud")
	static FORCEINLINE FVector GetLocation(FPointCloudPoint InPoint) { return InPoint.Location; }

	UFUNCTION(BlueprintPure, Category = "Point Cloud")
	static FORCEINLINE FColor GetColor(FPointCloudPoint InPoint) { return InPoint.Color; }

	UFUNCTION(BlueprintPure, Category = "Point Cloud")
	static FORCEINLINE bool IsEnabled(FPointCloudPoint InPoint) { return InPoint.bEnabled; }

	/** Location prior to transformation applied */
	UFUNCTION(BlueprintCallable, Category = "Point Cloud")
	static void SetOriginalLocation(UPARAM(ref) FPointCloudPoint &InPoint, FVector NewOriginalLocation) { InPoint.OriginalLocation = NewOriginalLocation; }

	UFUNCTION(BlueprintCallable, Category = "Point Cloud")
	static void SetColor(UPARAM(ref) FPointCloudPoint &InPoint, FColor NewColor) { InPoint.Color = NewColor; }

	UFUNCTION(BlueprintCallable, Category = "Point Cloud")
	static void SetEnabled(UPARAM(ref) FPointCloudPoint &InPoint, bool NewEnabled) { InPoint.bEnabled = NewEnabled; }
};