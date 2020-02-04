// Fill out your copyright notice in the Description page of Project Settings.


#include "TaskFactory.h"
#include "Kismet2/SClassPickerDialog.h"
#include "Tasks/Task.h"

#define LOCTEXT_NAMESPACE "Task"

UTaskFactory::UTaskFactory(const FObjectInitializer& ObjectInitializer) 
    : Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UTask::StaticClass();
}

UObject* UTaskFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
    if (TaskClass != nullptr)
    {
        return NewObject<UTask>(InParent, TaskClass, Name, Flags | RF_Transactional);
    }
    else
    {
        check(Class->IsChildOf(UTask::StaticClass()));
        return NewObject<UTask>(InParent, TaskClass, Name, Flags | RF_Transactional);
    }
}

bool UTaskFactory::ConfigureProperties()
{
    TaskClass = nullptr;

    FClassViewerModule& ClassViewerModule = FModuleManager::LoadModuleChecked<FClassViewerModule>("ClassViewer");
    FClassViewerInitializationOptions Options;
    Options.Mode = EClassViewerMode::ClassPicker;

    TSharedPtr<FTaskFilterViewer> Filter = MakeShareable<FTaskFilterViewer>(new FTaskFilterViewer);
    Options.ClassFilter = Filter;

    Filter->DisallowedClassFlags = CLASS_Abstract | CLASS_Deprecated;
    Filter->AllowedChildrenOfClasses.Add(UTask::StaticClass());

    const FText TitleText = LOCTEXT("CreateTaskOptions", "Pick Task Type");
    UClass* ChosenClass = nullptr;
    const bool bPressedOk = SClassPickerDialog::PickClass(TitleText, Options, ChosenClass, UTask::StaticClass());

    if (bPressedOk)
    {
        TaskClass = ChosenClass;
    }

    return bPressedOk;
}
