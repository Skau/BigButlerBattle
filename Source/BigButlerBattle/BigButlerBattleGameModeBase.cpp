// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.


#include "BigButlerBattleGameModeBase.h"
#include "ButlerGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Player/PlayerCharacterController.h"
#include "UI/PauseWidget.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"
#include "Tasks/TaskObject.h"
#include "EngineUtils.h"
#include "Math/RandomStream.h"
#include "Tasks/Task.h"
#include "Utils/btd.h"
#include "UI/GameFinishedWidget.h"
#include "Utils/Spawnpoint.h"
#include "UI/GameWidget.h"
#include "King/King.h"


ABigButlerBattleGameModeBase::ABigButlerBattleGameModeBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABigButlerBattleGameModeBase::StartToLeaveMap()
{
	Super::StartToLeaveMap();

	// Clear timers
	for (TActorIterator<ATaskObject> Itr(GetWorld()); Itr; ++Itr)
	{
		if (!Itr)
			continue;
		GetWorld()->GetTimerManager().ClearAllTimersForObject(*Itr);
	}

	for (TActorIterator<APlayerController> Itr(GetWorld()); Itr; ++Itr)
	{
		if (!Itr)
			continue;
		GetWorld()->GetTimerManager().ClearAllTimersForObject(*Itr);
	}
}

void ABigButlerBattleGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	// Get all controllers
	for (TActorIterator<APlayerCharacterController> Iter(GetWorld()); Iter; ++Iter)
	{
		Controllers.Add(*Iter);
	}

	// If in editor this can happen
	if (!Controllers.Num())
	{
		Controllers.Add(Cast<APlayerCharacterController>(UGameplayStatics::GetPlayerControllerFromID(GetWorld(), 0)));
	}

	Controllers.Sort([](const APlayerCharacterController& C1, const APlayerCharacterController& C2)
	{
		return	  UGameplayStatics::GetPlayerControllerID(const_cast<APlayerCharacterController*>(&C1)) 
				< UGameplayStatics::GetPlayerControllerID(const_cast<APlayerCharacterController*>(&C2));
	});

	for (auto& Controller : Controllers)
	{
		UE_LOG(LogTemp, Warning, TEXT("Controller ID: %i"), UGameplayStatics::GetPlayerControllerID(Controller));
	}

	// Get player start locations
	TArray<ASpawnpoint*> PlayerStarts;
	for (TActorIterator<ASpawnpoint> Iter(GetWorld()); Iter; ++Iter)
	{
		if(Iter->bIsStartSpawn)
			PlayerStarts.Add(*Iter);
	}

	if(!PlayerStarts.Num())
		UE_LOG(LogTemp, Error, TEXT("No spawnpoints found"));


	if (!Controllers.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("No player controllers found"));
		return;
	}

	switch (Controllers.Num())
	{
	case 2:
		Controllers[0]->bUseCustomSpringArmLength = true;
		Controllers[1]->bUseCustomSpringArmLength = true;
		break;
	case 3:
		Controllers[0]->bUseCustomSpringArmLength = true;
		break;
	}

	for(int i = 0; i < Controllers.Num(); ++i)
	{
		// Spawn character
		Controllers[i]->RespawnCharacter(PlayerStarts[i]);
		Controllers[i]->SetInputMode(FInputModeGameOnly());

		// Delegates
		Controllers[i]->OnPausedGame.BindUObject(this, &ABigButlerBattleGameModeBase::OnPlayerPaused);
		Controllers[i]->OnGameFinished.BindUObject(this, &ABigButlerBattleGameModeBase::OnGameFinished);

		Controllers[i]->OnMainItemPickedUp.BindUObject(this, &ABigButlerBattleGameModeBase::OnMainItemPickedUp);
		Controllers[i]->OnMainItemDropped.BindUObject(this, &ABigButlerBattleGameModeBase::OnMainItemDropped);
	}

	// Pause widget setup

	if (!PauseWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("Pause Widget Class not setup in Gamemode!"));
	}
	else
	{
		PauseWidget = CreateWidget<UPauseWidget>(Controllers[0], PauseWidgetClass);
		PauseWidget->AddToViewport();

		PauseWidget->ContinueGame.BindUObject(this, &ABigButlerBattleGameModeBase::OnPlayerContinued);
		PauseWidget->QuitGame.BindUObject(this, &ABigButlerBattleGameModeBase::OnPlayerQuit);
	}

	// Game finished widget setup

	if (!GameFinishedWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("Game Finished Widget Class not setup in Gamemode!"));
	}
	else
	{
		GameFinishedWidget = CreateWidget<UGameFinishedWidget>(Controllers[0], GameFinishedWidgetClass);
		GameFinishedWidget->AddToViewport();

		GameFinishedWidget->QuitGame.BindUObject(this, &ABigButlerBattleGameModeBase::OnPlayerQuit);
	}

	if (!GameWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("Game Widget Class not setup in Gamemode!"));
	}
	else
	{
		GameWidget = CreateWidget<UGameWidget>(Controllers[0], GameWidgetClass);
		GameWidget->UpdateTimer(FString::FromInt(static_cast<int>(TotalSecondsToHold)));
		GameWidget->AddToViewport();
	}

	// Spawnpoints

	SetupSpawnpoints();


	King = Cast<AKing>(UGameplayStatics::GetActorOfClass(GetWorld(), AKing::StaticClass()));
	if (!IsValid(King))
	{
		UE_LOG(LogTemp, Error, TEXT("No King present in map!"));
	}

	// Wait a bit for task objects to finish

	//btd::Delay(this, 0.1f, [=]()
	//{
	//	TaskGenerationStartTime = FPlatformTime::Seconds();
	//	BeginTaskGeneration();
	//});
}


void ABigButlerBattleGameModeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bTimeDone && bItemCurrentlyBeingHeld)
	{
		TotalSecondsToHold -= DeltaTime;

		if (TotalSecondsToHold <= 0.f)
		{
			if (King)
			{
				King->UpdateCanReceiveMainItem();
				GameWidget->UpdateTimer("Deliver Item!");
				bTimeDone = true;
				return;
			}
		}

		GameWidget->UpdateTimer(FString::FromInt(static_cast<int>(TotalSecondsToHold)));
	}
}

void ABigButlerBattleGameModeBase::OnPlayerPaused(int ControllerID) const
{
	const auto Controller = UGameplayStatics::GetPlayerControllerFromID(GetWorld(), ControllerID);

	if (!PauseWidget->IsVisible())
	{
		PauseWidget->SetVisibility(ESlateVisibility::Visible);
		PauseWidget->FocusWidget(Controller);
		UGameplayStatics::SetGamePaused(GetWorld(), true);
	}
	else
	{
		if (Controller == PauseWidget->GetOwningPlayerController())
		{
			OnPlayerContinued(ControllerID);
		}
	}
}

void ABigButlerBattleGameModeBase::OnPlayerContinued(const int ControllerID) const
{
	if (PauseWidget->IsVisible())
	{
		PauseWidget->SetVisibility(ESlateVisibility::Hidden);
		auto Controller = UGameplayStatics::GetPlayerControllerFromID(GetWorld(), ControllerID);
		Controller->SetInputMode(FInputModeGameOnly());
		PauseWidget->Reset();
		UGameplayStatics::SetGamePaused(GetWorld(), false);
	}
}

void ABigButlerBattleGameModeBase::OnGameFinished(const int ControllerID) const
{
	if (!GameFinishedWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("Game finished widget not setup!"));
		UE_LOG(LogTemp, Warning, TEXT("Player %i won!"), ControllerID +1);
		UGameplayStatics::OpenLevel(GetWorld(), "MainMenu");
	}

	if (!GameFinishedWidget->IsVisible())
	{
		GameFinishedWidget->SetWonText("Player " + FString::FromInt(ControllerID + 1) + " won!");
		GameFinishedWidget->SetVisibility(ESlateVisibility::Visible);
		const auto Controller = Cast<APlayerCharacterController>(UGameplayStatics::GetPlayerControllerFromID(GetWorld(), ControllerID));
		GameFinishedWidget->FocusWidget(Controller);
		UGameplayStatics::SetGamePaused(GetWorld(), true);
	}
}

void ABigButlerBattleGameModeBase::OnPlayerQuit() const
{
	UE_LOG(LogTemp, Warning, TEXT("GM: Player Quit"));

	Cast<UButlerGameInstance>(GetGameInstance())->LevelChanged(true);

	UGameplayStatics::OpenLevel(GetWorld(), "MainMenu");
}

