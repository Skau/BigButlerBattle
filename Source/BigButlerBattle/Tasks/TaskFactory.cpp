// Fill out your copyright notice in the Description page of Project Settings.


#include "TaskFactory.h"
#include "Kismet2/SClassPickerDialog.h"
#include "BaseTask.h"

#define LOCTEXT_NAMESPACE "Task"

UTaskFactory::UTaskFactory(const FObjectInitializer& ObjectInitializer) 
    : Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UBaseTask::StaticClass();
}

UObject* UTaskFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
    if (TaskClass != nullptr)
    {
        return NewObject<UBaseTask>(InParent, TaskClass, Name, Flags | RF_Transactional);
    }
    else
    {
        check(Class->IsChildOf(UBaseTask::StaticClass()));
        return NewObject<UBaseTask>(InParent, TaskClass, Name, Flags | RF_Transactional);
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
    Filter->AllowedChildrenOfClasses.Add(UBaseTask::StaticClass());

    const FText TitleText = LOCTEXT("CreateTaskOptions", "Pick Task Type");
    UClass* ChosenClass = nullptr;
    const bool bPressedOk = SClassPickerDialog::PickClass(TitleText, Options, ChosenClass, UBaseTask::StaticClass());

    if (bPressedOk)
    {
        TaskClass = ChosenClass;
    }

    return bPressedOk;
}
