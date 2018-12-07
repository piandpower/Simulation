// Copyright 2018 Michal Cieciura. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Materials/MaterialInterface.h"
#include "Curves/CurveFloat.h"
#include "PointCloudShared.h"
#include "PointCloud.generated.h"

class FPointCloudSection;
class UMaterialInstanceDynamic;

UENUM(BlueprintType)
enum class EPointCloudRenderMethod : uint8
{
	Point_Unlit,
	Point_Unlit_RGB,
	Point_Lit,
	Point_Lit_RGB,
	Sprite_Unlit,
	Sprite_Unlit_RGB,
	Sprite_Lit,
	Sprite_Lit_RGB
};

UENUM(BlueprintType)
enum class EPointCloudColorOverride : uint8
{
	None,
	Height,
	Position
};

UENUM(BlueprintType)
enum class EPointCloudSpriteMask : uint8
{
	None,
	Circle,
	Texture
};

/** Used for caching the asset registry tag data. */
struct FPointCloudAssetRegistryCache
{
	FString PointCountTotal;
	FString PointCountEnabled;
	FString ApproxSize;
	FString SectionCount;
	FString Usage;
};

UCLASS()
class POINTCLOUDRUNTIME_API UPointCloudSettings : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	/**
	* Size of the sections to divide the cloud into.
	* Smaller sections will result in better and smoother LOD control, but can increase draw calls.
	* Setting it to 0 will attempt to automatically select settings for the current bounds.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	FVector SectionSize;

	/**
	 * Minimum amount of points in a section to be rendered.
	 * Can significantly improve draw call count without noticeable quality drop, especially at higher LODs.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Performance")
	int32 MinimumSectionPointCount;

	/**
	 * Uses 16-bit precision for Location and 16-bit RGB color, but may significantly reduce VRAM requirements.
	 * NOTE: This will have no effect if the plugin has not been compiled with low precision support. You can enable/disable the support inside PointCloudShared.h file.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Performance")
	bool bUseLowPrecision;

	/**
	 * Points which are closer than this value will be removed.
	 * Higher number will result in lower cloud resolution.
	 * Values higher than 0 will enable the reduction.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Processing")
	float DensityReductionDistance;
	
	/**
	 * Points which are further away than this value will be removed.
	 * Useful to remove singular points and small clusters scattered around the area.
	 * Optimal value will depend on the density of the cloud and the value of DensityReductionDistance.
	 * Setting this value too low will cause quality degradation and cloud coverage loss.
	 * Values higher than 0 will enable the reduction.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Processing")
	float NoiseReductionDistance;

	/**
	 * Minimum amount of neighbors within the NoiseReductionDistance range of the point, for it to not be considered stray.
	 * Optimal value will depend on the density of the cloud and sizes of the artifacts. 5 seems to generally give good results without visible data loss.
	 * Setting this value too high will cause quality degradation and cloud coverage loss.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Processing")
	int32 NoiseReductionDensity;

	/**
	 * How should the point cloud be repositioned:
	 * None: no change to coordinates
	 * Center: moves center of the cloud to 0,0,0
	 * First Point: moves first point to 0,0,0
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transformation")
	EPointCloudOffset Offset;

	/** Offset to add to each point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transformation")
	FVector Translation;

	/** Scale to apply to each point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transformation")
	FVector Scale;

public:
#if WITH_EDITOR
	virtual bool CanEditChange(const UProperty* InProperty) const override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent) override;
#endif
};

UCLASS()
class POINTCLOUDRUNTIME_API UPointCloudImportSettings : public UPointCloudSettings
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import Settings")
	FPointCloudFileHeader FileHeader;

	/** First and Last indices of points to import */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import Settings")
	FPointCloudImportSettingsLineRange LineRange;
	
public:
	void LoadFileHeader(const FString& Filename);
};

UCLASS(BlueprintType, autoexpandcategories=("Performance", "Performance|LOD"))
class POINTCLOUDRUNTIME_API UPointCloud : public UPointCloudSettings
{
	GENERATED_UCLASS_BODY()

public:
	/**
	 * Loads the whole Vertex Buffer into GPU for much faster LOD toggle, at the cost of much higher VRAM requirements.
	 * Warning: Make sure you have sufficient VRAM capacity when this is enabled, otherwise the application may crash when rendering is attempted.
	 */
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Performance")
	//bool bPreloadVertexBuffer;

