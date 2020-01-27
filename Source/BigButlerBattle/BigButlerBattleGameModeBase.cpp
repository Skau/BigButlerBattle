// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.


#include "BigButlerBattleGameModeBase.h"
#include "ButlerGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Player/PlayerCharacterController.h"
#include "Player/PlayerCharacter.h"
#include "UI/PauseWidget.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"
#include "Tasks/TaskObject.h"
#include "EngineUtils.h"
#include "Math/RandomStream.h"
#include "Tasks/BaseTask.h"
#include "EngineUtils.h"
#include "King/King.h"

void ABigButlerBattleGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	// Get all controllers
	for (TActorIterator<APlayerCharacterController> iter(GetWorld()); iter; ++iter)
	{
		Controllers.Add(*iter);
	}

	// If in editor this can happen
	if (!Controllers.Num())
	{
		Controllers.Add(Cast<APlayerCharacterController>(UGameplayStatics::GetPlayerControllerFromID(GetWorld(), 0)));
	}
	
	for (auto& Controller : Controllers)
	{
		// Spawn character and posess

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		auto Character = GetWorld()->SpawnActor<APlayerCharacter>(PlayerCharacterClass, Params);
		Controller->Possess(Character);

		Controller->SetInputMode(FInputModeGameOnly());

		// Delegates

		Controller->OnPausedGame.BindUObject(this, &ABigButlerBattleGameModeBase::OnPlayerPaused);
		Controller->OnGameFinished.BindUObject(this, &ABigButlerBattleGameModeBase::OnGameFinished);
	}

	// Pause widget setup

	if (!PauseWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("Pause Widget Class not setup in Gamemode!"));
		return;
	}

	PauseWidget = CreateWidget<UPauseWidget>(Controllers[0], PauseWidgetClass);
	PauseWidget->AddToViewport();

	PauseWidget->ContinueGame.BindUObject(this, &ABigButlerBattleGameModeBase::OnPlayerContinued);
	PauseWidget->QuitGame.BindUObject(this, &ABigButlerBattleGameModeBase::OnPlayerQuit);

	// Wait a bit for task objects to finish

	FTimerDelegate TimerCallback;
	TimerCallback.BindLambda([&]
	{
		GenerateTasks();
	});
	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(Handle, TimerCallback, 0.1f, false);
}

void ABigButlerBattleGameModeBase::OnPlayerPaused(int ID)
{
	if (!PauseWidget->IsVisible())
	{
		PauseWidget->SetVisibility(ESlateVisibility::Visible);
		auto Controller = Cast<APlayerCharacterController>(UGameplayStatics::GetPlayerControllerFromID(GetWorld(), ID));
		PauseWidget->FocusWidget(Controller);
		UGameplayStatics::SetGamePaused(GetWorld(), true);
	}
	else
	{
		OnPlayerContinued(ID);
	}
}

void ABigButlerBattleGameModeBase::OnPlayerContinued(int ID)
{
	auto Controller = Cast<APlayerCharacterController>(UGameplayStatics::GetPlayerControllerFromID(GetWorld(), ID));

	if (PauseWidget->IsVisible())
	{
		PauseWidget->SetVisibility(ESlateVisibility::Hidden);
		Controller->SetInputMode(FInputModeGameOnly());
		UGameplayStatics::SetGamePaused(GetWorld(), false);
	}
}

void ABigButlerBattleGameModeBase::OnGameFinished(int ID)
{
	UE_LOG(LogTemp, Warning, TEXT("Player %i won!"), ID);
}

void ABigButlerBattleGameModeBase::OnPlayerQuit()
{
	UGameplayStatics::OpenLevel(GetWorld(), "MainMenu");
}

