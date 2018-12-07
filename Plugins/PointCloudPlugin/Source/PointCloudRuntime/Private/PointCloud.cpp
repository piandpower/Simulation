// Copyright 2018 Michal Cieciura. All Rights Reserved.

#include "PointCloud.h"
#include "PointCloudSection.h"
#include "PointCloudHelper.h"
#include "PointCloudShared.h"
#include "Misc/ScopedSlowTask.h"
#include "ConstructorHelpers.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Texture2D.h"

#define IS_PROPERTY(Name) PropertyChangedEvent.MemberProperty->GetName().Equals(#Name)

#define LOCTEXT_NAMESPACE "PointCloud"
#define MIN_SCALE 0.001f

/////////////////////////////////////////////////
// UPointCloudSettings

UPointCloudSettings::UPointCloudSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, SectionSize(FVector::ZeroVector)
	, MinimumSectionPointCount(10)
	, NoiseReductionDensity(5)
	, Offset(EPointCloudOffset::Center)
	, Scale(FVector::OneVector * 100)
{
}

#if WITH_EDITOR

bool UPointCloudSettings::CanEditChange(const UProperty* InProperty) const
{
	const bool ParentVal = Super::CanEditChange(InProperty);

	return ParentVal;
}

void UPointCloudSettings::PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent)
{
	if (PropertyChangedEvent.MemberProperty)
	{
		MinimumSectionPointCount = FMath::Max(MinimumSectionPointCount, 1);
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

#endif

/////////////////////////////////////////////////
// UPointCloudImportSettings

UPointCloudImportSettings::UPointCloudImportSettings(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	//Make sure we are transactional to allow undo redo
	this->SetFlags(RF_Transactional);
	FileHeader = FPointCloudFileHeader();
}

void UPointCloudImportSettings::LoadFileHeader(const FString& Filename)
{
	FileHeader = FPointCloudHelper::ReadFileHeader(Filename);
	FileHeader.bEnabled = true;
}

/////////////////////////////////////////////////
// UPointCloud

UPointCloud::UPointCloud(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	//, bPreloadVertexBuffer(true)
	, LODBias(0)
	, LODCount(10)
	, LODReduction(0.35f)
	, LODAggressiveness(0.625f)
	, RenderMethod(EPointCloudRenderMethod::Sprite_Lit_RGB)
	, Saturation(1)
	, Brightness(1)
	, Color(FLinearColor::White)
	, ColorOverride(EPointCloudColorOverride::None)
	, SpriteSize(FVector2D(10, 150))
	, SpriteSizeBias(0.775)
	, SpriteMask(EPointCloudSpriteMask::None)
	, AppliedOffset(FVector::ZeroVector)
	, AppliedScale(FVector::OneVector)
	, bDirty(true)
	, bTransformDirty(true)
	, bReductionDirty(true)
	, bSectionDirty(true)
	, bVBDirty(false)
	, bIBDirty(false)
{
	static ConstructorHelpers::FObjectFinder<UMaterial> M_PointCloud(TEXT("/PointCloudPlugin/Materials/M_PointCloud"));
	MasterMaterial = M_PointCloud.Object;

#if WITH_EDITOR
	if (Material == NULL)
	{
		Material = UMaterial::GetDefaultMaterial(MD_Surface);
		bUsesSprites = false;
	}
#endif
}

UPointCloud::~UPointCloud()
{
	ClearSectionData();
	Points.Empty();
}

void UPointCloud::Serialize(FArchive& Ar)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UPointCloud::Serialize"), STAT_PointCLoud_Serialize, STATGROUP_LoadTime);

	Super::Serialize(Ar);
	
	// Do not run full serialization if only changing properties
	if (Ar.IsTransacting())
	{
		return;
	}	

	bool bCooked = Ar.IsCooking();
	bool bLoading = Ar.IsLoading();
	Ar << bCooked;

	Ar << ColorMode;

	// Flags
	/* Do not use the first bit, for backward compatibility purposes. */
	uint8 Flags = (uint8)(bUsesSprites * 2) + (uint8)(bUseLowPrecision * 4);
	Ar << Flags;

	if (bLoading)
	{
		bUsesSprites = ((Flags & 2) == 2);
		bUseLowPrecision = ((Flags & 4) == 4);
	}

	// Num Points
	int32 NumPoints = bLoading ? 0 : (bCooked ? FPointCloudHelper::CountEnabledPoints(Points) : Points.Num());
	Ar << NumPoints;

	if (bLoading)
	{
		Points.AddUninitialized(NumPoints);
	}

	for (int32 i = 0; i < NumPoints; i++)
	{
		// Skip unnecessary points
		if (!bLoading && bCooked && !Points[i].bEnabled)
		{
			continue;
		}

		// Location
		if (UsesLowPrecision() && bCooked)
		{
			FFloat16 X = Points[i].Location.X;
			FFloat16 Y = Points[i].Location.Y;
			FFloat16 Z = Points[i].Location.Z;
			Ar << X;
			Ar << Y;
			Ar << Z;

			if (bLoading)
			{
				Points[i].Location = FVector(X, Y, Z);
			}
		}
		else	
		{
			if (bCooked)
			{
				Ar << Points[i].Location;
			}
			else
			{
				Ar << Points[i].OriginalLocation;
			}
		}

		if (bCooked)
		{
			Points[i].OriginalLocation = Points[i].Location;
		}

		if (bLoading && bCooked)
		{
			Points[i].bEnabled = true;
		}
		
		// bEnabled
		if(!bCooked)
		{
			Ar << Points[i].bEnabled;
		}

		// Color
		switch (ColorMode)
		{
		case EPointCloudColorMode::Intensity:
			Ar << Points[i].Color.R;
			break;

		case EPointCloudColorMode::RGB:
			Ar << Points[i].Color;
			break;

		default:
			break;
		}
	}

	if (Ar.IsLoading())
	{
		bTransformDirty = !bCooked;
		bSectionDirty = true;
		bReductionDirty = false;
		bDirty = true;
	}
}

void UPointCloud::GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const
{
	OutTags.Add(FAssetRegistryTag("PointCountTotal", PointCloudAssetRegistryCache.PointCountTotal, FAssetRegistryTag::TT_Numerical));
	OutTags.Add(FAssetRegistryTag("PointCountEnabled", PointCloudAssetRegistryCache.PointCountEnabled, FAssetRegistryTag::TT_Numerical));
	OutTags.Add(FAssetRegistryTag("ApproxSize", PointCloudAssetRegistryCache.ApproxSize, FAssetRegistryTag::TT_Dimensional));
	OutTags.Add(FAssetRegistryTag("SectionCount", PointCloudAssetRegistryCache.SectionCount, FAssetRegistryTag::TT_Numerical));
	OutTags.Add(FAssetRegistryTag("Usage", PointCloudAssetRegistryCache.Usage, FAssetRegistryTag::TT_Alphabetical));

	Super::GetAssetRegistryTags(OutTags);
}

void UPointCloud::PostLoad()
{
	Super::PostLoad();
	Rebuild();
}

#if WITH_EDITOR

bool UPointCloud::CanEditChange(const UProperty* InProperty) const
{
	const bool ParentVal = Super::CanEditChange(InProperty);

	//if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(UPointCloud, bPreloadVertexBuffer))
	//{
	//	return false;
	//}

	return ParentVal;
}

void UPointCloud::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	bool bCommonDirty = true;

	if (PropertyChangedEvent.MemberProperty)
	{
		LODCount = FMath::Max(LODCount, 1);
		LODReduction = FMath::Clamp(LODReduction, 0.0f, 1.0f);
		LODAggressiveness = FMath::Clamp(LODAggressiveness, 0.0f, 1.0f);

		if (IS_PROPERTY(DensityReductionDistance) || IS_PROPERTY(NoiseReductionDistance) || IS_PROPERTY(NoiseReductionDensity))
		{
			bReductionDirty = true;
		}

		if (IS_PROPERTY(LODBias))
		{
			bCommonDirty = false;
			bDirty = true;
			OnPointCloudChangedEvent.Broadcast();
		}

		if (IS_PROPERTY(LODAggressiveness) || IS_PROPERTY(LODCount) || IS_PROPERTY(LODReduction))
		{
			bCommonDirty = false;
			bIBDirty = true;
		}

		if (IS_PROPERTY(Offset) || IS_PROPERTY(Translation) || IS_PROPERTY(Scale))
		{
			bTransformDirty = true;
		}

#if WITH_LOW_PRECISION
		if (IS_PROPERTY(bUseLowPrecision))
		{
			bReductionDirty = true;
			bTransformDirty = true;
		}
#endif

		if (IS_PROPERTY(Brightness) || IS_PROPERTY(Saturation) || IS_PROPERTY(Color) || IS_PROPERTY(SpriteSizeBias) || IS_PROPERTY(SpriteSize) || IS_PROPERTY(ColorOverride) || IS_PROPERTY(SpriteMask) || IS_PROPERTY(SpriteTexture))
		{
			ApplyRenderingParameters();
			bCommonDirty = false;
		}
		
		if (IS_PROPERTY(RenderMethod))
		{
			// Only require full rebuild if Point/Sprite mode switched, otherwise simply update material params
			DetermineSpriteUsage();
			if (bUsesSprites == bUsesSpritesCached)
			{
				ApplyRenderingParameters();
				bCommonDirty = false;
			}
		}

		if (bCommonDirty)
		{
			bSectionDirty = true;

			SectionSize = FVector(FMath::Max(SectionSize.X, 0.0f), FMath::Max(SectionSize.Y, 0.0f), FMath::Max(SectionSize.Z, 0.0f));

			bDirty = true;
			OnPointCloudChangedEvent.Broadcast();
		}
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

void UPointCloud::ApplyRenderingParameters()
{
	bool bUnlit = false;
	bool bRGB = false;

	switch (RenderMethod)
	{
	case EPointCloudRenderMethod::Point_Unlit:
	case EPointCloudRenderMethod::Sprite_Unlit:
		bUnlit = true;
		break;
	case EPointCloudRenderMethod::Point_Unlit_RGB:
	case EPointCloudRenderMethod::Sprite_Unlit_RGB:
		bUnlit = true;
		bRGB = true;
		break;
	case EPointCloudRenderMethod::Point_Lit_RGB:
	case EPointCloudRenderMethod::Sprite_Lit_RGB:
		bRGB = true;
		break;
	}

	FMaterialInstanceBasePropertyOverrides Overrides;
	Overrides.bOverride_ShadingModel = bUnlit;
	Overrides.ShadingModel = EMaterialShadingModel::MSM_Unlit;

	FVector BoundsSize = GetBounds().GetBox().GetSize();
	if (BoundsSize == FVector::ZeroVector)
	{
		BoundsSize = FVector::OneVector;
	}

	float LODFactor = LODReduction < 1 ? SpriteSizeBias / (1 - LODReduction) : 0;

	for (FPointCloudSection *Section : Sections)
	{
		UMaterialInstanceDynamic *DynMaterial = Cast<UMaterialInstanceDynamic>(Section->GetMaterial());
		if (DynMaterial)
		{
			DynMaterial->SetScalarParameterValue("PC__Brightness", Brightness);
			DynMaterial->SetScalarParameterValue("PC__Saturation", Saturation);
			DynMaterial->SetVectorParameterValue("PC__Color", Color);
			DynMaterial->SetScalarParameterValue("PC__SpriteMin", SpriteSize.X);
			DynMaterial->SetScalarParameterValue("PC__SpriteMax", SpriteSize.Y);

			// CHANGED Added
			DynMaterial->SetScalarParameterValue("PC__Size", SpriteSize.Y);


			DynMaterial->SetScalarParameterValue("PC__LODFactor", LODFactor);
			DynMaterial->SetScalarParameterValue("PC__UseRGB", bRGB);
			DynMaterial->SetScalarParameterValue("PC__UseEmissive", bUnlit);
			DynMaterial->SetScalarParameterValue("PC__UseSprites", bUsesSpritesCached);
			DynMaterial->SetScalarParameterValue("PC__UseWorldColor", ColorOverride == EPointCloudColorOverride::Position);
			DynMaterial->SetScalarParameterValue("PC__UseHeightColor", ColorOverride == EPointCloudColorOverride::Height);
			DynMaterial->SetScalarParameterValue("PC__UseCircle", SpriteMask == EPointCloudSpriteMask::Circle);
			DynMaterial->SetScalarParameterValue("PC__UseMask", SpriteMask == EPointCloudSpriteMask::Texture);
			DynMaterial->SetVectorParameterValue("PC__Bounds", BoundsSize);
			DynMaterial->SetScalarParameterValue("PC__UseTexture", IsValid(SpriteTexture));
			if (SpriteTexture)
			{
				DynMaterial->SetTextureParameterValue("PC__Texture", SpriteTexture);
			}

			DynMaterial->BasePropertyOverrides = Overrides;
		}
	}
}

int32 UPointCloud::GetPointCount(bool bCountOnlyEnabled) const
{
	return bCountOnlyEnabled ? FPointCloudHelper::CountEnabledPoints(Points) : Points.Num();
}

FVector UPointCloud::GetOriginalDataOffset()
{
	return Points.Num() > 0 ? AppliedOffset : FVector::ZeroVector;
}

void UPointCloud::Rebuild(bool bForced)
{
	if (bForced)
	{
		bTransformDirty = bReductionDirty = bSectionDirty = true;

		// Will make sure we have a full section rebuild
		bIBDirty = bVBDirty = false;
	}

	bool bBoundsDirty = bTransformDirty || bReductionDirty || bSectionDirty;

	float MaxProgress = (bTransformDirty || bReductionDirty) + bTransformDirty + (2 * bReductionDirty) + bSectionDirty + (bVBDirty || bIBDirty) + bBoundsDirty + 1;

	FScopedSlowTask Progress(MaxProgress, LOCTEXT("Rebuild", "Rebuilding Cloud..."));
	Progress.MakeDialog();

	if (bTransformDirty || bReductionDirty)
	{
		Progress.EnterProgressFrame(1.f, LOCTEXT("RebuildPrep", "Preparing Data"));

		for (int i = 0; i < Points.Num(); i++)
		{
			if (bTransformDirty)
			{
				Points[i].Location = Points[i].OriginalLocation;
			}
			if (bReductionDirty)
			{
				Points[i].bEnabled = true;
			}
		}
	}

	if (bTransformDirty)
	{
		AppliedOffset = Translation;
		
		// Apply Min constraints to scale
		if (FMath::Abs(Scale.X) < MIN_SCALE) Scale.X = Scale.X >= 0 ? MIN_SCALE : -MIN_SCALE;
		if (FMath::Abs(Scale.Y) < MIN_SCALE) Scale.Y = Scale.Y >= 0 ? MIN_SCALE : -MIN_SCALE;
		if (FMath::Abs(Scale.Z) < MIN_SCALE) Scale.Z = Scale.Z >= 0 ? MIN_SCALE : -MIN_SCALE;

		AppliedScale = Scale;

		Progress.EnterProgressFrame(1.f, LOCTEXT("RebuildTransform", "Applying Transformations"));

		AppliedOffset -= FPointCloudHelper::Transform(Points, Offset, Translation, Scale, UsesLowPrecision());

		bTransformDirty = false;
	}

	if (bReductionDirty)
	{
		Progress.EnterProgressFrame(1.f, LOCTEXT("RebuildDensity", "Reducing Density"));
		FPointCloudHelper::DensityReduction(Points, DensityReductionDistance);
		Progress.EnterProgressFrame(1.f, LOCTEXT("RebuildNoise", "Reducing Noise"));
		FPointCloudHelper::NoiseReduction(Points, NoiseReductionDistance, NoiseReductionDensity);
		bReductionDirty = false;
	}
	
	if (bSectionDirty)
	{
		Material = CustomMaterial ? CustomMaterial : MasterMaterial;
		DetermineSpriteUsage();

		Progress.EnterProgressFrame(1.f, LOCTEXT("RebuildSections", "Building Sections"));

		SectionSize = FPointCloudHelper::AdjustSectionSize(Points, SectionSize);
		TArray<TArray<FPointCloudPoint*>> sections = FPointCloudHelper::SplitIntoSections(Points, SectionSize, MinimumSectionPointCount);

		ClearSectionData();
		Sections.AddUninitialized(sections.Num());
		for (int32 i = 0; i < sections.Num(); i++)
		{
			Sections[i] = new FPointCloudSection(this, sections[i]);
		}

		// CHANGED
		//ClearSectionData();
		//Sections.AddUninitialized(1);
		//TArray<FPointCloudPoint*> points;
		//for (FPointCloudPoint point : Points) {
		//	points.Add(&point);
		//}
		//Sections[0] = new FPointCloudSection(this, points);
		

		bSectionDirty = false;
		bUsesSpritesCached = bUsesSprites;
	}
	else if (bVBDirty || bIBDirty)
	{
		Progress.EnterProgressFrame(1.f, LOCTEXT("RebuildSections", "Building Sections"));

		for (FPointCloudSection *Section : Sections)
		{
			Section->Rebuild(bVBDirty, bIBDirty);
		}

		bVBDirty = bIBDirty = false;;
	}

	if (bBoundsDirty)
	{
		Progress.EnterProgressFrame(1.f, LOCTEXT("RebuildBounds", "Calculating Bounds"));

		FBox BoundingBox = FPointCloudHelper::CalculateBounds(Points);
		LocalBounds.BoxExtent = BoundingBox.GetExtent();
		LocalBounds.Origin = BoundingBox.GetCenter();
		LocalBounds.SphereRadius = LocalBounds.BoxExtent.Size();
	}

	Progress.EnterProgressFrame(1.f, LOCTEXT("RebuildFinalize", "Finalizing Data"));

	ApplyRenderingParameters();

	bDirty = false;
	OnPointCloudChangedEvent.Broadcast();
	OnPointCloudRebuiltEvent.Broadcast();

	CacheAssetRegistryTags();
}

void UPointCloud::Bake()
{
	// Remove existing components?

	FScopedSlowTask Progress(1.f, LOCTEXT("Bake", "Baking Cloud..."));
	Progress.MakeDialog();

	Progress.EnterProgressFrame(1.f, LOCTEXT("Bake1", "Baking Cloud..."));
	
	TArray<FPointCloudPoint> NewPoints;
	NewPoints.AddUninitialized(FPointCloudHelper::CountEnabledPoints(Points));
	for (int32 i = 0, idx = 0; i < Points.Num(); i++)
	{
		if (Points[i].bEnabled)
		{
			NewPoints[idx++] = Points[i];
		}
	}
	Points = NewPoints;
	
	// More memory efficient, but very slow
	//int32 RemoveStart = -1;
	//for (int32 i = 0, idx = 0; i < Points.Num(); i++)
	//{
	//	if (Points[i].bEnabled)
	//	{
	//		if (RemoveStart > -1)
	//		{
	//			Points.RemoveAt(RemoveStart, i - RemoveStart, false);
	//			i = RemoveStart;
	//			RemoveStart = -1;
	//		}
	//	}
	//	else
	//	{
	//		if (RemoveStart == -1)
	//		{
	//			RemoveStart = i;
	//		}
	//	}
	//}
	//Points.Shrink();

	bDirty = true;
	bSectionDirty = true;
	OnPointCloudChangedEvent.Broadcast();
}

void UPointCloud::ClearSectionData()
{
	for (FPointCloudSection *Section : Sections)
	{
		if (Section)
		{
			delete Section;
			Section = nullptr;
		}
	}

	Sections.Empty();
}

void UPointCloud::SetRenderingMethod(EPointCloudRenderMethod NewRenderMethod, bool bRebuild)
{
	RenderMethod = NewRenderMethod;

	// Only require full rebuild if Point/Sprite mode switched, otherwise simply update material params
	DetermineSpriteUsage();
	if (bUsesSprites == bUsesSpritesCached)
	{
		ApplyRenderingParameters();
	}
	else
	{
		bDirty = true;
		bSectionDirty = true;

		if (bRebuild)
		{
			Rebuild();
		}
	}
}

void UPointCloud::SetCustomMaterial(UMaterialInterface *InCustomMaterial, bool bRebuild /*= false*/)
{
	CustomMaterial = InCustomMaterial;
	ApplyRenderingParameters();
	bSectionDirty = true;
	bDirty = true;

	if (bRebuild)
	{
		Rebuild();
	}
}

void UPointCloud::SetPointCloudData(TArray<FPointCloudPoint> &InPoints, bool bRebuildCloud)
{
	Points = InPoints;

	if (bRebuildCloud)
	{
		Rebuild();
	}
}

void UPointCloud::SetSettings(UPointCloudSettings *Settings)
{
	if (Settings)
	{
		SectionSize = Settings->SectionSize;
		bUseLowPrecision = Settings->bUseLowPrecision;
		MinimumSectionPointCount = Settings->MinimumSectionPointCount;
		DensityReductionDistance = Settings->DensityReductionDistance;
		NoiseReductionDistance = Settings->NoiseReductionDistance;
		NoiseReductionDensity = Settings->NoiseReductionDensity;
		Offset = Settings->Offset;
		Translation = Settings->Translation;
		Scale = Settings->Scale;

		ApplyRenderingParameters();
	}
}

void UPointCloud::CacheAssetRegistryTags()
{
	FBoxSphereBounds Bounds = GetBounds();

	PointCloudAssetRegistryCache.PointCountTotal = FString::FromInt(GetPointCount(false));
	PointCloudAssetRegistryCache.PointCountEnabled = FString::FromInt(GetPointCount(true));
	PointCloudAssetRegistryCache.ApproxSize = FString::Printf(TEXT("%dx%dx%d"), FMath::RoundToInt(Bounds.BoxExtent.X * 2.0f), FMath::RoundToInt(Bounds.BoxExtent.Y * 2.0f), FMath::RoundToInt(Bounds.BoxExtent.Z * 2.0f));
	PointCloudAssetRegistryCache.SectionCount = FString::FromInt(GetSectionCount());
	PointCloudAssetRegistryCache.Usage = UsesSprites() ? "Sprites" : "Points";
}

void UPointCloud::DetermineSpriteUsage()
{
	bUsesSprites = false;

	switch (RenderMethod)
	{
	case EPointCloudRenderMethod::Sprite_Unlit:
	case EPointCloudRenderMethod::Sprite_Unlit_RGB:
	case EPointCloudRenderMethod::Sprite_Lit:
	case EPointCloudRenderMethod::Sprite_Lit_RGB:
		bUsesSprites = true;
		break;
	}
}

#undef LOCTEXT_NAMESPACE
