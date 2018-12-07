// Copyright 2018 Michal Cieciura. All Rights Reserved.

// Set to 1 to enable low precision.
// Warning: It will cause an extra VertexFactory to be compiled.
#ifndef WITH_LOW_PRECISION
	#define WITH_LOW_PRECISION 1
#endif

#pragma once

#include "CoreMinimal.h"
#include "PointCloudShared.generated.h"

#define BOOL2STR(v) (v ? TEXT("True") : TEXT("False"))
#define PC_LOG(Format, ...) UE_LOG(LogTemp, Warning, TEXT(Format), __VA_ARGS__)
#define PC_ERROR(Format, ...) UE_LOG(LogTemp, Error, TEXT(Format), __VA_ARGS__)

UENUM(BlueprintType)
enum class EPointCloudOffset : uint8
{
	None,
	Center,
	FirstPoint
};

UENUM(BlueprintType)
enum class EPointCloudColorMode : uint8
{
	None,
	Intensity,
	RGB
};

/**
 * Temporary internal data storage struct.
 * @TODO: To be removed and replaced by something more optimized, as this one potentially wastes a lot of memory.
 */
USTRUCT(BlueprintType)
struct POINTCLOUDRUNTIME_API FPointCloudPoint
{
	GENERATED_BODY()

	FVector OriginalLocation;
	FVector Location;
	FColor Color;	
	bool bEnabled;

	FPointCloudPoint() {}
	FPointCloudPoint(FVector &Location)
		: OriginalLocation(Location)
		, Location(Location)
		, bEnabled(true)
	{}
	FPointCloudPoint(FVector Location, FVector Color)
		: OriginalLocation(Location)
		, Location(Location)
		, Color(FLinearColor(Color).ToFColor(true))
		, bEnabled(true)
	{}
	FPointCloudPoint(float &X, float &Y, float &Z)
		: bEnabled(true)
	{
		Location.X = X;
		Location.Y = Y;
		Location.Z = Z;
		Color = FColor::White;
		OriginalLocation = Location;
	}
	FPointCloudPoint(float &X, float &Y, float &Z, float &I)
		: FPointCloudPoint(X, Y, Z)
	{
		Color.R = FMath::FloorToInt(I * 255.999f);
	}
	FPointCloudPoint(float X, float Y, float Z, float R, float G, float B)
		: FPointCloudPoint(X, Y, Z)
	{
		Color = FLinearColor(R, G, B).ToFColor(false);
	}


	FORCEINLINE float GridDistance(FPointCloudPoint *point)
	{
		FVector p = point->Location - Location;
		return FMath::Max(FMath::Max(FMath::Abs(p.X), FMath::Abs(p.Y)), FMath::Abs(p.Z));
	}
	FORCEINLINE void AddColor(FVector InColor) { Color = FLinearColor(InColor).ToFColor(false); }

	FORCEINLINE FString ToString() { return FString::Printf(TEXT("E: %s, OL: %s, L: %s, C: %s"), BOOL2STR(bEnabled), *OriginalLocation.ToString(), *Location.ToString(), *Color.ToString()); }
};

/** Used for importing text-based files. */
USTRUCT(BlueprintType)
struct POINTCLOUDRUNTIME_API FPointCloudFileHeader
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FileHeader")
	bool bEnabled;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FileHeader")
	FString Filename;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FileHeader")
	int32 LinesToSkip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FileHeader")
	bool bHasDescriptions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FileHeader")
	int32 EstimatedPointCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FileHeader")
	FString Delimiter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FileHeader")
	TArray<FString> Columns;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FileHeader")
	TArray<int32> SelectedColumns;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FileHeader")
	FVector2D RGBRange;
};

USTRUCT(BlueprintType)
struct POINTCLOUDRUNTIME_API FPointCloudImportSettingsLineRange
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import Settings")
	int32 First;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import Settings")
	int32 Last;

public:
	FPointCloudImportSettingsLineRange() {}
	FPointCloudImportSettingsLineRange(int32 First, int32 Last) : First(First), Last(Last) {}
};

USTRUCT(BlueprintType)
struct POINTCLOUDRUNTIME_API FPointCloudImportSettingsColumns
{
	GENERATED_BODY()

public:
	/** Index of a column containing Location X data */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import Settings")
	int32 LocationX;

	/** Index of a column containing Location Y data */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import Settings")
	int32 LocationY;

	/** Index of a column containing Location Z data */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import Settings")
	int32 LocationZ;

	/**
	 * Index of a column containing Red channel or Intensity data.
	 * Set to 0 if neither is available
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import Settings")
	int32 Red;

	/** Index of a column containing Green channel. Set to 0 if not available */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import Settings")
	int32 Green;

	/** Index of a column containing Blue channel. Set to 0 if not available */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import Settings")
	int32 Blue;

public:
	FPointCloudImportSettingsColumns()
	{
		LocationX = 0;
		LocationY = 1;
		LocationZ = 2;
		Red = 3;
		Green = 4;
		Blue = 5;
	}
};