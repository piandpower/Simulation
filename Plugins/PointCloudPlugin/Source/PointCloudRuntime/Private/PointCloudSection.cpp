// Copyright 2018 Michal Cieciura. All Rights Reserved.

#include "PointCloudSection.h"
#include "PointCloud.h"
#include "PointCloudShared.h"
#include "PointCloudHelper.h"
#include "VertexFactory.h"
#include "IPointCloudSectionProxy.h"
#include "MaterialShared.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"

////////////////////////////////////////////////////////////
// FPointCloudVertexBuffer

class FPointCloudVertexBuffer : public FVertexBuffer
{
public:
	uint8 *Data;
	uint32 DataSize;

	virtual void InitRHI() override
	{
		if (Data && DataSize)
		{
			FRHIResourceCreateInfo CreateInfo;
			void* Buffer = nullptr;
			VertexBufferRHI = RHICreateAndLockVertexBuffer(DataSize, BUF_Static, CreateInfo, Buffer);
			FMemory::Memcpy(Buffer, Data, DataSize);
			RHIUnlockVertexBuffer(VertexBufferRHI);
		}
	}
};

////////////////////////////////////////////////////////////
// FPointCloudVertexFactory

class FPointCloudVertexFactory : public FVertexFactory
{
	DECLARE_VERTEX_FACTORY_TYPE(FPointCloudVertexFactory);

public:
	struct FDataType
	{
		FVertexStreamComponent PositionComponent;
		FVertexStreamComponent ColorComponent;
	};

	FPointCloudVertexFactory(ERHIFeatureLevel::Type InFeatureLevel)
		: FVertexFactory(InFeatureLevel)
	{
	}
	virtual ~FPointCloudVertexFactory() { ReleaseResource(); }

	virtual void Init(const FPointCloudVertexBuffer* VertexBuffer)
	{
		if (IsInRenderingThread())
		{
			Data = GetDataType(VertexBuffer);
			UpdateRHI();
		}
		else
		{
			ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
				InitPointCloudVertexFactory,
				FPointCloudVertexFactory*, VertexFactory, this,
				const FPointCloudVertexBuffer*, VertexBuffer, VertexBuffer,
				{
					VertexFactory->Init(VertexBuffer);
				});
		}
	}

	virtual void InitRHI() override
	{
		FVertexDeclarationElementList Elements;

		Elements.Add(AccessStreamComponent(Data.PositionComponent, 0));
		Elements.Add(AccessStreamComponent(Data.ColorComponent, 1));

		InitDeclaration(Elements);
	}

	static FVertexFactoryShaderParameters* ConstructShaderParameters(EShaderFrequency ShaderFrequency) { return NULL; }
	static bool ShouldCache(EShaderPlatform Platform, const class FMaterial* Material, const class FShaderType* ShaderType)
	{
		return IsFeatureLevelSupported(Platform, ERHIFeatureLevel::SM5) && Platform == EShaderPlatform::SP_PCD3D_SM5;
	}
	static bool ShouldCompilePermutation(EShaderPlatform Platform, const class FMaterial* Material, const class FShaderType* ShaderType) { return true; }
	static void ModifyCompilationEnvironment(EShaderPlatform Platform, const FMaterial* Material, FShaderCompilerEnvironment& OutEnvironment) {}

	FDataType Data;

protected:
	virtual FDataType GetDataType(const FPointCloudVertexBuffer* VertexBuffer)
	{
		FDataType DataType;
		DataType.PositionComponent = FVertexStreamComponent(VertexBuffer, 0, 16, VET_Float3);
		DataType.ColorComponent = FVertexStreamComponent(VertexBuffer, 12, 16, VET_Color);
		return DataType;
	}
};

IMPLEMENT_VERTEX_FACTORY_TYPE(FPointCloudVertexFactory, "/Plugin/PointCloudPlugin/Private/PointCloudVertexFactory.ush", /* bUsedWithMaterials */ true, /* bSupportsStaticLighting */ false, /* bSupportsDynamicLighting */ true, /* bPrecisePrevWorldPos */ false, /* bSupportsPositionOnly */ true);

