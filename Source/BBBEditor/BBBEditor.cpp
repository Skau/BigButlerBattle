#include "BBBEditor.h"
#include "Modules/ModuleManager.h"
#include "Modules/ModuleInterface.h"

IMPLEMENT_GAME_MODULE(FBBBEditorModule, BBBEditor);

#define LOCTEXT_NAMESPACE "TutorialEditor"

void FBBBEditorModule::StartupModule()
{
    // UE_LOG(LogTemp, Warning, TEXT("BBBEditor: Log Started"));
}

void FBBBEditorModule::ShutdownModule()
{
    // UE_LOG(LogTemp, Warning, TEXT("BBBEditor: Log Ended"));
}

#undef LOCTEXT_NAMESPACE