// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacterController.h"
#include "UI/PlayerWidget.h"
#include "PlayerCharacter.h"
#include "BigButlerBattleGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Tasks/BaseTask.h"
#include "King/King.h"
#include "Tasks/TaskObject.h"

APlayerCharacterController::APlayerCharacterController() 
{
}

void APlayerCharacterController::BeginPlay()
{
	Super::BeginPlay();

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

void APlayerCharacterController::SetPlayerTasks(const TArray<TPair<UBaseTask*, ETaskState>>& Tasks)
{
	PlayerTasks = Tasks;

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

void APlayerCharacterController::OnPlayerPickedUpObject(UBaseTask* TaskIn)
{
	for (int i = 0; i < PlayerTasks.Num(); ++i)
	{
		if (PlayerTasks[i].Value == ETaskState::NotPresent)
		{
			if (PlayerTasks[i].Key->IsEqual(TaskIn))
			{
				PlayerTasks[i].Value = ETaskState::Present;
				SetPlayerTaskState(i, ETaskState::Present);
				break;
			}
		}
	}
}

void APlayerCharacterController::OnPlayerDroppedObject(UBaseTask* TaskIn)
{
	for (int i = 0; i < PlayerTasks.Num(); ++i)
	{
		if (PlayerTasks[i].Value == ETaskState::Present)
		{
			if (PlayerTasks[i].Key->IsEqual(TaskIn))
			{
				PlayerTasks[i].Value = ETaskState::NotPresent;
				SetPlayerTaskState(i, ETaskState::NotPresent);
				break;
			}
		}
	}
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