#if WITH_LOW_PRECISION
class FPointCloudVertexFactoryLow : public FPointCloudVertexFactory
{
	DECLARE_VERTEX_FACTORY_TYPE(FPointCloudVertexFactoryLow);
	
public:
	FPointCloudVertexFactoryLow(ERHIFeatureLevel::Type InFeatureLevel)
		: FPointCloudVertexFactory(InFeatureLevel)
	{
	}

	virtual void InitRHI() override
	{
		FVertexDeclarationElementList Elements;
		Elements.Add(AccessStreamComponent(Data.PositionComponent, 0));
		Elements.Add(AccessStreamComponent(Data.ColorComponent, 1));
		InitDeclaration(Elements);
	}

	static FVertexFactoryShaderParameters* ConstructShaderParameters(EShaderFrequency ShaderFrequency) { return NULL; }
	static bool ShouldCache(EShaderPlatform Platform, const class FMaterial* Material, const class FShaderType* ShaderType) { return FPointCloudVertexFactory::ShouldCache(Platform, Material, ShaderType); }
	static bool ShouldCompilePermutation(EShaderPlatform Platform, const class FMaterial* Material, const class FShaderType* ShaderType) { return true; }
	static void ModifyCompilationEnvironment(EShaderPlatform Platform, const FMaterial* Material, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.SetDefine(TEXT("USE_LOW_PRECISION"), TEXT("1"));
	}
	
protected:
	virtual FDataType GetDataType(const FPointCloudVertexBuffer* VertexBuffer) override
	{
		FDataType DataType;
		DataType.PositionComponent = FVertexStreamComponent(VertexBuffer, 0, 8, VET_Half4);
		DataType.ColorComponent = FVertexStreamComponent(VertexBuffer, 4, 8, VET_UShort2);
		return DataType;
	}
};

IMPLEMENT_VERTEX_FACTORY_TYPE(FPointCloudVertexFactoryLow, "/Plugin/PointCloudPlugin/Private/PointCloudVertexFactory.ush", /* bUsedWithMaterials */ true, /* bSupportsStaticLighting */ false, /* bSupportsDynamicLighting */ true, /* bPrecisePrevWorldPos */ false, /* bSupportsPositionOnly */ true);
#endif

////////////////////////////////////////////////////////////
// FPointCloudIndexBuffer

class FPointCloudIndexBuffer : public FIndexBuffer
{
public:
	uint8 *Data;
	uint32 DataSize;

	virtual void InitRHI() override
	{
		if (Data && DataSize)
		{
			FRHIResourceCreateInfo CreateInfo;
			void* Buffer = nullptr;
			IndexBufferRHI = RHICreateAndLockIndexBuffer(sizeof(uint32), DataSize, BUF_Static, CreateInfo, Buffer);
			FMemory::Memcpy(Buffer, Data, DataSize);
			RHIUnlockIndexBuffer(IndexBufferRHI);
		}
	}
};

////////////////////////////////////////////////////////////
// FPointCloudSectionProxy

class FPointCloudSectionProxy : public IPointCloudSectionProxy
{
private:
	FPointCloudVertexBuffer VertexBuffer;
	FPointCloudIndexBuffer IndexBuffer;
	FPointCloudIndexBuffer IndexBufferSpecial;
	FPointCloudVertexFactory* VertexFactory;

	UMaterialInterface *Material;
	UMaterialInstanceDynamic *MID;

	uint32 RenderMode;
	uint32 VertexCount;
	uint32 MinPointCount;

