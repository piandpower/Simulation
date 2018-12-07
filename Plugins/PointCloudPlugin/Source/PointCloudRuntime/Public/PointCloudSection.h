// Copyright 2018 Michal Cieciura. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UPointCloud;
class UMaterialInterface;
struct FPointCloudPoint;
class IPointCloudSectionProxy;

class FPointCloudSection 
{
private:
	/** Pointer to the point cloud this section belong to */
	UPointCloud *Cloud;
	
	/** Holds the reference to the points used by this section */
	const TArray<FPointCloudPoint*> Points;

	FBoxSphereBounds LocalBounds;

	UMaterialInterface *Material;

	/** Holds raw data for the buffers */
	uint8* VertexBuffer;
	uint32 VertexBufferSize;
	uint8* IndexBuffer;
	uint8* IndexBufferSpecial;	// For view mode overrides if using sprites
	uint32 IndexBufferSize;

	uint32 VertexCount;

	/** Used for LOD toggle */
	TArray<uint32> NumPrimitives;
	TArray<float> ScreenSizes;

	FMatrix ProjectionMatrix;

public:
	FPointCloudSection(UPointCloud *InCloud, const TArray<FPointCloudPoint*> InPoints);
	~FPointCloudSection();

	IPointCloudSectionProxy* BuildProxy();

	UMaterialInterface* GetMaterial() const { return Material && Material->IsValidLowLevel() ? Material : nullptr; }

	FORCEINLINE uint32 GetVertexCount() const { return VertexCount; }

	FORCEINLINE FBoxSphereBounds GetLocalBounds() const { return LocalBounds; }
	FBoxSphereBounds GetWorldBounds(const FTransform& LocalToWorld) const { return LocalBounds.TransformBy(LocalToWorld); }

	void Rebuild(bool bBuildVB, bool bBuildIB);

private:
	void Dispose(bool bVB, bool bIB);

	FBoxSphereBounds CalcBounds(const FTransform& Transform = FTransform::Identity) const;

	FORCEINLINE double CalculateSkip(int32 LODLevel) const { return ((double)1) / FMath::Pow(1 - FMath::Clamp(Cloud->LODReduction, 0.0f, 1.0f), LODLevel); }

	void BuildVB();
	void BuildIBAndLOD();
};
