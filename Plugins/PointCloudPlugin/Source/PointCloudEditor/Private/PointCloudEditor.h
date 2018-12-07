// Copyright 2018 Michal Cieciura. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/GCObject.h"
#include "Toolkits/IToolkitHost.h"
#include "Toolkits/AssetEditorToolkit.h"

class UPointCloud;
class SWidget;

class FPointCloudEditor : public FAssetEditorToolkit, public FGCObject
{
public:
	FPointCloudEditor();

	// IToolkit interface
	virtual void RegisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;
	// End of IToolkit interface

	// FAssetEditorToolkit
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FText GetToolkitName() const override;
	virtual FText GetToolkitToolTipText() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual void OnToolkitHostingStarted(const TSharedRef<class IToolkit>& Toolkit) override;
	virtual void OnToolkitHostingFinished(const TSharedRef<class IToolkit>& Toolkit) override;
	// End of FAssetEditorToolkit

	// FSerializableObject interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	// End of FSerializableObject interface

public:
	void InitPointCloudEditor(const EToolkitMode::Type Mode, const TSharedPtr< class IToolkitHost >& InitToolkitHost, class UPointCloud* InitPointCloud);

	UPointCloud* GetPointCloudBeingEdited() const { return PointCloudBeingEdited; }

protected:
	UPointCloud* PointCloudBeingEdited;
	TSharedPtr<FUICommandList> PointCloudCommands;

	TSharedPtr<STextBlock> TotalCountWidget;
	TSharedPtr<STextBlock> EnabledCountWidget;
	TSharedPtr<STextBlock> PointSizeWidget;
	TSharedPtr<STextBlock> TotalSizeWidget;
	TSharedPtr<STextBlock> ApproxBoundsWidget;
	TSharedPtr<STextBlock> ApproxSectionCountWidget;
	TSharedPtr<STextBlock> ColorDataWidget;
	TSharedPtr<STextBlock> CloudStatusWidget;

protected:
	void BindCommands();
	void ExtendMenu();
	void ExtendToolbar();

	TSharedRef<SWidget> BuildPointCloudStatistics();
	TSharedRef<SDockTab> SpawnTab_Details(const FSpawnTabArgs& Args);

	void ExecuteCommand_Rebuild();
	void ExecuteCommand_Bake();

	void UpdateCloudStatistics();

	void OnPointCloudChanged(); 
};