	int32 CurrentLOD;
	int32 LODBias;
	TArray<float> ScreenSizes;
	TArray<uint32> NumPrimitives;

public:
	FPointCloudSectionProxy(uint8* InIndexBuffer, uint8* InIndexBufferSpecial, uint32 InIndexBufferSize, uint8* InVertexBuffer, uint32 InVertexBufferSize, const bool bUseSprites, uint32 VertexCount, UMaterialInterface* Material, TArray<uint32> NumPrimitives, uint32 MinPointCount, TArray<float> ScreenSizes, const bool bUseLowPrecision, int32 LODBias)
		: Material(Material)
		, MID(Cast<UMaterialInstanceDynamic>(Material))
		, RenderMode(bUseSprites ? PT_TriangleList : PT_PointList)
		, VertexCount(VertexCount)
		, MinPointCount(MinPointCount)
		, CurrentLOD(0)
		, LODBias(LODBias)
		, ScreenSizes(ScreenSizes)
		, NumPrimitives(NumPrimitives)
	{
		IndexBuffer.Data = InIndexBuffer;
		IndexBuffer.DataSize = InIndexBufferSize;
		IndexBufferSpecial.Data = InIndexBufferSpecial;
		IndexBufferSpecial.DataSize = bUseSprites ? InIndexBufferSize / 6 : InIndexBufferSize;
		VertexBuffer.Data = InVertexBuffer;
		VertexBuffer.DataSize = InVertexBufferSize;

#if WITH_LOW_PRECISION
		if (bUseLowPrecision)
		{
			VertexFactory = new FPointCloudVertexFactoryLow(ERHIFeatureLevel::SM4);
		}
		else
#endif
		{
			VertexFactory = new FPointCloudVertexFactory(ERHIFeatureLevel::SM4);
		}

		VertexFactory->Init(&VertexBuffer);

		// Enqueue initialization of render resource
		BeginInitResource(&VertexBuffer);
		BeginInitResource(&IndexBuffer);
		BeginInitResource(&IndexBufferSpecial);
		BeginInitResource(VertexFactory);
	}
	virtual ~FPointCloudSectionProxy()
	{
		VertexBuffer.ReleaseResource();
		IndexBuffer.ReleaseResource();
		IndexBufferSpecial.ReleaseResource();
		VertexFactory->ReleaseResource();

		delete VertexFactory;
		VertexFactory = NULL;
	}

	virtual void SetupMesh(FMeshBatch &MeshBatch, bool bVisualizeLOD) override
	{
		MeshBatch.VertexFactory = VertexFactory;
		MeshBatch.Type = bVisualizeLOD ? PT_PointList : RenderMode;
		FMeshBatchElement& BatchElement = MeshBatch.Elements[0];
		BatchElement.IndexBuffer = bVisualizeLOD && RenderMode == PT_TriangleList ? &IndexBufferSpecial : &IndexBuffer;
		BatchElement.FirstIndex = 0;
		BatchElement.NumPrimitives = bVisualizeLOD && RenderMode == PT_TriangleList ? NumPrimitives[CurrentLOD] / 2 : NumPrimitives[CurrentLOD];
		BatchElement.MinVertexIndex = 0;
		BatchElement.MaxVertexIndex = VertexCount - 1;
		MeshBatch.LODIndex = CurrentLOD;

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
		BatchElement.VisualizeElementIndex = 0;
		MeshBatch.VisualizeLODIndex = CurrentLOD;
#endif
	}

	virtual UMaterialInterface* GetMaterial() const override { return Material; }
	virtual int32 GetLOD() const override { return CurrentLOD; }
	virtual bool NeedsRendering() override { return NumPrimitives[CurrentLOD] >= MinPointCount; }
	virtual void ComputeAndSetLOD(float ScreenSize) override
	{
		CurrentLOD = 0;

		for (int32 LODIndex = ScreenSizes.Num() - 1; LODIndex >= 0; --LODIndex)
		{
			if (ScreenSizes[LODIndex] > ScreenSize)
			{
				CurrentLOD = LODIndex;
				break;
			}
		}

		CurrentLOD = FMath::Clamp(CurrentLOD + LODBias, 0 , ScreenSizes.Num() - 1);

		if (MID)
		{
			float SSThis = ScreenSizes[CurrentLOD];
			float SSNext = CurrentLOD > 0 ? ScreenSizes[CurrentLOD - 1] : SSThis;

			float LODFactor = MID->K2_GetScalarParameterValue("PC__LODFactor");
			float LOD = SSThis == SSNext ? CurrentLOD : (CurrentLOD - (ScreenSize - SSThis) / (SSNext - SSThis));
			float Min = MID->K2_GetScalarParameterValue("PC__SpriteMin");
			float Max = MID->K2_GetScalarParameterValue("PC__SpriteMax");

			// CHANGED
			MID->SetScalarParameterValue("PC__Size", Max);
		}
	}
};