void ABigButlerBattleGameModeBase::SetupSpawnpoints()
{
	for (TActorIterator<ASpawnpoint> Iter(GetWorld()); Iter; ++Iter)
	{
		if (!Spawnpoints.Find(Iter->RoomSpawn))
			Spawnpoints.Add(Iter->RoomSpawn);

		Spawnpoints[Iter->RoomSpawn].Add(*Iter);
	}
}

void ABigButlerBattleGameModeBase::BeginTaskGeneration()
{
	RemainingTasksToCreate = TotalTasks;

	// The actual tasks to generate player tasks from
	TArray<UTask*> Tasks;

	// Putting the ranges in a map for an easier/optimal algorithm.
	TMap<EObjectType, FIntRange> Ranges
	{
		{EObjectType::Drink, WineRange},
		{EObjectType::Food, FoodRange},
		{EObjectType::Cutlery, CutleryRange}
	};

	// Get all tasks available in the world
	TMap<EObjectType, TArray<UTask*>> WorldTaskData = GetWorldTaskData();
	if (!WorldTaskData.Num())
	{
		EndTaskGeneration(Tasks);
		return;
	}

	// Setup random generator and get seed from game instance
	auto Instance = Cast<UButlerGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	FRandomStream Stream;
	Stream.Initialize(Instance->GetCurrentRandomSeed());

	// Shuffle the tasks
	for (auto& TaskData : WorldTaskData)
	{
		btd::ShuffleArray(TaskData.Value, Stream);
	}

	// Get all types
	TArray<EObjectType> Types;
	WorldTaskData.GetKeys(Types);
	if (!Types.Num())
	{
		EndTaskGeneration(Tasks);
		return;
	}

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

	/* Create minimum number of tasks */

	btd::ShuffleArray(Types, Stream);

	// Get the tasks
	auto TasksCreated = GenerateTasks(Types, Ranges, Stream, WorldTaskData, true);

	// Append them
	Tasks += TasksCreated;

	// Check if we are done early
	if (RemainingTasksToCreate <= 0)
	{
		EndTaskGeneration(Tasks);
		return;
	}

	btd::ShuffleArray(Types, Stream);

	/* Create remaining tasks */

	while (RemainingTasksToCreate > 0)
	{
		// Get the tasks
		TasksCreated = GenerateTasks(Types, Ranges, Stream, WorldTaskData, false);

		// Append them
		Tasks += TasksCreated;

		// If all ranges max values are zero we are done
		int Sum = 0;
		for (auto& Range : Ranges)
			Sum += Range.Value.Max;

		if (Sum <= 0)
			break;
	}

	EndTaskGeneration(Tasks);
}

TMap<EObjectType, TArray<UTask*>> ABigButlerBattleGameModeBase::GetWorldTaskData() const
{
	TMap<EObjectType, TArray<UTask*>> WorldTaskData;

	// Get all actors that are task objects
	for (TActorIterator<ATaskObject> Itr(GetWorld()); Itr; ++Itr)
	{
		if (!Itr)
			continue;

		// Get the task
		auto Task = Itr->GetTaskData();

		// If the task is legit
		if (Task && Task->Type != EObjectType::None)
		{
			// [] operator on TMaps doesn't automatically add, so we do it manually
			if (!WorldTaskData.Find(Task->Type))
				WorldTaskData.Add(Task->Type, TArray<UTask*>());

			// Add the task to the types TArray of tasks
			WorldTaskData[Task->Type].Add(Task);
		}
	}

	return WorldTaskData;
}

TArray<UTask*> ABigButlerBattleGameModeBase::GenerateTasks(const TArray<EObjectType>& Types, TMap<EObjectType, FIntRange>& Ranges, const FRandomStream& Stream, TMap<EObjectType, TArray<UTask*>>& WorldTaskData, const bool bShouldGenerateMinTasks)
{
	TArray<UTask*> Tasks;

	int MinToCreate, MaxToCreate;

	for (auto& Type : Types)
	{
		// Get the tasks available
		auto& TaskData = WorldTaskData[Type];

		// When we want to add minimum number of tasks
		if (bShouldGenerateMinTasks)
		{
			// Update min in case there are not enough tasks available
			Ranges[Type].Min = FMath::Clamp(FMath::Min(TaskData.Num(), Ranges[Type].Min), 0, RemainingTasksToCreate);

			if (Ranges[Type].Min == 0)
				continue;

			MinToCreate = Ranges[Type].Min;
			MaxToCreate = Ranges[Type].Min;
		}
		// When we want to fill in the rest
		else
		{
			// Calculate new max number of tasks for this type based on tasks available
			Ranges[Type].Max = FMath::Clamp(FMath::Min(TaskData.Num(), Ranges[Type].Max), 0, RemainingTasksToCreate);

			if (Ranges[Type].Max == 0)
				continue;

			MinToCreate = 0;
			MaxToCreate = Ranges[Type].Max;
		}

		// Get the tasks
		auto TasksCreated = ProcessWorldTasks(TaskData, Stream, MinToCreate, MaxToCreate);

		// Append them
		Tasks += TasksCreated;

		// Update max tasks left for this type
		Ranges[Type].Max -= TasksCreated.Num();

		if (RemainingTasksToCreate <= 0)
		{
			return Tasks;
		}
	}

	return Tasks;
}

