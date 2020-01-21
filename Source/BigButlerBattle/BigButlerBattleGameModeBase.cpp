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

	const TMap<EObjectType, FIntRange> Ranges
	{
		{EObjectType::Wine, WineRange},
		{EObjectType::Food, FoodRange}
	};

	Tasks.Reserve(TotalTasks);

	TMap<EObjectType, TArray<FTask>> WorldTaskData;

	// Get all actors that are task objects
	for (TActorIterator<ATaskObject> itr(GetWorld()); itr; ++itr)
	{
		if (itr->GetObjectType() != EObjectType::None)
		{
			FTask Task(itr->GetObjectName(), *itr->GetObjectData());

			if (!WorldTaskData.Find(Task.Data.Type))
				WorldTaskData.Add(Task.Data.Type, TArray<FTask>());

			WorldTaskData[Task.Data.Type].Add(Task);
		}
	}

	// If there are no task objects, just return
	if (!WorldTaskData.Num())
		return;

	// Setup random generator

	auto Instance = Cast<UButlerGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	FRandomStream Stream;
	Stream.Initialize(Instance->GetCurrentRandomSeed());

	UE_LOG(LogTemp, Warning, TEXT("Seed: %i"), Stream.GetCurrentSeed());

	// Shuffle the arrays

	for (auto& TaskData : WorldTaskData)
	{
		auto TaskArray = TaskData.Value;
		if (TaskArray.Num())
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

	// Setup map to know how many there are left to add of each type
	TMap<EObjectType, int> RemainingCounts;
	TArray<EObjectType> Types;
	WorldTaskData.GetKeys(Types);

	if (!Types.Num())
		return;

	for (auto& Data : WorldTaskData)
	{
		RemainingCounts.Add(Data.Key, Data.Value.Num());
	}

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
			auto Type = Types[Index];
			auto& TaskData = WorldTaskData[Type];

			if (!TaskData.Num())
				continue;

			int MaxNumberOfPossibleTasksToGet = TaskData.Num();
			MaxNumberOfPossibleTasksToGet = FMath::Clamp(MaxNumberOfPossibleTasksToGet, 0, FMath::Min(Ranges[Type].Max, MaxNumberOfPossibleTasksToGet));
			
			if (MaxNumberOfPossibleTasksToGet == 0)
				continue;

			int ActualNumberOfTasksToGet = Stream.RandRange(Ranges[Type].Min, MaxNumberOfPossibleTasksToGet);

			for (int i = 0; i < ActualNumberOfTasksToGet; ++i)
			{
				auto Task = TaskData[i];
				Tasks.Add(Task);
				++i;

				Remaining = TotalTasks - Tasks.Num();
				if (Remaining <= 0)
					break;
			}
			TaskData.RemoveAt(0, ActualNumberOfTasksToGet);
			RemainingCounts[Type] = FMath::Max(0, TaskData.Num());
		}

		int Sum = 0;
		for (auto& Count : RemainingCounts)
			Sum += Count.Value;

		if (Sum <= 0)
			break;
	}

	OnTasksGenerated.ExecuteIfBound(Tasks);
}
