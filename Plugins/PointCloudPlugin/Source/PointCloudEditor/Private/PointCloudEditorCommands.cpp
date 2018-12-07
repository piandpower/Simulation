// Copyright 2018 Michal Cieciura.All Rights Reserved.

#include "PointCloudEditorCommands.h"

#define LOCTEXT_NAMESPACE "PointCloudEditor"

//////////////////////////////////////////////////////////////////////////
// FSpriteEditorCommands

void FPointCloudEditorCommands::RegisterCommands()
{
	UI_COMMAND(RebuildCloud, "Rebuild Cloud", "Rebuilds the cloud using current settings", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(BakeCloud, "Bake Cloud", "Permanently discards disabled points", EUserInterfaceActionType::Button, FInputChord());
}

//////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
