// Copyright 2018 Michal Cieciura.All Rights Reserved.

#include "PointCloudEditor.h"
#include "PointCloud.h"
#include "PointCloudHelper.h"
#include "EditorStyleSet.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/Commands/UICommandList.h"
#include "PointCloudEditorCommands.h"
#include "IPointCloudEditorModule.h"
#include "SSingleObjectDetailsPanel.h"
#include "SBorder.h"

#define LOCTEXT_NAMESPACE "PointCloudEditor"

const FName PointCloudEditorAppName = FName(TEXT("PointCloudEditorApp"));

struct FPointCloudEditorTabs
{
	static const FName DetailsID;
};

const FName FPointCloudEditorTabs::DetailsID(TEXT("Details"));

/////////////////////////////////////////////////////
// SPointCloudPropertiesTabBody

class SPointCloudPropertiesTabBody : public SSingleObjectDetailsPanel
{
public:
	SLATE_BEGIN_ARGS(SPointCloudPropertiesTabBody) {}
	SLATE_END_ARGS()

private:
	// Pointer back to owning sprite editor instance (the keeper of state)
	TWeakPtr<class FPointCloudEditor> PointCloudEditorPtr;
public:
	void Construct(const FArguments& InArgs, TSharedPtr<FPointCloudEditor> InPointCloudEditor)
	{
		PointCloudEditorPtr = InPointCloudEditor;

		SSingleObjectDetailsPanel::Construct(SSingleObjectDetailsPanel::FArguments().HostCommandList(InPointCloudEditor->GetToolkitCommands()).HostTabManager(InPointCloudEditor->GetTabManager()), /*bAutomaticallyObserveViaGetObjectToObserve=*/ true, /*bAllowSearch=*/ true);
	}

	// SSingleObjectDetailsPanel interface
	virtual UObject* GetObjectToObserve() const override
	{
		return PointCloudEditorPtr.Pin()->GetPointCloudBeingEdited();
	}

	virtual TSharedRef<SWidget> PopulateSlot(TSharedRef<SWidget> PropertyEditorWidget) override
	{
		return SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.FillHeight(1)
			[
				PropertyEditorWidget
			];
	}
	// End of SSingleObjectDetailsPanel interface
};

//////////////////////////////////////////////////////////////////////////
// FPointCloudEditor

FPointCloudEditor::FPointCloudEditor()
	: PointCloudBeingEdited(nullptr)
{
}

void FPointCloudEditor::RegisterTabSpawners(const TSharedRef<class FTabManager>& TabManager)
{
	WorkspaceMenuCategory = TabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_PointCloudEditor", "Point Cloud Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(TabManager);

	TabManager->RegisterTabSpawner(FPointCloudEditorTabs::DetailsID, FOnSpawnTab::CreateSP(this, &FPointCloudEditor::SpawnTab_Details))
		.SetDisplayName(LOCTEXT("DetailsTabLabel", "Details"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));
}

void FPointCloudEditor::UnregisterTabSpawners(const TSharedRef<class FTabManager>& TabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(TabManager);

	TabManager->UnregisterTabSpawner(FPointCloudEditorTabs::DetailsID);
}

FName FPointCloudEditor::GetToolkitFName() const
{
	return FName("PointCloudEditor");
}

FText FPointCloudEditor::GetBaseToolkitName() const
{
	return LOCTEXT("PointCloudEditorAppLabel", "Point Cloud Editor");
}

FText FPointCloudEditor::GetToolkitName() const
{
	const bool bDirtyState = PointCloudBeingEdited->GetOutermost()->IsDirty();

	FFormatNamedArguments Args;
	Args.Add(TEXT("PointCloudName"), FText::FromString(PointCloudBeingEdited->GetName()));
	Args.Add(TEXT("DirtyState"), bDirtyState ? FText::FromString(TEXT("*")) : FText::GetEmpty());
	return FText::Format(LOCTEXT("PointCloudEditorToolkitName", "{PointCloudName}{DirtyState}"), Args);
}

FText FPointCloudEditor::GetToolkitToolTipText() const
{
	return FAssetEditorToolkit::GetToolTipTextForObject(PointCloudBeingEdited);
}

FLinearColor FPointCloudEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor::White;
}

FString FPointCloudEditor::GetWorldCentricTabPrefix() const
{
	return TEXT("PointCloudEditor");
}

void FPointCloudEditor::OnToolkitHostingStarted(const TSharedRef<class IToolkit>& Toolkit)
{
}

