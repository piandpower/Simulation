// Copyright 2018 Michal Cieciura. All Rights Reserved.

#include "PointCloudFactory.h"
#include "PointCloudHelper.h"
#include "PointCloudShared.h"
#include "PointCloudImportUI.h"
#include "PointCloudEditor.h"
#include "PointCloudStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogFactory, Log, All);

#define LOCTEXT_NAMESPACE "PointCloud"

FText FAssetTypeActions_PointCloud::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "FAssetTypeActions_PointCloud", "Point Cloud");
}

void FAssetTypeActions_PointCloud::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor /*= TSharedPtr<IToolkitHost>()*/)
{
	const EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		if (UPointCloud* PointCloud = Cast<UPointCloud>(*ObjIt))
		{
			TSharedRef<FPointCloudEditor> NewPointCloudEditor(new FPointCloudEditor());
			NewPointCloudEditor->InitPointCloudEditor(Mode, EditWithinLevelEditor, PointCloud);
		}
	}
}

UPointCloudFactory::UPointCloudFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = false;
	bEditorImport = true;
	Formats.Add(TEXT("xyz;Point Cloud"));
	//Formats.Add(TEXT("csv;Point Cloud"));
	Formats.Add(TEXT("txt;Point Cloud"));
	//Formats.Add(TEXT("bin;Point Cloud"));
	//Formats.Add(TEXT("las;Point Cloud"));
	SupportedClass = UPointCloud::StaticClass();
}

UObject* UPointCloudFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
	UObject *OutObject = NULL;

	FEditorDelegates::OnAssetPreImport.Broadcast(this, InClass, InParent, InName, *FPaths::GetExtension(Filename));

	ImportSettings = FPointCloudImportUI::ShowImportDialog(IsTextFile(Filename), Filename);
	if (ImportSettings == NULL)
	{
		bOutOperationCanceled = true;
	}
	else
	{
		Warn->BeginSlowTask(NSLOCTEXT("PointCloudFactory", "BeginImportingPointCloudTask", "Importing Point Cloud"), true);

		OutObject = UPointCloudStatics::LoadPointCloudFromFile_Full(Filename, ImportSettings, InParent, InName, Flags);
		
		Warn->EndSlowTask();
	}

	FEditorDelegates::OnAssetPostImport.Broadcast(this, OutObject);

	return OutObject;
}

bool UPointCloudFactory::DoesSupportClass(UClass* Class)
{
	UE_LOG(LogTemp, Log, TEXT("DoesSupportClass"));
	return false;
}

#undef LOCTEXT_NAMESPACE