	/**
	 * Will shift LOD for each tile by the provided value.
	 * Positive values will degrade density, negative values will increase it.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD", meta = (DisplayName = "LOD Bias"))
	int32 LODBias;

	/** Specifies the number of LODs to generate */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD", meta = (DisplayName = "LOD Count"))
	int32 LODCount;

	/**
	 * Specifies the percentage of points to remove in each LOD step.
	 * Requires range of 0 - 1. Higher numbers will result in faster quality degradation.
	 * For example, setting it to 0.5 will generate LOD0: 100%, LOD1: 50%, LOD2: 25%, LOD3: 12.5% etc.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD", meta = (DisplayName = "LOD Reduction Factor", ClampMin = "0.0", ClampMax = "1.0"))
	float LODReduction;

	/**
	 * Specifies how aggressive should the LOD switching be.
	 * Higher values will result in lower quality tiles appearing sooner.
	 * Range is 0 - 1
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD", meta = (DisplayName = "LOD Aggressiveness", ClampMin = "0.0", ClampMax = "1.0"))
	float LODAggressiveness;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
	EPointCloudRenderMethod RenderMethod;

	/** Ignored if non-RGB rendering mode is selected */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
	float Saturation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
	float Brightness;

	/** Ignored if RGB rendering mode is selected */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
	FLinearColor Color;

	/**
	 * None - no effect
	 * Height - The cloud color will be overridden with height-based color
	 * Position - The cloud color will be overridden with position-based color
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
	EPointCloudColorOverride ColorOverride;

	/** Minimum and Maximum sizes of the sprite */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering|Sprite")
	FVector2D SpriteSize;

	/**
	 * Allows fine-tuning of the sprite size auto-adjustment.
	 * Setting it too high may result in unnecessarily large sprites, degrading quality,
	 * while setting it too low will result in visible gaps between individual sprites.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering|Sprite")
	float SpriteSizeBias;

	/** Texture to use with sprites. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering|Sprite")
	class UTexture2D *SpriteTexture;

	/**
	 * Affects the shape of sprites.
	 * None - Standard, square shape. Fastest.
	 * Circle - Simple circle, medium speed.
	 * Texture - Alpha channel of the sprite texture. Slowest.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering|Sprite")
	EPointCloudSpriteMask SpriteMask;

	/**
	 * Setting this will automatically replace the default material.
	 * To access the properties selected via the panel, use MF_PointCloudParameters node.
	 * NOTE: if used in conjunction with Sprite rendering mode, make sure to assign WorldPositionOffset
	 * via MF_PointCloud_SpriteTransform node (or some other way), otherwise nothing will be visible.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Rendering")
	UMaterialInterface *CustomMaterial;

	/** Specifies what color information is contained inside the data */
	EPointCloudColorMode ColorMode;

	/** Array of processed sections */
	TArray<FPointCloudSection*> Sections;

private:
	bool bUsesSprites;
	bool bUsesSpritesCached;

	UPROPERTY()
	FVector AppliedOffset;
	UPROPERTY()
	FVector AppliedScale;

	FBoxSphereBounds LocalBounds;

	FPointCloudAssetRegistryCache PointCloudAssetRegistryCache;

	/** Flags that the point cloud settings have been modified but not rebuilt yet.  */
	bool bDirty;

	/** Flags that the Transform parameters have changed and will re-run it during next Rebuild. */
	bool bTransformDirty;

	/** Flags that the Density or Noise Reduction parameters have changed and will re-run them during next Rebuild. */
	bool bReductionDirty;

	/** Flags that the Section split parameters have changed and will re-run it during next Rebuild. */
	bool bSectionDirty;

	/** Flags that the LOD parameters have changed and will require Vertex Buffer rebuild. */
	bool bVBDirty;

	/** Flags that the LOD parameters have changed and will require Index Buffer rebuild. */
	bool bIBDirty;

