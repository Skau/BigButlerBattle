// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "BBBCSVImportOptions.h"
#include "UObject/UObjectHash.h"
#include "UObject/UObjectIterator.h"
#include "UObject/Package.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "EditorStyleSet.h"
#include "Engine/UserDefinedStruct.h"
#include "Engine/DataTable.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "ObjectEditorUtils.h"
#include "DataTableEditorUtils.h"

#define LOCTEXT_NAMESPACE "CSVImportFactory"

void SBBBCSVImportOptions::Construct(const FArguments& InArgs)
{
	WidgetWindow = InArgs._WidgetWindow;
	TempImportDataTable = InArgs._TempImportDataTable;

	// Make array of enum pointers
	TSharedPtr<EBBBCSVImportType> DataTableTypePtr = MakeShareable(new EBBBCSVImportType(EBBBCSVImportType::ECSV_DataTable));
	ImportTypes.Add( DataTableTypePtr );
	ImportTypes.Add(MakeShareable(new EBBBCSVImportType(EBBBCSVImportType::ECSV_CurveTable)));
	ImportTypes.Add(MakeShareable(new EBBBCSVImportType(EBBBCSVImportType::ECSV_CurveFloat)));
	ImportTypes.Add(MakeShareable(new EBBBCSVImportType(EBBBCSVImportType::ECSV_CurveVector)));
	ImportTypes.Add(MakeShareable(new EBBBCSVImportType(EBBBCSVImportType::ECSV_BezierDataTable)));

	// Create properties view
	FPropertyEditorModule & EditModule = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs(/*bUpdateFromSelection=*/ false, /*bLockable=*/ false, /*bAllowSearch=*/ false, /*InNameAreaSettings=*/ FDetailsViewArgs::HideNameArea, /*bHideSelectionTip=*/ true);
	PropertyView = EditModule.CreateDetailView(DetailsViewArgs);

	PropertyView->SetIsPropertyVisibleDelegate(FIsPropertyVisible::CreateLambda([](const FPropertyAndParent& InPropertyAndParent)
	{
		static FName ImportOptions = FName(TEXT("ImportOptions"));

		// Only show import options
		FName CategoryName = FObjectEditorUtils::GetCategoryFName(&InPropertyAndParent.Property);

		if (CategoryName == ImportOptions)
		{
			return true;
		}

		return false;
	}));

	RowStructCombo = FDataTableEditorUtils::MakeRowStructureComboBox(FDataTableEditorUtils::FOnDataTableStructSelected::CreateSP(this, &SBBBCSVImportOptions::OnStructSelected));
	RowStructCombo->SetVisibility(TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateSP(this, &SBBBCSVImportOptions::GetTableRowOptionVis)));

	// Create widget
	this->ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FEditorStyle::GetBrush(TEXT("Menu.Background")))
		.Padding(10)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBorder)
				.Padding(FMargin(3))
				.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
				.Visibility( InArgs._FullPath.IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible )
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(STextBlock)
						.Font(FEditorStyle::GetFontStyle("CurveEd.LabelFont"))
						.Text(LOCTEXT("Import_CurrentFileTitle", "Current File: "))
					]
					+SHorizontalBox::Slot()
					.Padding(5, 0, 0, 0)
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Font(FEditorStyle::GetFontStyle("CurveEd.InfoFont"))
						.Text(InArgs._FullPath)
					]
				]
			]

			// Import type
			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(2)
			[
				SNew(STextBlock)
				.Text( LOCTEXT("ChooseAssetType", "Import As:") )
			]
			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SAssignNew(ImportTypeCombo, SComboBox< TSharedPtr<EBBBCSVImportType> >)
				.OptionsSource( &ImportTypes )
				.OnGenerateWidget( this, &SBBBCSVImportOptions::MakeImportTypeItemWidget )
				.OnSelectionChanged( this, &SBBBCSVImportOptions::OnImportTypeSelected)
				[
					SNew(STextBlock)
					.Text(this, &SBBBCSVImportOptions::GetSelectedItemText)
				]
			]
			// Data row struct
			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(2)
			[
				SNew(STextBlock)
				.Text( LOCTEXT("ChooseRowType", "Choose DataTable Row Type:") )
				.Visibility( this, &SBBBCSVImportOptions::GetTableRowOptionVis )
			]
			+SVerticalBox::Slot()
			.AutoHeight()
			[
				RowStructCombo.ToSharedRef()
			]
			// Curve interpolation
			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(2)
			[
				SNew(STextBlock)
				.Text( LOCTEXT("ChooseCurveType", "Choose Curve Interpolation Type:") )
				.Visibility( this, &SBBBCSVImportOptions::GetCurveTypeVis )
			]
			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SAssignNew(CurveInterpCombo, SComboBox<CurveInterpModePtr>)
				.OptionsSource( &CurveInterpModes )
				.OnGenerateWidget( this, &SBBBCSVImportOptions::MakeCurveTypeWidget )
				.Visibility( this, &SBBBCSVImportOptions::GetCurveTypeVis )
				[
					SNew(STextBlock)
					.Text(this, &SBBBCSVImportOptions::GetSelectedCurveTypeText)
				]
			]
			// Details panel
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(2)
			[
				SNew(SBox)
				.WidthOverride(400)
				.Visibility(this, &SBBBCSVImportOptions::GetDetailsPanelVis)
				[
					PropertyView.ToSharedRef()
				]
			]
			// Ok/Cancel
			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2)
				[
					SNew(SButton)
					.Text(LOCTEXT("OK", "OK"))
					.OnClicked( this, &SBBBCSVImportOptions::OnImport )
					.IsEnabled( this, &SBBBCSVImportOptions::CanImport )
				]
				+SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2)
				[
					SNew(SButton)
					.Text(LOCTEXT("Cancel", "Cancel"))
					.OnClicked( this, &SBBBCSVImportOptions::OnCancel )
				]
			]
		]
	];

	// set-up selection
	ImportTypeCombo->SetSelectedItem(DataTableTypePtr);
	PropertyView->SetObject(TempImportDataTable.Get());

	// Populate the valid interpolation modes
	{
		CurveInterpModes.Add( MakeShareable( new ERichCurveInterpMode(ERichCurveInterpMode::RCIM_Constant) ) );
		CurveInterpModes.Add( MakeShareable( new ERichCurveInterpMode(ERichCurveInterpMode::RCIM_Linear) ) );
		CurveInterpModes.Add( MakeShareable( new ERichCurveInterpMode(ERichCurveInterpMode::RCIM_Cubic) ) );
	}

	// NB: Both combo boxes default to first item in their options lists as initially selected item
}

	/** If we should import */
