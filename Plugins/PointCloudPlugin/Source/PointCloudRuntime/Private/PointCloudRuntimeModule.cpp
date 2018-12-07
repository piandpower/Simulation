// Copyright 2018 Michal Cieciura. All Rights Reserved.

#include "IPointCloudRuntimeModule.h"

class FPointCloudRuntimeModule : public IPointCloudRuntimeModule
{
	// Begin IModuleInterface Interface
	virtual void StartupModule() override {}
	virtual void ShutdownModule() override {}
	// End IModuleInterface Interface
};

IMPLEMENT_MODULE(FPointCloudRuntimeModule, PointCloudRuntime)

