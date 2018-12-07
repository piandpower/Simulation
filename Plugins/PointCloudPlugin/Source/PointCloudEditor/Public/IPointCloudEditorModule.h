// Copyright 2018 Michal Cieciura. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "Toolkits/AssetEditorToolkit.h"

#define POINTCLOUD_EDITOR_MODULE_NAME "Paper2DEditor"

/**
* The public interface to this module
*/
class IPointCloudEditorModule : public IModuleInterface
{

public:
	virtual TSharedPtr<class FExtensibilityManager> GetMenuExtensibilityManager() { return nullptr; }
	virtual TSharedPtr<class FExtensibilityManager> GetToolBarExtensibilityManager() { return nullptr; }

	/**
	* Singleton-like access to this module's interface.  This is just for convenience!
	* Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
	*
	* @return Returns singleton instance, loading the module on demand if needed
	*/
	static inline IPointCloudEditorModule& Get()
	{
		return FModuleManager::LoadModuleChecked<IPointCloudEditorModule>("PointCloudEditor");
	}

	/**
	* Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
	*
	* @return True if the module is loaded and ready to use
	*/
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("PointCloudEditor");
	}
};

