// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.


#include "BigButlerBattleGameModeBase.h"
#include "ButlerGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "PlayerCharacterController.h"
#include "PlayerCharacter.h"
#include "PauseWidget.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"
#include "TaskObject.h"
#include "EngineUtils.h"
#include "Math/RandomStream.h"
#include "BaseTask.h"

float ABigButlerBattleGameModeBase::GetAngleBetween(FVector Vector1, FVector Vector2)
{
	return FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Vector1, Vector2) / (Vector1.Size() * Vector2.Size())));
}

float ABigButlerBattleGameModeBase::GetAngleBetweenNormals(FVector Normal1, FVector Normal2)
{
	return FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Normal1, Normal2)));
}

void ABigButlerBattleGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	// Spawn and possess
	const bool bIDsIsEmpty = GetNumPlayers() == 0;
	int PauseWidgetOwner = 0;
	if (!bIDsIsEmpty)
	{
		TArray<AActor*> Actors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerCharacterController::StaticClass(), Actors);
		for (auto& Actor : Actors)
		{
			auto controller = Cast<APlayerCharacterController>(Actor);
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			auto Character = GetWorld()->SpawnActor<APlayerCharacter>(PlayerCharacterClass, Params);
			controller->Possess(Character);
			controller->PauseGame.BindUObject(this, &ABigButlerBattleGameModeBase::OnPlayerPaused);
			controller->SetInputMode(FInputModeGameOnly());
			PauseWidgetOwner = UGameplayStatics::GetPlayerControllerID(controller);
		}
	}
	// For editor testing
	else
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		auto Character = GetWorld()->SpawnActor<APlayerCharacter>(PlayerCharacterClass, Params);
		auto PC = Cast<APlayerCharacterController>(UGameplayStatics::GetPlayerControllerFromID(GetWorld(), 0)); 
		PC->Possess(Character);
		PC->SetInputMode(FInputModeGameOnly());
		PC->PauseGame.BindUObject(this, &ABigButlerBattleGameModeBase::OnPlayerPaused);
	}

	if (!PauseWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("Pause Widget Class not setup in Gamemode!"));
		return;
	}

	PauseWidget = CreateWidget<UPauseWidget>(UGameplayStatics::GetPlayerControllerFromID(GetWorld(), PauseWidgetOwner), PauseWidgetClass);
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

void ABigButlerBattleGameModeBase::OnPlayerPaused(APlayerCharacterController* Controller)
{
	if (!PauseWidget->IsVisible())
	{
		PauseWidget->SetVisibility(ESlateVisibility::Visible);
		PauseWidget->FocusWidget(Controller);
		UGameplayStatics::SetGamePaused(GetWorld(), true);
	}
	else
	{
		OnPlayerContinued(Controller);
	}
}

void ABigButlerBattleGameModeBase::OnPlayerContinued(APlayerCharacterController* Controller)
{
	if (PauseWidget->IsVisible())
	{
		PauseWidget->SetVisibility(ESlateVisibility::Hidden);
		Controller->SetInputMode(FInputModeGameOnly());
		UGameplayStatics::SetGamePaused(GetWorld(), false);
	}
}

void ABigButlerBattleGameModeBase::OnPlayerQuit()
{
	UGameplayStatics::OpenLevel(GetWorld(), "MainMenu");
}

void ABigButlerBattleGameModeBase::GenerateTasks()
{
	if (Tasks.Num())
	{
		Tasks.Empty();
	}

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
			// [] operator on TMaps don't automatically add, so we do it manually
			if (!WorldTaskData.Find(Task->Type))
				WorldTaskData.Add(Task->Type, TArray<UBaseTask*>());

			// Add the task to the types TArray of tasks
			WorldTaskData[Task->Type].Add(Task);	
		}
	}

	// If there are no tasks, just return
	if (!WorldTaskData.Num())
		return;

	Tasks.Reserve(TotalTasks);

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

	// Remove the custom ranges we don't care about (none in the world)
	for (auto& Type : Types)
	{
		bool Found = false;

		for (auto& Range : Ranges)
			Found = true;

		if (!Found)
			Ranges.Remove(Type);
	}


	// Used for the outside while loop to keep track of when reach max possible tasks
	int Remaining = TotalTasks - Tasks.Num();

	// Start index so it's random
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

			// No more tasks or the max number of tasks of the type lef to add is zero - check next type
			if (TasksAvailable == 0 || Ranges[Type].Max == 0)
				continue;

			// The max number of possible tasks to get is the lowest between how many there are available and the custom max range
			int MaxNumberOfPossibleTasksToGet = FMath::Min(TasksAvailable, Ranges[Type].Max);

			// This is the new max number of tasks for this type
			Ranges[Type].Max = MaxNumberOfPossibleTasksToGet;

			// No more available to add - check next type
			if (Ranges[Type].Max == 0)
				continue;

			// The random part, how many to actually get is a random value between the lowest of available tasks and custom min range and the calculated max
			int TasksToAdd = Stream.RandRange(FMath::Min(TasksAvailable, Ranges[Type].Min), MaxNumberOfPossibleTasksToGet);

			// Add the tasks - we can loop from 0 because we shuffled the tasks earlier
			for (int i = 0; i < TasksToAdd; ++i)
			{
				Tasks.Add(TaskData[i]);

				// Quick check to make sure we don't go over max total tasks
				Remaining = TotalTasks - Tasks.Num();
				if (Remaining <= 0)
					break;
			}

			// Remove the tasks just added
			TaskData.RemoveAt(0, TasksToAdd);
		}

		// If all ranges max values are zero we are done
		bool Done = true;
		for (auto& Range : Ranges)
		{
			if (Range.Value.Max > 0)
			{
				Done = false;
				break;
			}
		}
		
		if (Done)
			break;
	}

	OnTasksGenerated.ExecuteIfBound(Tasks);
}