////////////////////////////////////////////////////////////
// FPointCloudSection

FPointCloudSection::FPointCloudSection(UPointCloud *InCloud, const TArray<FPointCloudPoint*> InPoints)
	: Cloud(InCloud)
	, Points(InPoints)
	, VertexBuffer(nullptr)
	, IndexBuffer(nullptr)
	, IndexBufferSpecial(nullptr)
	, VertexCount(0)
	, ProjectionMatrix(FPerspectiveMatrix(0.785398175f, 1920, 1080, 1.0f))
{
	LocalBounds = CalcBounds();

	if (Cloud)
	{
		Material = UMaterialInstanceDynamic::Create(Cloud->GetMaterial(), nullptr);
		Rebuild(true, true);
	}
}

FPointCloudSection::~FPointCloudSection()
{
	Dispose(true, true);
}

IPointCloudSectionProxy* FPointCloudSection::BuildProxy()
{
	return (!GetMaterial()) ? NULL : new FPointCloudSectionProxy(IndexBuffer, IndexBufferSpecial, IndexBufferSize, VertexBuffer, VertexBufferSize, Cloud->UsesSprites(), VertexCount, GetMaterial(), NumPrimitives, Cloud->MinimumSectionPointCount, ScreenSizes, Cloud->UsesLowPrecision(), Cloud->LODBias);
}

void FPointCloudSection::Rebuild(bool bBuildVB, bool bBuildIB)
{
	Dispose(bBuildVB, bBuildIB);

	if (bBuildVB)
	{
		BuildVB();
	}

	if (bBuildIB)
	{
		BuildIBAndLOD();
	}
}

void FPointCloudSection::Dispose(bool bVB, bool bIB)
{
	if (bVB && VertexBuffer)
	{
		delete[] VertexBuffer;
		VertexBuffer = nullptr;
	}

	if (bIB)
	{
		if (IndexBuffer)
		{
			delete[] IndexBuffer;
			IndexBuffer = nullptr;
		}

		if (IndexBufferSpecial)
		{
			delete[] IndexBufferSpecial;
			IndexBufferSpecial = nullptr;
		}
	}
}

FBoxSphereBounds FPointCloudSection::CalcBounds(const FTransform& Transform /*= FTransform::Identity*/) const
{
	FBox BoundingBox = FPointCloudHelper::CalculateBounds(Points, Transform);

	FBoxSphereBounds Bounds;
	Bounds.BoxExtent = BoundingBox.GetExtent();
	Bounds.Origin = BoundingBox.GetCenter();
	Bounds.SphereRadius = Bounds.BoxExtent.Size();

	return Bounds;
}

void FPointCloudSection::BuildVB()
{
	bool bUseSprites = Cloud->UsesSprites();

	TArray<FPointCloudPoint*> EnabledPoints = FPointCloudHelper::GetEnabledPoints(Points);

	VertexCount = (uint32)EnabledPoints.Num();
	VertexBufferSize = VertexCount * FPointCloudHelper::CalculatePointSize(Cloud, false);
	VertexBuffer = new uint8[VertexBufferSize];
	uint8* DataPtr = VertexBuffer;

	for (uint32 idx = 0; idx < VertexCount; idx++)
	{
		for (uint8 u = 0; u < (bUseSprites ? 4 : 1); u++)
		{
			if (Cloud->UsesLowPrecision())
			{
				// Converting into 16 bit RGB
				uint16 r = ((EnabledPoints[idx]->Color.R & 0xF8) << 8) & 0xF800;
				uint16 g = ((EnabledPoints[idx]->Color.G & 0xFC) << 3) & 0x07E0;
				uint16 b = ((EnabledPoints[idx]->Color.B & 0xF8) >> 3) & 0x001F;
				uint16 rgb = r | g | b;

				FFloat16 X(EnabledPoints[idx]->Location.X), Y(EnabledPoints[idx]->Location.Y), Z(EnabledPoints[idx]->Location.Z);

				FMemory::Memcpy(DataPtr, &X, sizeof(FFloat16));
				DataPtr += sizeof(FFloat16);
				FMemory::Memcpy(DataPtr, &Y, sizeof(FFloat16));
				DataPtr += sizeof(FFloat16);
				FMemory::Memcpy(DataPtr, &Z, sizeof(FFloat16));
				DataPtr += sizeof(FFloat16);
				FMemory::Memcpy(DataPtr, &rgb, sizeof(uint16));
				DataPtr += sizeof(uint16);
			}
			else
			{
				FMemory::Memcpy(DataPtr, &EnabledPoints[idx]->Location, sizeof(FVector));
				DataPtr += sizeof(FVector);
				FMemory::Memcpy(DataPtr, &EnabledPoints[idx]->Color, sizeof(FColor));
				DataPtr += sizeof(FColor);
			}
		}
	}

	DataPtr = nullptr;
}

