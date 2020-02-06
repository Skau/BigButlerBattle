// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacterController.h"
#include "UI/PlayerWidget.h"
#include "PlayerCharacter.h"
#include "BigButlerBattleGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Tasks/Task.h"
#include "King/King.h"
#include "Tasks/TaskObject.h"
#include "GameFramework/PlayerStart.h"
#include "Utils/btd.h"

APlayerCharacterController::APlayerCharacterController() 
{
}

void APlayerCharacterController::BeginPlay()
{
	Super::BeginPlay();

	bShowMouseCursor = false;

	// Add in-game UI if we're actually in the game and not main menu
	PlayerCharacter = Cast<APlayerCharacter>(GetPawn());
	if (PlayerCharacter && PlayerWidgetType)
	{
		PlayerWidget = CreateWidget<UPlayerWidget>(this, PlayerWidgetType);
		PlayerWidget->AddToPlayerScreen();

		PlayerCharacter->OnTaskObjectPickedUp.BindUObject(this, &APlayerCharacterController::OnPlayerPickedUpObject);
		PlayerCharacter->OnTaskObjectDropped.BindUObject(this, &APlayerCharacterController::OnPlayerDroppedObject);
	}
}

void APlayerCharacterController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void APlayerCharacterController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlayerCharacterController::SetupInputComponent()
{
	Super::SetupInputComponent();
	check(InputComponent != nullptr);

	InputComponent->BindAction("PauseGame", EInputEvent::IE_Pressed, this, &APlayerCharacterController::PauseGamePressed);
}

void APlayerCharacterController::PauseGamePressed()
{
	auto ID = UGameplayStatics::GetPlayerControllerID(this);
	OnPausedGame.ExecuteIfBound(ID);
}

void APlayerCharacterController::SetPlayerTasks(const TArray<TPair<UTask*, ETaskState>>& Tasks)
{
	PlayerTasks += Tasks;

	PlayerWidget->InitializeTaskWidgets(Tasks);

	for (int i = 0; i < PlayerTasks.Num(); ++i)
	{
		SetPlayerTaskName(i, PlayerTasks[i].Key->Name);
		SetPlayerTaskState(i, PlayerTasks[i].Value);
	}
}

void APlayerCharacterController::SetPlayerTaskName(int Index, FString Name)
{
	PlayerWidget->UpdateTaskSlotName(Index, Name);
}

void APlayerCharacterController::SetPlayerTaskState(int Index, ETaskState NewState)
{
	PlayerTasks[Index].Value = NewState;
	PlayerWidget->UpdateTaskState(Index, NewState);
}

void APlayerCharacterController::OnPlayerPickedUpObject(ATaskObject* Object)
{
	UpdatePlayerTasks();
}

void APlayerCharacterController::OnPlayerDroppedObject(ATaskObject* Object)
{
	UpdatePlayerTasks();

	Object->OnTaskObjectDelivered.BindUObject(this, &APlayerCharacterController::OnTaskObjectDelivered);
}

void APlayerCharacterController::UpdatePlayerTasks()
{
	for (int i = 0; i < PlayerTasks.Num(); ++i)
	{
		if(PlayerTasks[i].Value != ETaskState::Finished)
			SetPlayerTaskState(i, ETaskState::NotPresent);
	}

	auto Inventory = PlayerCharacter->GetInventory();
	TArray<int> IndicesFound;
	for (int i = 0; i < Inventory.Num(); ++i)
	{
		if (auto Obj = Inventory[i])
		{
			auto TaskData = Obj->GetTaskData();

			for (int j = 0; j < PlayerTasks.Num(); ++j)
			{
				if (IndicesFound.Find(j) == INDEX_NONE)
				{
					auto PlayerTask = PlayerTasks[j];
					if (PlayerTask.Value != ETaskState::Finished && TaskData->IsEqual(PlayerTask.Key))
					{
						SetPlayerTaskState(j, ETaskState::Present);
						IndicesFound.Add(j);
						break;
					}
				}
			}
		}
	}
}

void APlayerCharacterController::OnTaskObjectDelivered(ATaskObject* Object)
{
	for (int i = 0; i < PlayerTasks.Num(); ++i)
	{
		auto PlayerTask = PlayerTasks[i];
		if (PlayerTask.Value == ETaskState::NotPresent)
		{
			if (PlayerTask.Key->IsEqual(Object->GetTaskData()))
			{
				SetPlayerTaskState(i, ETaskState::Finished);
				Object->Destroy();
				break;
			}
		}
	}
}

void APlayerCharacterController::OnCharacterFell()
{
	btd::Delay(this, RespawnTime, [=]()
	{
		if (PlayerCharacter)
		{
			bAutoManageActiveCameraTarget = false;
			UnPossess();
			PlayerCharacter->Destroy();
			PlayerCharacter = nullptr;
		}
		RespawnCharacter();
	});
}

void APlayerCharacterController::CheckIfTasksAreDone(TArray<ATaskObject*>& Inventory)
{
	for (int i = 0; i < Inventory.Num(); ++i)
	{
		if (Inventory[i] == nullptr)
			continue;

		for (int j = 0; j < PlayerTasks.Num(); ++j)
		{
			if (PlayerTasks[j].Value == ETaskState::Present)
			{
				if (Inventory[i] != nullptr)
				{
					auto Task = Inventory[i]->GetTaskData();
					if (PlayerTasks[j].Key->IsEqual(Task))
					{
						Inventory[i]->Destroy();
						Inventory[i] = nullptr;
						SetPlayerTaskState(j, ETaskState::Finished);
					}
				}
			}
		}
	}

	for (auto& Task : PlayerTasks)
	{
		if (Task.Value != ETaskState::Finished)
			return;
	}

	auto ID = UGameplayStatics::GetPlayerControllerID(this);
	OnGameFinished.ExecuteIfBound(ID);
}

void APlayerCharacterController::RespawnCharacter(APlayerStart* PlayerStart)
{
	if (!PlayerCharacterClass)
	{
		UE_LOG(LogTemp, Error, TEXT("Player Controller: Player Character Class not set!"));
		return;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	if (PlayerStart)
	{
		SpawnTransform = PlayerStart->GetActorTransform();
	}

	PlayerCharacter = GetWorld()->SpawnActor<APlayerCharacter>(PlayerCharacterClass, SpawnTransform, Params);
	Possess(PlayerCharacter);
	SetViewTargetWithBlend(PlayerCharacter, 0.5f, EViewTargetBlendFunction::VTBlend_Cubic, 0.5f, true);
	PlayerCharacter->OnCharacterFell.BindUObject(this, &APlayerCharacterController::OnCharacterFell);
}
