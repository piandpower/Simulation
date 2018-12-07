// Copyright 2018 Michal Cieciura. All Rights Reserved.

#include "PointCloudImportUI.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IMainFrameModule.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Input/SButton.h"
#include "EditorStyleSet.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "PointCloudHelper.h"

#define LOCTEXT_NAMESPACE "PointCloudImportUI"

/////////////////////////////////////////////////
// SPointCloudOptionWindow

void SPointCloudOptionWindow::Construct(const FArguments& InArgs)
{
	ImportUI = InArgs._ImportUI;
	WidgetWindow = InArgs._WidgetWindow;

	check (ImportUI);
	
	TSharedPtr<SHorizontalBox> HeaderButtons;
	TSharedPtr<SBox> InspectorBox;
	this->ChildSlot
	[
		SNew(SBox)
		.MaxDesiredHeight(InArgs._MaxWindowHeight)
		.MaxDesiredWidth(InArgs._MaxWindowWidth)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(2)
			[
				SAssignNew(InspectorBox, SBox)
				.MaxDesiredHeight(750.0f)
				.WidthOverride(400.0f)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Right)
			.Padding(2)
			[
				SNew(SUniformGridPanel)
				.SlotPadding(2)
				+ SUniformGridPanel::Slot(0, 0)
				[
					SAssignNew(ImportButton, SButton)
					.HAlign(HAlign_Center)
					.Text(LOCTEXT("PointCloudOptionWindow_Import", "Import"))
					.IsEnabled(this, &SPointCloudOptionWindow::CanImport)
					.OnClicked(this, &SPointCloudOptionWindow::OnImport)
				]
				+ SUniformGridPanel::Slot(1, 0)
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.Text(LOCTEXT("PointCloudOptionWindow_Cancel", "Cancel"))
					.ToolTipText(LOCTEXT("PointCloudOptionWindow_Cancel_ToolTip", "Cancels importing this Point Cloud file"))
					.OnClicked(this, &SPointCloudOptionWindow::OnCancel)
				]
			]
		]
	];

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

	InspectorBox->SetContent(DetailsView->AsShared());
		
	DetailsView->SetObject(ImportUI);
}

/////////////////////////////////////////////////
// FPointCloudImportUI

UPointCloudImportSettings * FPointCloudImportUI::ShowImportDialog(bool bTextImporter, const FString& Filename)
{
	TSharedPtr<SWindow> ParentWindow;

	if (FModuleManager::Get().IsModuleLoaded("MainFrame"))
	{
		IMainFrameModule& MainFrame = FModuleManager::LoadModuleChecked<IMainFrameModule>("MainFrame");
		ParentWindow = MainFrame.GetParentWindow();
	}

	const float ImportWindowWidth = 410.0f;
	const float ImportWindowHeight = 750.0f;
	FVector2D ImportWindowSize = FVector2D(ImportWindowWidth, ImportWindowHeight); // Max window size it can get based on current slate

	FSlateRect WorkAreaRect = FSlateApplicationBase::Get().GetPreferredWorkArea();
	FVector2D DisplayTopLeft(WorkAreaRect.Left, WorkAreaRect.Top);
	FVector2D DisplaySize(WorkAreaRect.Right - WorkAreaRect.Left, WorkAreaRect.Bottom - WorkAreaRect.Top);

	float ScaleFactor = FPlatformApplicationMisc::GetDPIScaleFactorAtPoint(DisplayTopLeft.X, DisplayTopLeft.Y);
	ImportWindowSize *= ScaleFactor;

	FVector2D WindowPosition = (DisplayTopLeft + (DisplaySize - ImportWindowSize) / 2.0f) / ScaleFactor;

	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(NSLOCTEXT("UnrealEd", "PointCloudImportOpionsTitle", "Point Cloud Import Options"))
		.SizingRule(ESizingRule::Autosized)
		.AutoCenter(EAutoCenter::None)
		.ClientSize(ImportWindowSize)
		.ScreenPosition(WindowPosition);

	UPointCloudImportSettings *ImportUI = NewObject<UPointCloudImportSettings>();

	if (bTextImporter)
	{
		ImportUI->LoadFileHeader(Filename);
	}

	TSharedPtr<SPointCloudOptionWindow> PointCloudOptionWindow;
	Window->SetContent
	(
		SAssignNew(PointCloudOptionWindow, SPointCloudOptionWindow)
		.ImportUI(ImportUI)
		.bUseTextImporter(bTextImporter)
		.WidgetWindow(Window)
		.MaxWindowHeight(ImportWindowHeight)
		.MaxWindowWidth(ImportWindowWidth)
	);

	// @todo: we can make this slow as showing progress bar later
	FSlateApplication::Get().AddModalWindow(Window, ParentWindow, false);

	if (PointCloudOptionWindow->bCancelled)
	{
		delete ImportUI;
		ImportUI = NULL;
	}

	return ImportUI;
}

#undef LOCTEXT_NAMESPACE