void FPointCloudEditor::OnToolkitHostingFinished(const TSharedRef<class IToolkit>& Toolkit)
{
}

void FPointCloudEditor::AddReferencedObjects(FReferenceCollector& Collector)
{
	if (IsValid(PointCloudBeingEdited))
	{
		Collector.AddReferencedObject(PointCloudBeingEdited);
	}
}

void FPointCloudEditor::InitPointCloudEditor(const EToolkitMode::Type Mode, const TSharedPtr< class IToolkitHost >& InitToolkitHost, class UPointCloud* InitPointCloud)
{
	PointCloudBeingEdited = InitPointCloud;

	if (PointCloudBeingEdited)
	{
		PointCloudBeingEdited->OnPointCloudChanged().Add(TBaseDelegate<void>::CreateSP(this, &FPointCloudEditor::OnPointCloudChanged));
	}

	FPointCloudEditorCommands::Register();

	BindCommands();

	TSharedPtr<FPointCloudEditor> PointCloudEditor = SharedThis(this);

	// Default layout
	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("Standalone_PointCloudEditor_Layout_v1")
		->AddArea
		(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Vertical)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.1f)
				->SetHideTabWell(true)
				->AddTab(GetToolbarTabId(), ETabState::OpenedTab)
			)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.9f)
				->SetHideTabWell(true)
				->AddTab(FPointCloudEditorTabs::DetailsID, ETabState::OpenedTab)
			)
		);

	// Initialize the asset editor
	InitAssetEditor(Mode, InitToolkitHost, PointCloudEditorAppName, StandaloneDefaultLayout, /*bCreateDefaultStandaloneMenu=*/ true, /*bCreateDefaultToolbar=*/ true, InitPointCloud);
	
	// Extend things
	ExtendMenu();
	ExtendToolbar();
	RegenerateMenusAndToolbars();

	UpdateCloudStatistics();
}

void FPointCloudEditor::BindCommands()
{
	PointCloudCommands = MakeShareable(new FUICommandList);

	PointCloudCommands->MapAction(
		FPointCloudEditorCommands::Get().RebuildCloud,
		FExecuteAction::CreateSP(this, &FPointCloudEditor::ExecuteCommand_Rebuild));

	PointCloudCommands->MapAction(
		FPointCloudEditorCommands::Get().BakeCloud,
		FExecuteAction::CreateSP(this, &FPointCloudEditor::ExecuteCommand_Bake));
}

void FPointCloudEditor::ExtendMenu()
{
}

void FPointCloudEditor::ExtendToolbar()
{
	struct Local
	{
		static void FillToolbar(FToolBarBuilder& ToolbarBuilder)
		{
			const FPointCloudEditorCommands& PointCloudCommands = FPointCloudEditorCommands::Get();

			ToolbarBuilder.BeginSection("Command");
			{
				ToolbarBuilder.AddToolBarButton(PointCloudCommands.RebuildCloud, NAME_None, TAttribute<FText>(), TAttribute<FText>(), FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Build"));
				ToolbarBuilder.AddToolBarButton(PointCloudCommands.BakeCloud, NAME_None, TAttribute<FText>(), TAttribute<FText>(), FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Recompile"));
			}
			ToolbarBuilder.EndSection();
		}
	};

	TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);

	ToolbarExtender->AddToolBarExtension(
		"Asset",
		EExtensionHook::After,
		PointCloudCommands->AsShared(),
		FToolBarExtensionDelegate::CreateStatic(&Local::FillToolbar)
	);

	AddToolbarExtender(ToolbarExtender);

	AddToolbarExtender(IPointCloudEditorModule::Get().GetToolBarExtensibilityManager()->GetAllExtenders());
}

TSharedRef<SWidget> FPointCloudEditor::BuildPointCloudStatistics()
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(0.5f)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
				.AutoHeight()
				.VAlign(VAlign_Center)
				.Padding(4.0f)
				[
					SAssignNew(TotalCountWidget, STextBlock)
				]

			+ SVerticalBox::Slot()
				.AutoHeight()
				.VAlign(VAlign_Center)
				.Padding(4.0f)
				[
					SAssignNew(EnabledCountWidget, STextBlock)
				]

			+ SVerticalBox::Slot()
				.AutoHeight()
				.VAlign(VAlign_Center)
				.Padding(4.0f)
				[
					SAssignNew(ApproxBoundsWidget, STextBlock)
				]

			+ SVerticalBox::Slot()
				.AutoHeight()
				.VAlign(VAlign_Center)
				.Padding(4.0f)
				[
					SAssignNew(ApproxSectionCountWidget, STextBlock)
				]
		]

		+ SHorizontalBox::Slot()
		.FillWidth(0.5f)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
				.AutoHeight()
				.VAlign(VAlign_Center)
				.Padding(4.0f)
				[
					SAssignNew(PointSizeWidget, STextBlock)
				]

			+ SVerticalBox::Slot()
				.AutoHeight()
				.VAlign(VAlign_Center)
				.Padding(4.0f)
				[
					SAssignNew(TotalSizeWidget, STextBlock)
				]

			+ SVerticalBox::Slot()
				.AutoHeight()
				.VAlign(VAlign_Center)
				.Padding(4.0f)
				[
					SAssignNew(ColorDataWidget, STextBlock)
				]

			+ SVerticalBox::Slot()
				.AutoHeight()
				.VAlign(VAlign_Center)
				.Padding(4.0f)
				[
					SAssignNew(CloudStatusWidget, STextBlock)
				]
		];
}