TArray<UTask*> ABigButlerBattleGameModeBase::ProcessWorldTasks(TArray<UTask*>& TaskData, const FRandomStream& Stream, const int Min, const int Max)
{
	TArray<UTask*> Tasks;

	const int TasksToAdd = Stream.RandRange(Min, Max);

	if (TasksToAdd == 0)
		return Tasks;

	for (int i = 0; i < TasksToAdd; ++i)
	{
		Tasks.Add(TaskData[i]);

		RemainingTasksToCreate -= 1;

		if (RemainingTasksToCreate <= 0)
		{
			return Tasks;
		}
	}

	TaskData.RemoveAt(0, TasksToAdd);

	return Tasks;
}

void ABigButlerBattleGameModeBase::EndTaskGeneration(TArray<UTask*> Tasks)
{
	if (!Tasks.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("No tasks were created. Hopefully it was intended."));
		return;
	}

	if (Tasks.Num() > TotalTasks)
	{
		UE_LOG(LogTemp, Error, TEXT("Current task count is %i. It's greater than set max!"), Tasks.Num());
		UE_LOG(LogTemp, Error, TEXT("Shaving off %i tasks to make the round playable"), Tasks.Num() - TotalTasks);
		Tasks.RemoveAt(0, Tasks.Num() - TotalTasks);
	}

	GeneratePlayerTasks(Tasks);

	const double TaskGenerationEndTime = FPlatformTime::Seconds();

	UE_LOG(LogTemp, Warning, TEXT("Task generation finished. Total time used: %f"), TaskGenerationEndTime - TaskGenerationStartTime);
}

void ABigButlerBattleGameModeBase::GeneratePlayerTasks(TArray<UTask*> Tasks)
{
	for (auto& Controller : Controllers)
	{
		auto ID = UGameplayStatics::GetPlayerControllerID(Controller);
		TArray<TPair<UTask*, ETaskState>> PlayerTasks;
		for (auto& Task : Tasks)
		{
			PlayerTasks.Add(TPair<UTask*, ETaskState>(Task, ETaskState::NotPresent));
		}
		Controller->SetPlayerTasks(PlayerTasks);
	}
}

void ABigButlerBattleGameModeBase::GenerateExtraTask()
{
	auto TaskData = GetWorldTaskData();
	TArray<EObjectType> Types;
	TaskData.GetKeys(Types);

	auto Instance = Cast<UButlerGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	FRandomStream Stream;
	Stream.Initialize(Instance->GetCurrentRandomSeed());

	auto Tasks = ProcessWorldTasks(TaskData[Types[FMath::RandRange(0, Types.Num() - 1)]], Stream, 1, 1);
	GeneratePlayerTasks(Tasks);
}

void ABigButlerBattleGameModeBase::OnMainItemPickedUp()
{
	bItemCurrentlyBeingHeld = true;
}

void ABigButlerBattleGameModeBase::OnMainItemDropped()
{
	bItemCurrentlyBeingHeld = false;
}

ASpawnpoint* ABigButlerBattleGameModeBase::GetRandomSpawnpoint(const ERoomSpawn Room, const FVector& Position)
{
	float ClosestDistance = MAX_FLT;
	ASpawnpoint* ClosestSpawnpoint = nullptr;
	
	for (auto& Spawnpoint : Spawnpoints[Room])
	{
		const auto Distance = FVector::Distance(Position, Spawnpoint->GetActorLocation());
		if (Distance < ClosestDistance)
		{
			ClosestSpawnpoint = Spawnpoint;
			ClosestDistance = Distance;
		}
	}

	return ClosestSpawnpoint;
}
