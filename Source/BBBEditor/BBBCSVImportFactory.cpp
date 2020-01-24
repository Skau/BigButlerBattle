// Fill out your copyright notice in the Description page of Project Settings.


#include "BBBCSVImportFactory.h"
#include "BBBCSVImportOptions.h"

#include "Interfaces/IMainFrameModule.h"

DEFINE_LOG_CATEGORY(LogCSVImportFactory);

#define LOCTEXT_NAMESPACE "BBBCSVImportFactory"

UBBBCSVImportFactory::UBBBCSVImportFactory(const FObjectInitializer& ObjectInitializer)
    : Super{ObjectInitializer}
{
    ImportPriority = 101;
}

FBBBCSVImportSettings::FBBBCSVImportSettings()
{
	ImportRowStruct = nullptr;
	ImportType = EBBBCSVImportType::ECSV_DataTable;
	ImportCurveInterpMode = ERichCurveInterpMode::RCIM_Linear;
}

static UClass* GetCurveClass( EBBBCSVImportType ImportType )
{
	switch( ImportType )
	{
	case EBBBCSVImportType::ECSV_CurveFloat:
		return UCurveFloat::StaticClass();
		break;
	case EBBBCSVImportType::ECSV_CurveVector:
		return UCurveVector::StaticClass();
		break;
	case EBBBCSVImportType::ECSV_CurveLinearColor:
		return UCurveLinearColor::StaticClass();
		break;
	default:
		return UCurveVector::StaticClass();
		break;
	}
}

