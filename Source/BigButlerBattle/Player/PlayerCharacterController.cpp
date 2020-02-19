// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacterController.h"
#include "UI/PlayerWidget.h"
#include "PlayerCharacter.h"
#include "BigButlerBattleGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Tasks/Task.h"
#include "Tasks/TaskObject.h"
#include "Utils/btd.h"
#include "ButlerGameInstance.h"

APlayerCharacterController::APlayerCharacterController() 
{
}

void APlayerCharacterController::BeginPlay()
{
	Super::BeginPlay();

	bShowMouseCursor = false;

	ButlerGameMode = Cast<ABigButlerBattleGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
}

void APlayerCharacterController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	PlayerCharacter = Cast<APlayerCharacter>(InPawn);
	if (PlayerCharacter)
	{
		PlayerCharacter->OnTaskObjectPickedUp.BindUObject(this, &APlayerCharacterController::OnPlayerPickedUpObject);
		PlayerCharacter->OnTaskObjectDropped.BindUObject(this, &APlayerCharacterController::OnPlayerDroppedObject);
		PlayerCharacter->OnCharacterFell.BindUObject(this, &APlayerCharacterController::OnCharacterFell);
		PlayerCharacter->OnDeliverTasks.BindUObject(this, &APlayerCharacterController::CheckIfTasksAreDone);

		if (!PlayerWidget && PlayerWidgetType)
		{
			PlayerWidget = CreateWidget<UPlayerWidget>(this, PlayerWidgetType);
			PlayerWidget->AddToPlayerScreen();
		}
		SetViewTargetWithBlend(PlayerCharacter, 0.5f, EViewTargetBlendFunction::VTBlend_Cubic, 0.5f, true);

		UpdatePlayerTasks();

		if (PlayerWidget->Visibility == ESlateVisibility::Hidden)
			PlayerWidget->SetVisibility(ESlateVisibility::Visible);

		auto GameInstance = Cast<UButlerGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
		check(GameInstance != nullptr);
		PlayerCharacter->SetCameraInvertYaw(GameInstance->PlayerOptions[UGameplayStatics::GetPlayerControllerID(this)].InvertCameraYaw);
		PlayerCharacter->SetCameraInvertPitch(GameInstance->PlayerOptions[UGameplayStatics::GetPlayerControllerID(this)].InvertCameraPitch);
	}
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
	const auto ID = UGameplayStatics::GetPlayerControllerID(this);
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

void APlayerCharacterController::SetPlayerTaskName(int Index, const FString& Name) const
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
		if(PlayerTasks[i].Value == ETaskState::Present)
			SetPlayerTaskState(i, ETaskState::NotPresent);

	for (auto& InventoryObject : PlayerCharacter->GetInventory())
	{
		if (IsValid(InventoryObject))
		{
			for (int i = 0; i < PlayerTasks.Num(); ++i)
			{
				if (PlayerTasks[i].Value == ETaskState::NotPresent && InventoryObject->GetTaskData()->IsEqual(PlayerTasks[i].Key))
				{
					SetPlayerTaskState(i, ETaskState::Present);
					break;
				}
			}
		}
	}
}

void APlayerCharacterController::OnTaskObjectDelivered(ATaskObject* Object)
{
	for (int i = 0; i < PlayerTasks.Num(); ++i)
	{
		const auto PlayerTask = PlayerTasks[i];
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

void APlayerCharacterController::OnCharacterFell(ERoomSpawn Room, const FVector Position)
{
	PlayerWidget->SetVisibility(ESlateVisibility::Hidden);
	btd::Delay(this, RespawnTime, [=]()
	{
		if (PlayerCharacter)
		{
			bAutoManageActiveCameraTarget = false;
			UnPossess();
			PlayerCharacter->Destroy();
			PlayerCharacter = nullptr;
		}

		if (!ButlerGameMode)
		{
			UE_LOG(LogTemp, Error, TEXT("APlayerCharacterController::RespawnCharacter: Wrong gamemode setup!"));
			return;
		}

		const auto Spawnpoint = ButlerGameMode->GetRandomSpawnpoint(Room, Position);
		RespawnCharacter(Spawnpoint);
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
					const auto Task = Inventory[i]->GetTaskData();
					if (PlayerTasks[j].Key->IsEqual(Task))
					{
						Inventory[i]->Destroy();
						Inventory[i] = nullptr;
						SetPlayerTaskState(j, ETaskState::Finished);

						if(PlayerCharacter->GetCurrentItemIndex() == i)
							PlayerCharacter->IncrementCurrentItemIndex();
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

	const auto ID = UGameplayStatics::GetPlayerControllerID(this);
	OnGameFinished.ExecuteIfBound(ID);
}

void APlayerCharacterController::RespawnCharacter(ASpawnpoint* Spawnpoint)
{
	if (!PlayerCharacterClass)
	{
		UE_LOG(LogTemp, Error, TEXT("Player Controller: Player Character Class not set!"));
		return;
	}
	
	
	if (Spawnpoint)
	{
		const auto SpawnTransform = Spawnpoint->GetTransform();

		PlayerCharacter = GetWorld()->SpawnActorDeferred<APlayerCharacter>(PlayerCharacterClass, SpawnTransform, this, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

		PlayerCharacter->SetCustomSpringArmLength();

		PlayerCharacter->CurrentRoom = Spawnpoint->RoomSpawn;

		UGameplayStatics::FinishSpawningActor(PlayerCharacter, SpawnTransform);

		Possess(PlayerCharacter);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Player Controller: Null spawnpoint provided!"));
	}
}
