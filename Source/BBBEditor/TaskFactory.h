#pragma once

#include "UnrealEd.h"
#include "ClassViewerModule.h"
#include "ClassViewerFilter.h"
#include "Factories/Factory.h"
#include "TaskFactory.generated.h"

/**
*
*/
UCLASS()
class BBBEDITOR_API UTaskFactory : public UFactory
{
    GENERATED_BODY()

public:
    UTaskFactory(const FObjectInitializer& ObjectInitializer);

private:
    UPROPERTY(EditAnywhere, Category = Task)
    TSubclassOf<class UBaseTask> TaskClass;
    
    virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
    virtual bool ConfigureProperties() override;

};

class FTaskFilterViewer : public IClassViewerFilter
{
public:
    TSet<const UClass*> AllowedChildrenOfClasses;
    EClassFlags DisallowedClassFlags;

    virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef<class FClassViewerFilterFuncs> InFilterFuncs) override
    {
        return !InClass->HasAnyClassFlags(DisallowedClassFlags)
           && InFilterFuncs->IfInChildOfClassesSet(AllowedChildrenOfClasses, InClass) != EFilterReturn::Failed;
    }

    virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef<const IUnloadedBlueprintData> InUnloadedClassData, TSharedRef<FClassViewerFilterFuncs> InFilterFuncs) override
    {
        return !InUnloadedClassData->HasAnyClassFlags(DisallowedClassFlags)
            && InFilterFuncs->IfInChildOfClassesSet(AllowedChildrenOfClasses, InUnloadedClassData) != EFilterReturn::Failed;
    }
};