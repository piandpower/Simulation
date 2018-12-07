// Copyright 2018 Michal Cieciura. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PointCloudShared.h"

class UMaterial;
class UMaterialInterface;
class UPointCloud;
struct FExpressionInput;

/**
 * Holds helper functions for dealing with point clouds.
 * Mostly import and processing related functionality
 */
class POINTCLOUDRUNTIME_API FPointCloudHelper
{
public:
	/**
	 * Attempts to parse the given data as BIN input.
	 * Outputs arrays for Locations and Colors (if available)
	 * Returns true if the parsing was successful
	 */
	static bool ImportAsBIN(const uint8*& Buffer, TArray<FPointCloudPoint>& OutPoints, EPointCloudColorMode &ColorMode, uint32 FirstIndex, uint32 LastIndex);

	/**
	 * Attempts to parse the given data as LAS input.
	 * Outputs arrays for Locations and Colors (if available)
	 * Returns true if the parsing was successful
	 */
	static bool ImportAsLAS(const uint8*& Buffer, TArray<FPointCloudPoint>& OutPoints, EPointCloudColorMode &ColorMode, uint32 FirstIndex, uint32 LastIndex);

	/**
	 * Attempts to parse the given data as Text input.
	 * Outputs arrays for Locations and Colors (if available).
	 * Returns true if the parsing was successful.
	 */
	static bool ImportAsText(const FString& Filename, TArray<FPointCloudPoint>& OutPoints, EPointCloudColorMode &ColorMode, uint32 FirstIndex, uint32 LastIndex, FPointCloudFileHeader PointCloudFileHeader);

	/** Reads and parses header information about the given file. */
	static FPointCloudFileHeader ReadFileHeader(const FString& Filename);

	/** Scans the given file and searches for Min and Max values in the given columns. */
	static FVector2D ReadFileMinMaxColumn(const FString& Filename, int32 ColumnIndex);
	static FVector2D ReadFileMinMaxColumns(const FString& Filename, TArray<int32> Columns);
	static FVector2D ReadFileMinMaxColumns(const uint8 *DataPtr, uint8 Delimiter, TArray<int32> Columns);

	/** Reduces the density of the points using provided settings. */
	static void DensityReduction(TArray<FPointCloudPoint>& Points, const float MinDistanceBetweenPoints);

	/** Attempts to reduce the noise in the point cloud using settings provided. */
	static void NoiseReduction(TArray<FPointCloudPoint>& Points, const float MaxDistanceBetweenPoints, const int32 MinPointDensity);

	/**
	 * Transforms the point cloud using settings provided.
	 * Returns original offset location.
	 */
	static FVector Transform(TArray<FPointCloudPoint>& Points, const EPointCloudOffset Offset, const FVector Translation, const FVector Scale, bool bUseLowPrecision);

	/**
	 * Splits the given points into sections of specified size.
	 * Can specify minimum number of points the section needs to contain to not be rejected
	 * Returns the array of chunks the points were split into.
	 */
	static TArray<TArray<FPointCloudPoint*>> SplitIntoSections(TArray<FPointCloudPoint*>& Points, const FVector SectionSize, int32 MinSectionCount = 1, int32 MaxSectionCount = 0);
	static TArray<TArray<FPointCloudPoint*>> SplitIntoSections(TArray<FPointCloudPoint>& Points, const FVector SectionSize, int32 MinSectionCount = 1, int32 MaxSectionCount = 0);

	/** Adjusts the Section Size if necessary */
	static FVector AdjustSectionSize(TArray<FPointCloudPoint>& Points, const FVector SectionSize);

	/** Returns the number of enabled points inside the provided set */
	static int32 CountEnabledPoints(const TArray<FPointCloudPoint*>& Points);
	static int32 CountEnabledPoints(const TArray<FPointCloudPoint>& Points);

	/** Returns the enabled points inside the provided set */
	static TArray<FPointCloudPoint*> GetEnabledPoints(const TArray<FPointCloudPoint*>& Points);

	/** Returns bounds of the provided set */
	static FBox CalculateBounds(const TArray<FPointCloudPoint*>& Points);
	static FBox CalculateBounds(const TArray<FPointCloudPoint>& Points);
	static FBox CalculateBounds(const TArray<FPointCloudPoint*>& Points, const FTransform& Transform);
	static FBox CalculateBounds(const TArray<FPointCloudPoint>& Points, const FTransform& Transform);

	/** Returns VRAM used by a single point, in bytes */
	static int32 CalculatePointSize(UPointCloud *PointCloud, bool bIncludeIB = true);

	/** Returns Color Mode as string */
	static FString GetColorModeAsString(class UPointCloud *PointCloud);

#if WITH_EDITOR
	/** Returns true if Material uses UV expression */
	static bool MaterialUsesUV(UMaterialInterface *Material);
	static bool MaterialUsesUV(UMaterial *Material);

private:
	static UMaterial* GetTopMostMaterial(UMaterialInterface *Material);
	static bool MaterialContainsExpressionOfClass(UMaterial *Material, UClass* ExpressionClass);
	static bool ExpressionInputContainsExpressionOfClass(FExpressionInput *Expression, UClass* ExpressionClass);
#endif

private:
	static void AdjustBounds(FBox &Bounds);
};