void ABigButlerBattleGameModeBase::GenerateTasks()
{
	TArray<UBaseTask*> Tasks;

	TMap<EObjectType, FIntRange> Ranges
	{
		{EObjectType::Wine, WineRange},
		{EObjectType::Food, FoodRange}
	};

	TMap<EObjectType, TArray<UBaseTask*>> WorldTaskData;

	// Get all actors that are task objects
	for (TActorIterator<ATaskObject> itr(GetWorld()); itr; ++itr)
	{
		if (!itr)
			continue;

		// Get the task
		auto Task = itr->GetTaskData();

		// If the task is legit
		if (Task && Task->Type != EObjectType::None)
		{
			// [] operator on TMaps doesn't automatically add, so we do it manually
			if (!WorldTaskData.Find(Task->Type))
				WorldTaskData.Add(Task->Type, TArray<UBaseTask*>());

			// Add the task to the types TArray of tasks
			WorldTaskData[Task->Type].Add(Task);
		}
	}

	// If there are no tasks, just return
	if (!WorldTaskData.Num())
		return;

	// Setup random generator and get seed from game instance

	auto Instance = Cast<UButlerGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	FRandomStream Stream;
	Stream.Initialize(Instance->GetCurrentRandomSeed());

	// Shuffle the arrays

	for (auto& TaskData : WorldTaskData)
	{
		auto TaskArray = TaskData.Value;
		if (!TaskArray.Num())
			continue;

		int LastIndex = TaskArray.Num() - 1;
		for (int i = 0; i < LastIndex; ++i)
		{
			int Index = Stream.RandRange(0, LastIndex);
			if (i != Index)
			{
				TaskArray.Swap(i, Index);
			}
		}
	}

	// Get all types
	TArray<EObjectType> Types;
	WorldTaskData.GetKeys(Types);
	if (!Types.Num())
		return;

	// Remove Ranges not needed (no tasks of type in world)
	TArray<EObjectType> TypesToRemove;
	for (auto& Range : Ranges)
	{
		if (!Types.FindByKey(Range.Key))
		{
			TypesToRemove.Add(Range.Key);
		}
	}
	for (auto& Type : TypesToRemove)
	{
		Ranges.Remove(Type);
	}

	// Used for the outside while loop to keep track of when max possible tasks are reached
	int Remaining = TotalTasks;

	// Random start index
	int Index = Stream.RandRange(0, Types.Num() - 1);

	// Loop until there are no more tasks to add
	while (Remaining > 0)
	{
		// Loop through all task types, with an extra break on remaining == 0
		// Index will loop back to 0 if reach end of types
		for (int Iterations = 0; Iterations < WorldTaskData.Num() && Remaining > 0; ++Iterations, Index = ++Index % WorldTaskData.Num())
		{
			// Get the type
			auto Type = Types[Index];

			// Get the tasks available
			auto& TaskData = WorldTaskData[Type];

			// Get the number of tasks available
			int TasksAvailable = TaskData.Num();

			// Calculate new min number of tasks for this type
			Ranges[Type].Min = FMath::Clamp(FMath::Min(TasksAvailable, Ranges[Type].Min), 0, TotalTasks);

			// Calculate new max number of tasks for this type
			Ranges[Type].Max = FMath::Clamp(FMath::Min(TasksAvailable, Ranges[Type].Max), Ranges[Type].Min, TotalTasks);

			if (Ranges[Type].Max == 0)
				continue;

			// How many to actually get is a random value between the lowest and highest possible tasks
			int TasksToAdd = Stream.RandRange(Ranges[Type].Min, Ranges[Type].Max);

			// If no tasks to add, just continue
			if (TasksToAdd == 0)
				continue;

			// Add the tasks - we can loop from 0 because we shuffled the tasks earlier
			for (int i = 0; i < TasksToAdd; ++i)
			{
				Tasks.Add(TaskData[i]);
			}

			Ranges[Type].Max -= TasksToAdd;

			// Remove the tasks just added
			TaskData.RemoveAt(0, TasksToAdd);
		}

		// If all ranges max values are zero we are done

		int Sum = 0;
		for (auto& Range : Ranges)
			Sum += Range.Value.Max;

		if (Sum == 0)
			break;
	}

	UE_LOG(LogTemp, Warning, TEXT("Total tasks: %i"), Tasks.Num());
	if (Tasks.Num() > 6)
	{
		UE_LOG(LogTemp, Error, TEXT("FIX THIS: Shaving off excess tasks."))
		Tasks.RemoveAt(0, Tasks.Num() - 6);
	}

	// Setup player tasks and give them to the controllers
	for (auto& Controller : Controllers)
	{
		auto ID = UGameplayStatics::GetPlayerControllerID(Controller);
		TArray<TPair<UBaseTask*, ETaskState>> PlayerTasks;
		for (auto& Task : Tasks)
		{
			PlayerTasks.Add(TPair<UBaseTask*, ETaskState>(Task, ETaskState::NotPresent));
		}
		Controller->SetPlayerTasks(PlayerTasks);
	}
}