void FPointCloudSection::BuildIBAndLOD()
{
	bool bUseSprites = Cloud->UsesSprites();
	bool *UniqueCheck = new bool[VertexCount];
	FMemory::Memset(UniqueCheck, 0, VertexCount);

	IndexBufferSize = VertexCount * sizeof(uint32) * (bUseSprites ? 6 : 1);
	IndexBuffer = new uint8[IndexBufferSize];
	uint8* DataPtr = IndexBuffer;
	IndexBufferSpecial = new uint8[VertexCount * sizeof(uint32)];
	uint8* DataPtrSpecial = IndexBufferSpecial;

	ScreenSizes.Empty();
	ScreenSizes.AddUninitialized(Cloud->LODCount);
	NumPrimitives.Empty();
	NumPrimitives.AddUninitialized(Cloud->LODCount);

	float aggr = 1 / Cloud->LODAggressiveness;

	uint32 numPrimitives = 0;
	for (int32 l = Cloud->LODCount - 1; l >= 0; l--)
	{
		double Skip = CalculateSkip(l);

		for (double PointIdx = 0; PointIdx < VertexCount; PointIdx += Skip)
		{
			if (PointIdx >= 0)
			{
				uint32 idx = (uint32)PointIdx;

				if (!UniqueCheck[idx])
				{
					UniqueCheck[idx] = true;

					if (bUseSprites)
					{
						uint32 idx0 = idx * 4;
						uint32 idx1 = idx0 + 1;
						uint32 idx2 = idx0 + 2;
						uint32 idx3 = idx0 + 3;

						FMemory::Memcpy(DataPtr, &idx0, sizeof(uint32)); DataPtr += sizeof(uint32);
						FMemory::Memcpy(DataPtr, &idx1, sizeof(uint32)); DataPtr += sizeof(uint32);
						FMemory::Memcpy(DataPtr, &idx2, sizeof(uint32)); DataPtr += sizeof(uint32);
						FMemory::Memcpy(DataPtr, &idx0, sizeof(uint32)); DataPtr += sizeof(uint32);
						FMemory::Memcpy(DataPtr, &idx2, sizeof(uint32)); DataPtr += sizeof(uint32);
						FMemory::Memcpy(DataPtr, &idx3, sizeof(uint32)); DataPtr += sizeof(uint32);

						numPrimitives += 2;

						// Special buffer
						FMemory::Memcpy(DataPtrSpecial, &idx0, sizeof(uint32));
						DataPtrSpecial += sizeof(uint32);
					}
					else
					{
						FMemory::Memcpy(DataPtr, &idx, sizeof(uint32));
						DataPtr += sizeof(uint32);
						numPrimitives++;
					}
				}
			}
		}

		NumPrimitives[l] = numPrimitives;
		ScreenSizes[l] = FMath::Square(ComputeBoundsScreenSize(FVector::ZeroVector, LocalBounds.SphereRadius, FVector(0.0f, 0.0f, (FMath::Pow(aggr, l) + 1) * LocalBounds.SphereRadius), ProjectionMatrix) * 0.5f);
	}

	delete[] UniqueCheck;
	DataPtr = nullptr;
	DataPtrSpecial = nullptr;
}
