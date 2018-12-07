// Copyright 2018 Michal Cieciura. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "EditorStyleSet.h"
#include "PointCloudStyle.h"

class FPointCloudEditorCommands : public TCommands<FPointCloudEditorCommands>
{
public:
	FPointCloudEditorCommands()
		: TCommands<FPointCloudEditorCommands>(
			TEXT("PointCloudEditor"), // Context name for fast lookup
			NSLOCTEXT("Contexts", "PointCloudEditor", "Point Cloud Editor"), // Localized context name for displaying
			NAME_None, // Parent
			FPointCloudStyle::Get()->GetStyleSetName() // Icon Style Set
			)
	{
	}

	// TCommand<> interface
	virtual void RegisterCommands() override;
	// End of TCommand<> interface

public:
	TSharedPtr<FUICommandInfo> RebuildCloud;
	TSharedPtr<FUICommandInfo> BakeCloud;
};