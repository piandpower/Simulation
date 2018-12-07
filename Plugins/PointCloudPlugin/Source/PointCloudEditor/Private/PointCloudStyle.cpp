// Copyright 2018 Michal Cieciura.All Rights Reserved.

#include "PointCloudStyle.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateTypes.h"
#include "EditorStyleSet.h"
#include "IPluginManager.h"

#define IMAGE_PLUGIN_BRUSH( RelativePath, ... ) FSlateImageBrush( FPointCloudStyle::InContent( RelativePath, ".png" ), __VA_ARGS__ )

TSharedPtr<FSlateStyleSet> FPointCloudStyle::StyleSet = nullptr;
TSharedPtr<class ISlateStyle> FPointCloudStyle::Get() { return StyleSet; }

void FPointCloudStyle::Initialize()
{
	// Const icon & thumbnail sizes
	const FVector2D Icon16x16(16.0f, 16.0f);
	const FVector2D Icon40x40(40.0f, 40.0f);
	const FVector2D Icon128x128(128.0f, 128.0f);

	// Only register once
	if (StyleSet.IsValid())
	{
		return;
	}


	StyleSet = MakeShareable(new FSlateStyleSet(GetStyleSetName()));
	StyleSet->SetContentRoot(FPaths::EngineContentDir() / TEXT("Editor/Slate"));
	StyleSet->SetCoreContentRoot(FPaths::EngineContentDir() / TEXT("Slate"));

	//StyleSet->Set("PointCloudEditor.RebuildCloud", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/Icon"), Icon40x40));

	FString ContentDir = IPluginManager::Get().FindPlugin(TEXT("PointCloudPlugin"))->GetContentDir();
	StyleSet->SetContentRoot(ContentDir);

	StyleSet->Set("ClassIcon.PointCloud", new IMAGE_PLUGIN_BRUSH("icon_32", Icon16x16));
	StyleSet->Set("ClassThumbnail.PointCloud", new IMAGE_PLUGIN_BRUSH("icon_128", Icon128x128));

	StyleSet->Set("ClassIcon.PointCloudActor", new IMAGE_PLUGIN_BRUSH("icon_32", Icon16x16));
	StyleSet->Set("ClassThumbnail.PointCloudActor", new IMAGE_PLUGIN_BRUSH("icon_128", Icon128x128));

	StyleSet->Set("ClassIcon.PointCloudComponent", new IMAGE_PLUGIN_BRUSH("icon_32", Icon16x16));
	StyleSet->Set("ClassThumbnail.PointCloudComponent", new IMAGE_PLUGIN_BRUSH("icon_128", Icon128x128));

	FSlateStyleRegistry::RegisterSlateStyle(*StyleSet.Get());
}

#undef IMAGE_PLUGIN_BRUSH

void FPointCloudStyle::Shutdown()
{
	if (StyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet.Get());
		ensure(StyleSet.IsUnique());
		StyleSet.Reset();
	}
}

FName FPointCloudStyle::GetStyleSetName()
{
	static FName PaperStyleName(TEXT("PointCloudStyle"));
	return PaperStyleName;
}

FString FPointCloudStyle::InContent(const FString& RelativePath, const ANSICHAR* Extension)
{
	static FString ContentDir = IPluginManager::Get().FindPlugin(TEXT("PointCloudPlugin"))->GetContentDir() / TEXT("Icons");
	return (ContentDir / RelativePath) + Extension;
}