UObject* UBBBCSVImportFactory::FactoryCreateText(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const TCHAR*& Buffer, const TCHAR* BufferEnd, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
    UE_LOG(LogTemp, Warning, TEXT("Custom CSV importer run!: FactoryCreateText"));
    
    GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPreImport(this, InClass, InParent, InName, Type);

	bOutOperationCanceled = false;

	// See if table/curve already exists
	UDataTable* ExistingTable = FindObject<UDataTable>(InParent, *InName.ToString());
	UCurveTable* ExistingCurveTable = FindObject<UCurveTable>(InParent, *InName.ToString());
	UCurveBase* ExistingCurve = FindObject<UCurveBase>(InParent, *InName.ToString());

	// Save off information if so
	bool bHaveInfo = false;
	ERichCurveInterpMode ImportCurveInterpMode = RCIM_Linear;
	EBBBCSVImportType ImportType = EBBBCSVImportType::ECSV_DataTable;
	UClass* DataTableClass = UDataTable::StaticClass();

	// Clear our temp table
	TempImportDataTable = nullptr;

	if (IsAutomatedImport())
	{
		ImportCurveInterpMode = AutomatedImportSettings.ImportCurveInterpMode;
		ImportType = static_cast<EBBBCSVImportType>(AutomatedImportSettings.ImportType);

		TempImportDataTable = NewObject<UDataTable>(this, UDataTable::StaticClass(), InName, Flags);
		TempImportDataTable->RowStruct = AutomatedImportSettings.ImportRowStruct;

		// For automated import to work a row struct must be specified for a datatable type or a curve type must be specified
		bHaveInfo = TempImportDataTable->RowStruct != nullptr || ImportType != EBBBCSVImportType::ECSV_DataTable;
	}
	else if (ExistingTable != nullptr)
	{
		ImportType = EBBBCSVImportType::ECSV_DataTable;
		TempImportDataTable = NewObject<UDataTable>(this, ExistingTable->GetClass(), InName, Flags);
		TempImportDataTable->CopyImportOptions(ExistingTable);
		
		bHaveInfo = true;
	}
	else if (ExistingCurveTable != nullptr)
	{
		ImportType = EBBBCSVImportType::ECSV_CurveTable;
		bHaveInfo = true;
	}
	else if (ExistingCurve != nullptr)
	{
		ImportType = ExistingCurve->IsA(UCurveFloat::StaticClass()) ? EBBBCSVImportType::ECSV_CurveFloat : EBBBCSVImportType::ECSV_CurveVector;
		bHaveInfo = true;
	}

	bool bDoImport = true;

	if (!TempImportDataTable)
	{
		// Create an empty one
		TempImportDataTable = NewObject<UDataTable>(this, UDataTable::StaticClass(), InName, Flags);
	}

	// If we do not have the info we need, pop up window to ask for things
	if (!bHaveInfo && !IsAutomatedImport())
	{
		TSharedPtr<SWindow> ParentWindow;
		// Check if the main frame is loaded.  When using the old main frame it may not be.
		if (FModuleManager::Get().IsModuleLoaded( "MainFrame" ))
		{
			IMainFrameModule& MainFrame = FModuleManager::LoadModuleChecked<IMainFrameModule>( "MainFrame" );
			ParentWindow = MainFrame.GetParentWindow();
		}

		TSharedPtr<SBBBCSVImportOptions> ImportOptionsWindow;

		TSharedRef<SWindow> Window = SNew(SWindow)
			.Title( LOCTEXT("DataTableOptionsWindowTitle", "DataTable Options" ))
			.SizingRule( ESizingRule::Autosized );
		
		FString ParentFullPath;

		if (InParent)
		{
			ParentFullPath = InParent->GetPathName();
		}

		Window->SetContent
		(
			SAssignNew(ImportOptionsWindow, SBBBCSVImportOptions)
				.WidgetWindow(Window)
				.FullPath(FText::FromString(ParentFullPath))
				.TempImportDataTable(TempImportDataTable)
		);

		FSlateApplication::Get().AddModalWindow(Window, ParentWindow, false);

		ImportType = static_cast<EBBBCSVImportType>(ImportOptionsWindow->GetSelectedImportType());
		TempImportDataTable->RowStruct = ImportOptionsWindow->GetSelectedRowStruct();
		ImportCurveInterpMode = ImportOptionsWindow->GetSelectedCurveIterpMode();
		bDoImport = ImportOptionsWindow->ShouldImport();
		bOutOperationCanceled = !bDoImport;
	}
	else if (!bHaveInfo && IsAutomatedImport())
	{
		if (ImportType == EBBBCSVImportType::ECSV_DataTable && !TempImportDataTable->RowStruct)
		{
			UE_LOG(LogCSVImportFactory, Error, TEXT("A Data table row type must be specified in the import settings json file for automated import"));
		}
		bDoImport = false;
	}

	UObject* NewAsset = nullptr;
	if (bDoImport)
	{
		// Convert buffer to an FString (will this be slow with big tables?)
		FString String;
		//const int32 BufferSize = BufferEnd - Buffer;
		//appBufferToString( String, Buffer, BufferSize );
		int32 NumChars = (BufferEnd - Buffer);
		TArray<TCHAR>& StringChars = String.GetCharArray();
		StringChars.AddUninitialized(NumChars+1);
		FMemory::Memcpy(StringChars.GetData(), Buffer, NumChars*sizeof(TCHAR));
		StringChars.Last() = 0;

		TArray<FString> Problems;

		if (ImportType == EBBBCSVImportType::ECSV_DataTable)
		{
			// If there is an existing table, need to call this to free data memory before recreating object
			UDataTable::FOnDataTableChanged OldOnDataTableChanged;
			if (ExistingTable != nullptr)
			{
				OldOnDataTableChanged = MoveTemp(ExistingTable->OnDataTableChanged());
				ExistingTable->OnDataTableChanged().Clear();
				DataTableClass = ExistingTable->GetClass();
				ExistingTable->EmptyTable();
			}

			// Create/reset table
			UDataTable* NewTable = NewObject<UDataTable>(InParent, DataTableClass, InName, Flags);
			
			NewTable->CopyImportOptions(TempImportDataTable);
			NewTable->AssetImportData->Update(CurrentFilename);

			// Go ahead and create table from string
			Problems = DoImportDataTable(NewTable, String);

			// hook delegates back up and inform listeners of changes
			NewTable->OnDataTableChanged() = MoveTemp(OldOnDataTableChanged);
			NewTable->OnDataTableChanged().Broadcast();

			// Print out
			UE_LOG(LogCSVImportFactory, Log, TEXT("Imported DataTable '%s' - %d Problems"), *InName.ToString(), Problems.Num());
			NewAsset = NewTable;
		}
		else if (ImportType == EBBBCSVImportType::ECSV_CurveTable)
		{
			UClass* CurveTableClass = UCurveTable::StaticClass();

			// If there is an existing table, need to call this to free data memory before recreating object
			UCurveTable::FOnCurveTableChanged OldOnCurveTableChanged;
			if (ExistingCurveTable != nullptr)
			{
				OldOnCurveTableChanged = MoveTemp(ExistingCurveTable->OnCurveTableChanged());
				ExistingCurveTable->OnCurveTableChanged().Clear();
				CurveTableClass = ExistingCurveTable->GetClass();
				ExistingCurveTable->EmptyTable();
			}

			// Create/reset table
			UCurveTable* NewTable = NewObject<UCurveTable>(InParent, CurveTableClass, InName, Flags);
			NewTable->AssetImportData->Update(CurrentFilename);

			// Go ahead and create table from string
			Problems = DoImportCurveTable(NewTable, String, ImportCurveInterpMode);

			// hook delegates back up and inform listeners of changes
			NewTable->OnCurveTableChanged() = MoveTemp(OldOnCurveTableChanged);
			NewTable->OnCurveTableChanged().Broadcast();

			// Print out
			UE_LOG(LogCSVImportFactory, Log, TEXT("Imported CurveTable '%s' - %d Problems"), *InName.ToString(), Problems.Num());
			NewAsset = NewTable;
		}
		else if (ImportType == EBBBCSVImportType::ECSV_CurveFloat || ImportType == EBBBCSVImportType::ECSV_CurveVector || ImportType == EBBBCSVImportType::ECSV_CurveLinearColor)
		{
			UClass* CurveClass = ExistingCurve ? ExistingCurve->GetClass() : GetCurveClass( ImportType );

			// Create/reset curve
			UCurveBase* NewCurve = NewObject<UCurveBase>(InParent, CurveClass, InName, Flags);

			Problems = DoImportCurve(NewCurve, String);

			UE_LOG(LogCSVImportFactory, Log, TEXT("Imported Curve '%s' - %d Problems"), *InName.ToString(), Problems.Num());
			NewCurve->AssetImportData->Update(CurrentFilename);
			NewAsset = NewCurve;
		}
		else if (ImportType == EBBBCSVImportType::ECSV_BezierDataTable)
		{
			// Something here
		}
		
		if (Problems.Num() > 0)
		{
			FString AllProblems;

			for (int32 ProbIdx=0; ProbIdx<Problems.Num(); ProbIdx++)
			{
				// Output problems to log
				UE_LOG(LogCSVImportFactory, Log, TEXT("%d:%s"), ProbIdx, *Problems[ProbIdx]);
				AllProblems += Problems[ProbIdx];
				AllProblems += TEXT("\n");
			}

			if (!IsAutomatedImport())
			{
				// Pop up any problems for user
				FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(AllProblems));
			}
		}
	}

	GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPostImport(this, NewAsset);

	return NewAsset;
}