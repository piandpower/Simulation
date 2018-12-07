// Copyright 2018 Michal Cieciura. All Rights Reserved.

#include "PointCloudComponent.h"
#include "Engine/CollisionProfile.h"
#include "PointCloudSection.h"
#include "IPointCloudSectionProxy.h"
#include "PrimitiveSceneProxy.h"
#include "MeshBatch.h"
#include "Engine.h"
#include "Runtime/Launch/Resources/Version.h"

////////////////////////////////////////////////////////////
// FPointCloudSceneProxy

class FPointCloudSceneProxy : public FPrimitiveSceneProxy
{
public:
	FPointCloudSceneProxy(UPointCloudComponent* Component) : FPrimitiveSceneProxy(Component)
	{
		Section = Component->GetSection()->BuildProxy();
		MaterialRelevance = Component->GetMaterialRelevance(GetScene().GetFeatureLevel());
	}

	virtual ~FPointCloudSceneProxy()
	{
		if (Section != NULL)
		{
			delete Section;
			Section = NULL;
		}
	}

	void BuildMeshBatch(FMeshBatch &MeshBatch, const FSceneView* View, bool bVisualizeLOD) const
	{
		if (Section && Section->GetMaterial())
		{
			const FBoxSphereBounds& ProxyBounds = GetBounds();
			Section->ComputeAndSetLOD(ComputeBoundsScreenRadiusSquared(ProxyBounds.Origin, ProxyBounds.SphereRadius, *View) * View->LODDistanceFactor * View->LODDistanceFactor);
			Section->SetupMesh(MeshBatch, bVisualizeLOD);

			MeshBatch.bWireframe = false;
			MeshBatch.MaterialRenderProxy = Section->GetMaterial()->GetRenderProxy(IsSelected());
			MeshBatch.ReverseCulling = IsLocalToWorldDeterminantNegative();
			MeshBatch.DepthPriorityGroup = SDPG_World;
			MeshBatch.bCanApplyViewModeOverrides = true;
			MeshBatch.Elements[0].PrimitiveUniformBuffer = CreatePrimitiveUniformBufferImmediate(GetLocalToWorld(), GetBounds(), GetLocalBounds(), true, UseEditorDepthTest());
		}
	}
	
	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
	{
		if (Section && Section->GetMaterial() && Section->NeedsRendering())
		{
			QUICK_SCOPE_CYCLE_COUNTER(STAT_PointCloudSceneProxy_GetDynamicMeshElements);

			const bool bVisualizeLOD = AllowDebugViewmodes() && ViewFamily.EngineShowFlags.LODColoration;

			for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
			{
				if (VisibilityMap & (1 << ViewIndex))
				{
					FMeshBatch& Mesh = Collector.AllocateMesh();
					BuildMeshBatch(Mesh, Views[ViewIndex], bVisualizeLOD);
					Collector.AddMesh(ViewIndex, Mesh);
				}
			}
		}
	}

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
	{
		FPrimitiveViewRelevance Result;

		if (Section != NULL)
		{
			Result.bDrawRelevance = IsShown(View);
			Result.bShadowRelevance = IsShadowCast(View);
			Result.bDynamicRelevance = true;
			Result.bStaticRelevance = false;
			Result.bRenderInMainPass = ShouldRenderInMainPass();
			Result.bUsesLightingChannels = GetLightingChannelMask() != GetDefaultLightingChannelMask();
			Result.bRenderCustomDepth = ShouldRenderCustomDepth();
			MaterialRelevance.SetPrimitiveViewRelevance(Result);
		}

		return Result;
	}

	virtual bool CanBeOccluded() const override { return !MaterialRelevance.bDisableDepthTest; }

	virtual uint32 GetMemoryFootprint(void) const override { return(sizeof(*this) + GetAllocatedSize()); }
		
	uint32 GetAllocatedSize(void) const { return(FPrimitiveSceneProxy::GetAllocatedSize()); }

#if ENGINE_MINOR_VERSION > 18
	virtual SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}
#endif

protected:
	FMaterialRelevance MaterialRelevance;
	IPointCloudSectionProxy *Section;
};

////////////////////////////////////////////////////////////
// UPointCloudComponent

UPointCloudComponent::UPointCloudComponent( const FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer )
{
	PrimaryComponentTick.bCanEverTick = false;
	SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
}

void UPointCloudComponent::SetSection(FPointCloudSection *InSection)
{
	Section = InSection;
	MarkRenderStateDirty();
	UpdateBounds();
}

FPrimitiveSceneProxy* UPointCloudComponent::CreateSceneProxy()
{
	FPrimitiveSceneProxy* Proxy = NULL;
	if(GetMaterial(0))
	{
		Proxy = new FPointCloudSceneProxy(this);
	}
	return Proxy;
}

UMaterialInterface* UPointCloudComponent::GetMaterial(int32 ElementIndex) const
{
	return Section ? Section->GetMaterial() : nullptr;
}

FBoxSphereBounds UPointCloudComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	return Section != NULL ? Section->GetWorldBounds(LocalToWorld) : USceneComponent::CalcBounds(LocalToWorld);
}

void UPointCloudComponent::DestroyComponent(bool bPromoteChildren /*= false*/)
{
	Section = NULL;
	Super::DestroyComponent(bPromoteChildren);
}

void UPointCloudComponent::AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector)
{
	UPointCloudComponent* This = CastChecked<UPointCloudComponent>(InThis);
	Super::AddReferencedObjects(This, Collector);

	if (This->GetMaterial(0))
	{
		UMaterialInterface *MI = This->GetMaterial(0);
		Collector.AddReferencedObject(MI);
	}
}
