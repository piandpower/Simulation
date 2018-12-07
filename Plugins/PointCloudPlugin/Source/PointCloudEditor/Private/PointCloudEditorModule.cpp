// Copyright 2018 Michal Cieciura. All Rights Reserved.

#include "IPointCloudEditorModule.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "LevelEditor.h"
#include "PointCloud.h"
#include "PointCloudFactory.h"
#include "PointCloudStyle.h"
#include "PointCloudDetails.h"

class FPointCloudEditorModule : public IPointCloudEditorModule
{
	virtual TSharedPtr<FExtensibilityManager> GetMenuExtensibilityManager() override { return MenuExtensibilityManager; }
	virtual TSharedPtr<FExtensibilityManager> GetToolBarExtensibilityManager() override { return ToolBarExtensibilityManager; }

	// Begin IModuleInterface Interface
	virtual void StartupModule() override
	{
		MenuExtensibilityManager = MakeShareable(new FExtensibilityManager);
		ToolBarExtensibilityManager = MakeShareable(new FExtensibilityManager);

		// Register slate style overrides
		FPointCloudStyle::Initialize();
		
		// Register asset type
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		AssetTools.RegisterAssetTypeActions(MakeShareable(new FAssetTypeActions_PointCloud));

		//if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
		//{
		//	SettingsModule->RegisterSettings("Editor", "ContentEditors", "SpriteEditor",
		//		LOCTEXT("SpriteEditorSettingsName", "Sprite Editor"),
		//		LOCTEXT("SpriteEditorSettingsDescription", "Configure the look and feel of the Sprite Editor."),
		//		GetMutableDefault<USpriteEditorSettings>());
		//}

		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

		PropertyModule.RegisterCustomPropertyTypeLayout("PointCloudFileHeader", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FPointCloudFileHeaderCustomization::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout("PointCloudImportSettingsLineRange", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FPointCloudLineRangeCustomization::MakeInstance));
	}
	virtual void ShutdownModule() override
	{
		MenuExtensibilityManager.Reset();
		ToolBarExtensibilityManager.Reset();

		if (UObjectInitialized())
		{
			//if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
			//{
			//	SettingsModule->UnregisterSettings("Editor", "ContentEditors", "SpriteEditor");
			//}
		}

		// Unregister the asset type that we registered
		if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
		{
			IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
			AssetTools.UnregisterAssetTypeActions(MakeShareable(new FAssetTypeActions_PointCloud));
		}
		
		// Unregister slate style overrides
		FPointCloudStyle::Shutdown();
	}
	// End IModuleInterface Interface
	
private:
	TSharedPtr<FExtensibilityManager> MenuExtensibilityManager;
	TSharedPtr<FExtensibilityManager> ToolBarExtensibilityManager;
};

IMPLEMENT_MODULE(FPointCloudEditorModule, PointCloudEditor)