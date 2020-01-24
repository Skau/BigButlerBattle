// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UnrealEd.h"
#include "Factories/CSVImportFactory.h"
#include "BBBCSVImportFactory.generated.h"

/**
 * 
 */
UCLASS()
class BBBEDITOR_API UBBBCSVImportFactory : public UCSVImportFactory
{
	GENERATED_BODY()

public:
	UBBBCSVImportFactory(const FObjectInitializer& ObjectInitializer);

private:
	//~ Begin UFactory Interface
	UObject* FactoryCreateText(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const TCHAR*& Buffer, const TCHAR* BufferEnd, FFeedbackContext* Warn, bool& bOutOperationCanceled) override;
	//~ End UFactory Interface
};