	/** Stores the data */
	//UPROPERTY()
	TArray<FPointCloudPoint> Points;

	UPROPERTY(Transient)
	UMaterialInterface *Material;
	UPROPERTY(Transient)
	UMaterialInterface *MasterMaterial;

public:
	DECLARE_EVENT(UPointCloud, FOnPointCloudChanged);

	~UPointCloud();

	// Begin UObject Interface.
	virtual void Serialize(FArchive& Ar) override;
	virtual void GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const override;
	virtual void PostLoad() override;

#if WITH_EDITOR
	virtual bool CanEditChange(const UProperty* InProperty) const override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent) override;
	// End UObject Interface.
#endif

	virtual FOnPointCloudChanged& OnPointCloudRebuilt() { return OnPointCloudRebuiltEvent; }
	virtual FOnPointCloudChanged& OnPointCloudChanged() { return OnPointCloudChangedEvent; }

	UFUNCTION(BlueprintPure, Category = "Rendering")
	FORCEINLINE bool UsesSprites() const { return bUsesSprites; }

	UFUNCTION(BlueprintPure, Category = "Rendering")
	FORCEINLINE UMaterialInterface* GetMaterial() const { return Material; }

	UFUNCTION(BlueprintPure, Category = "Point Cloud")
	FORCEINLINE bool UsesLowPrecision()
	{
#if WITH_LOW_PRECISION
		return bUseLowPrecision;
#else
		return false;
#endif		
	}

	/** Applies specified rendering parameters (Brightness, Saturation, etc) to the selected material */
	UFUNCTION(BlueprintCallable, Category = "Rendering")
	void ApplyRenderingParameters();

	UFUNCTION(BlueprintPure, Category = "Point Cloud")
	FORCEINLINE bool IsDirty() { return bDirty; }

	UFUNCTION(BlueprintPure, Category = "Point Cloud")
	FBoxSphereBounds GetBounds() const { return LocalBounds; }

	UFUNCTION(BlueprintPure, Category = "Point Cloud")
	inline int32 GetSectionCount() const { return Sections.Num(); }

	UFUNCTION(BlueprintPure, Category = "Point Cloud")
	int32 GetPointCount(bool bCountOnlyEnabled) const;

	UFUNCTION(BlueprintPure, Category = "Point Cloud")
	FVector GetOriginalDataOffset();
	
	/** Rebuilds the cloud using current settings */
	UFUNCTION(BlueprintCallable, Category = "Point Cloud")
	void Rebuild(bool bForced = false);

	/** Permanently strips all disabled points */
	UFUNCTION(BlueprintCallable, Category = "Point Cloud")
	void Bake();

	/** Removes all processed section data. */
	UFUNCTION(BlueprintCallable, Category = "Point Cloud")
	void ClearSectionData();

	/** Sets new Rendering Method. */
	UFUNCTION(BlueprintCallable, Category = "Point Cloud")
	void SetRenderingMethod(EPointCloudRenderMethod NewRenderMethod, bool bRebuild = false);

	/** Sets Custom Material and optionally rebuilds the cloud. */
	UFUNCTION(BlueprintCallable, Category = "Point Cloud")
	void SetCustomMaterial(UMaterialInterface *InCustomMaterial, bool bRebuild = false);

	/** Return Points Data as reference to array */
	UFUNCTION(BlueprintPure, Category = "Point Cloud")
	FORCEINLINE TArray<FPointCloudPoint>& GetPointCloudData() { return Points; }

	/**
	 * Replaces the original data with the set provided.
	 * Optionally rebuilds using current settings.
	 */
	UFUNCTION(BlueprintCallable, Category = "Point Cloud")
	void SetPointCloudData(UPARAM(ref) TArray<FPointCloudPoint> &InPoints, bool bRebuildCloud = true);

	/** Bulk sets the new settings from the ones provided */
	void SetSettings(UPointCloudSettings *Settings);

private:
	void CacheAssetRegistryTags();
	void DetermineSpriteUsage();

	FOnPointCloudChanged OnPointCloudChangedEvent;
	FOnPointCloudChanged OnPointCloudRebuiltEvent;
};