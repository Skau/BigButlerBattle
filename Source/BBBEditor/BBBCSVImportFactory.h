// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UnrealEd.h"
#include "Factories/CSVImportFactory.h"
#include "BBBCSVImportFactory.generated.h"

/** Enum to indicate what to import CSV as */
UENUM(BlueprintType)
enum class EBBBCSVImportType : uint8
{
	/** Import as UDataTable */
	ECSV_DataTable,
	/** Import as UCurveTable */
	ECSV_CurveTable,
	/** Import as a UCurveFloat */
	ECSV_CurveFloat,
	/** Import as a UCurveVector */
	ECSV_CurveVector,
	/** Import as a UCurveLinearColor */
	ECSV_CurveLinearColor,
	/** Import as a UBezierDataTable */
	ECSV_BezierDataTable,
};

USTRUCT(BlueprintType)
struct FBBBCSVImportSettings
{
	GENERATED_BODY()

	FBBBCSVImportSettings();

	UPROPERTY(BlueprintReadWrite, Category="Misc")
	UScriptStruct* ImportRowStruct;

	UPROPERTY(BlueprintReadWrite, Category="Misc")
	EBBBCSVImportType ImportType;

	UPROPERTY(BlueprintReadWrite, Category="Misc")
	TEnumAsByte<ERichCurveInterpMode> ImportCurveInterpMode;
};

/**
 * 
 */
UCLASS()
class BBBEDITOR_API UBBBCSVImportFactory : public UCSVImportFactory
{
	GENERATED_BODY()

public:
	UBBBCSVImportFactory(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintReadWrite, Category="Automation")
	FBBBCSVImportSettings CustomAutomatedImportSettings;

private:
	//~ Begin UFactory Interface
	UObject* FactoryCreateText(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const TCHAR*& Buffer, const TCHAR* BufferEnd, FFeedbackContext* Warn, bool& bOutOperationCanceled) override;
	//~ End UFactory Interface
};
