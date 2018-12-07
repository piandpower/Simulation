// Copyright 2018 Michal Cieciura. All Rights Reserved.

#pragma once

struct FMeshBatch;
class UMaterialInterface;

/** Interface for the section proxy  */
class IPointCloudSectionProxy
{
public:
	virtual ~IPointCloudSectionProxy() {}
	virtual void SetupMesh(FMeshBatch &MeshBatch, bool bVisualizeLOD) = 0;
	virtual UMaterialInterface* GetMaterial() const = 0;
	virtual bool NeedsRendering() = 0;
	virtual void ComputeAndSetLOD(float ScreenSize) = 0;
	virtual int32 GetLOD() const = 0;
};