bool SBBBCSVImportOptions::ShouldImport()
{
	return ((SelectedStruct != nullptr) || GetSelectedImportType() != static_cast<uint8>(EBBBCSVImportType::ECSV_DataTable)) && bImport;
}

/** Get the row struct we selected */
UScriptStruct* SBBBCSVImportOptions::GetSelectedRowStruct()
{
	return SelectedStruct;
}

/** Get the import type we selected */
uint8 SBBBCSVImportOptions::GetSelectedImportType()
{
	return SelectedImportType;
}

/** Get the interpolation mode we selected */
ERichCurveInterpMode SBBBCSVImportOptions::GetSelectedCurveIterpMode()
{
	return SelectedCurveInterpMode;
}
	
/** Whether to show table row options */
EVisibility SBBBCSVImportOptions::GetTableRowOptionVis() const
{
	return (ImportTypeCombo.IsValid() && *ImportTypeCombo->GetSelectedItem() == EBBBCSVImportType::ECSV_DataTable) ? EVisibility::Visible : EVisibility::Collapsed;
}

/** Whether to show table row options */
EVisibility SBBBCSVImportOptions::GetCurveTypeVis() const
{
	return (ImportTypeCombo.IsValid() && *ImportTypeCombo->GetSelectedItem() == EBBBCSVImportType::ECSV_CurveTable) ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SBBBCSVImportOptions::GetDetailsPanelVis() const
{
	return (ImportTypeCombo.IsValid() && *ImportTypeCombo->GetSelectedItem() == EBBBCSVImportType::ECSV_DataTable) ? EVisibility::Visible : EVisibility::Hidden;
}

FString SBBBCSVImportOptions::GetImportTypeText(TSharedPtr<EBBBCSVImportType> Type) const
{
	FString EnumString;
	if (*Type == EBBBCSVImportType::ECSV_DataTable)
	{
		EnumString = TEXT("DataTable");
	}
	else if (*Type == EBBBCSVImportType::ECSV_CurveTable)
	{
		EnumString = TEXT("CurveTable");
	}
	else if (*Type == EBBBCSVImportType::ECSV_CurveFloat)
	{
		EnumString = TEXT("Float Curve");
	}
	else if (*Type == EBBBCSVImportType::ECSV_CurveVector)
	{
		EnumString = TEXT("Vector Curve");
	}
	return EnumString;
}

/** Called to create a widget for each struct */
TSharedRef<SWidget> SBBBCSVImportOptions::MakeImportTypeItemWidget(TSharedPtr<EBBBCSVImportType> Type)
{
	return	SNew(STextBlock)
			.Text(FText::FromString(GetImportTypeText(Type)));
}

void SBBBCSVImportOptions::OnImportTypeSelected(TSharedPtr<EBBBCSVImportType> Selection, ESelectInfo::Type SelectionType)
{
	if (Selection.IsValid() && *Selection == EBBBCSVImportType::ECSV_DataTable)
	{
		PropertyView->SetObject(TempImportDataTable.Get());
	}
	else
	{
		PropertyView->SetObject(nullptr);
	}
}

void SBBBCSVImportOptions::OnStructSelected(UScriptStruct* NewStruct)
{
	SelectedStruct = NewStruct;
}

FString SBBBCSVImportOptions::GetCurveTypeText(CurveInterpModePtr InterpMode) const
{
	FString EnumString;

	switch(*InterpMode)
	{
		case ERichCurveInterpMode::RCIM_Constant : 
			EnumString = TEXT("Constant");
			break;

		case ERichCurveInterpMode::RCIM_Linear : 
			EnumString = TEXT("Linear");
			break;

		case ERichCurveInterpMode::RCIM_Cubic : 
			EnumString = TEXT("Cubic");
			break;
	}
	return EnumString;
}

/** Called to create a widget for each curve interpolation enum */
TSharedRef<SWidget> SBBBCSVImportOptions::MakeCurveTypeWidget(CurveInterpModePtr InterpMode)
{
	FString Label = GetCurveTypeText(InterpMode);
	return SNew(STextBlock) .Text( FText::FromString(Label) );
}

/** Called when 'OK' button is pressed */
FReply SBBBCSVImportOptions::OnImport()
{
	SelectedImportType = static_cast<uint8>(*ImportTypeCombo->GetSelectedItem());
	if (CurveInterpCombo->GetSelectedItem().IsValid())
	{
		SelectedCurveInterpMode = *CurveInterpCombo->GetSelectedItem();
	}
	bImport = true;
	if (WidgetWindow.IsValid())
	{
		WidgetWindow.Pin()->RequestDestroyWindow();
	}
	return FReply::Handled();
}

bool SBBBCSVImportOptions::CanImport() const
{
	EBBBCSVImportType ImportType = *ImportTypeCombo->GetSelectedItem();

	switch (ImportType)
	{
	case EBBBCSVImportType::ECSV_DataTable:
		return SelectedStruct != nullptr;
		break;
	case EBBBCSVImportType::ECSV_CurveTable:
		return CurveInterpCombo->GetSelectedItem().IsValid();
		break;
	case EBBBCSVImportType::ECSV_CurveFloat:
	case EBBBCSVImportType::ECSV_CurveVector:
	case EBBBCSVImportType::ECSV_CurveLinearColor:
		return true;
	}
	
	return false;
}

/** Called when 'Cancel' button is pressed */
FReply SBBBCSVImportOptions::OnCancel()
{
	bImport = false;
	if (WidgetWindow.IsValid())
	{
		WidgetWindow.Pin()->RequestDestroyWindow();
	}
	return FReply::Handled();
}

FText SBBBCSVImportOptions::GetSelectedItemText() const
{
	TSharedPtr<EBBBCSVImportType> SelectedType = ImportTypeCombo->GetSelectedItem();

	return (SelectedType.IsValid())
		? FText::FromString(GetImportTypeText(SelectedType))
		: FText::GetEmpty();
}

FText SBBBCSVImportOptions::GetSelectedCurveTypeText() const
{
	CurveInterpModePtr CurveModePtr = CurveInterpCombo->GetSelectedItem();
	return (CurveModePtr.IsValid())
		? FText::FromString(GetCurveTypeText(CurveModePtr))
		: FText::GetEmpty();
}

#undef LOCTEXT_NAMESPACE