TSharedRef<SDockTab> FPointCloudEditor::SpawnTab_Details(const FSpawnTabArgs& Args)
{
	TSharedPtr<FPointCloudEditor> PointCloudEditorPtr = SharedThis(this);

	// Spawn the tab
	return SNew(SDockTab)
		.Label(LOCTEXT("DetailsTab_Title", "Details"))
		[
			SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(2.0f)
				[
					SNew(SBorder)
					[
						BuildPointCloudStatistics()
					]
				]

				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				.Padding(2.0f)
				[
					SNew(SBorder)
					.Padding(4.0f)
					[
						SNew(SPointCloudPropertiesTabBody, PointCloudEditorPtr)
					]
				]
		];
}

void FPointCloudEditor::ExecuteCommand_Rebuild()
{
	if (PointCloudBeingEdited)
	{
		PointCloudBeingEdited->Rebuild();
	}
}

void FPointCloudEditor::ExecuteCommand_Bake()
{
	if (PointCloudBeingEdited)
	{
		PointCloudBeingEdited->Bake();
	}
}

void FPointCloudEditor::UpdateCloudStatistics()
{
	uint64 Size = FPointCloudHelper::CalculatePointSize(PointCloudBeingEdited);
	int32 TotalCount = PointCloudBeingEdited->GetPointCount(false);
	int32 EnabledCount = PointCloudBeingEdited->GetPointCount(true);

	TotalCountWidget->SetText(FText::Format(LOCTEXT("SrcCount", "Total Point Count: {0}"), TotalCount));
	EnabledCountWidget->SetText(FText::Format(LOCTEXT("EnabledCount", "Enabled Point Count: {0} ({1} %)"), EnabledCount, FMath::RoundToInt(EnabledCount * 1000.0f / TotalCount) * 0.1f));
	PointSizeWidget->SetText(FText::Format(LOCTEXT("PointSize", "Point Size: {0} bytes"), Size));
	TotalSizeWidget->SetText(FText::Format(LOCTEXT("TotalSize", "Total Size: {0} MB"), FMath::RoundToInt(Size * EnabledCount / 10485.76f) * 0.01f));
	ColorDataWidget->SetText(FText::FromString(FString("Color Data: ").Append(FPointCloudHelper::GetColorModeAsString(PointCloudBeingEdited))));
	
	FVector BoundingSize = PointCloudBeingEdited->GetBounds().GetBox().GetSize();

	ApproxBoundsWidget->SetText(FText::Format(LOCTEXT("ApproxBounds", "Approximate Bounds: {0} x {1} x {2}"), FMath::CeilToInt(BoundingSize.X), FMath::CeilToInt(BoundingSize.Y), FMath::CeilToInt(BoundingSize.Z)));
	ApproxSectionCountWidget->SetText(FText::Format(LOCTEXT("SectionCount", "Section Count: {0}"), PointCloudBeingEdited->GetSectionCount()));

	if (PointCloudBeingEdited->IsDirty())
	{
		CloudStatusWidget->SetText(LOCTEXT("DirtyCloud", "Status: Cloud needs rebuilding"));
		CloudStatusWidget->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.02f, 0.02f)));
	}
	else
	{
		CloudStatusWidget->SetText(LOCTEXT("DirtyCloud", "Status: Cloud ready"));
		CloudStatusWidget->SetColorAndOpacity(FSlateColor(FLinearColor(0, 0.8f, 0)));
	}
}

void FPointCloudEditor::OnPointCloudChanged()
{
	UpdateCloudStatistics();
}

#undef LOCTEXT_NAMESPACE