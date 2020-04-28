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
			const auto ID = UGameplayStatics::GetPlayerControllerID(this);
			PlayerWidget->ID = ID;
			PlayerWidget->AddToPlayerScreen();
		}
		SetViewTargetWithBlend(PlayerCharacter, 0.5f, EViewTargetBlendFunction::VTBlend_Cubic, 0.5f, true);

		if (PlayerWidget->Visibility == ESlateVisibility::Hidden)
			PlayerWidget->SetVisibility(ESlateVisibility::Visible);

		UpdateCameraSettings();
	}
}

void APlayerCharacterController::SetupInputComponent()
{
	Super::SetupInputComponent();
	check(InputComponent != nullptr);

	FInputActionBinding ab{ "PauseGame", EInputEvent::IE_Pressed };
	ab.ActionDelegate.GetDelegateForManualSet().BindUObject(this, &APlayerCharacterController::PauseGamePressed);
	ab.bExecuteWhenPaused = true;
	InputComponent->AddActionBinding(ab);

	InputComponent->BindAction("ShowKeybinds", EInputEvent::IE_Pressed, this, &APlayerCharacterController::ShowKeybinds);
	InputComponent->BindAction("ShowKeybinds", EInputEvent::IE_Released, this, &APlayerCharacterController::HideKeybinds);
}

void APlayerCharacterController::UpdateCameraSettings()
{
	if (auto GameInstance = Cast<UButlerGameInstance>(UGameplayStatics::GetGameInstance(GetWorld())))
	{
		if (PlayerCharacter != nullptr)
		{
			PlayerCharacter->SetCameraInvertYaw(GameInstance->PlayerOptions[UGameplayStatics::GetPlayerControllerID(this)].InvertCameraYaw);
			PlayerCharacter->SetCameraInvertPitch(GameInstance->PlayerOptions[UGameplayStatics::GetPlayerControllerID(this)].InvertCameraPitch);
		}
	}
}

void APlayerCharacterController::PauseGamePressed()
{
	UE_LOG(LogTemp, Warning, TEXT("Pause game pressed"));
	const auto ID = UGameplayStatics::GetPlayerControllerID(this);
	OnPausedGame.ExecuteIfBound(ID);
}

void APlayerCharacterController::OnPlayerPickedUpObject(ATaskObject* Object)
{
	// Destroy all other items if we have picked up main item.
	if (Object->GetIsMainItem())
	{
		auto& PlayerInventory = PlayerCharacter->GetInventory();
		for(int i = 0; i < 4; ++i) // Only to 4, so we don't destroy the main object
		{
			if (PlayerInventory[i])
			{
				PlayerInventory[i]->Destroy();
				PlayerInventory[i] = nullptr;
			}
		}

		PlayerCharacter->bHasMainItem = true;
		OnMainItemStateChange.ExecuteIfBound(UGameplayStatics::GetPlayerControllerID(this), EMainItemState::PickedUp);
	}
}

void APlayerCharacterController::OnPlayerDroppedObject(ATaskObject* Object)
{
	if (Object->GetIsMainItem())
	{
		PlayerCharacter->bHasMainItem = false;
		OnMainItemStateChange.ExecuteIfBound(UGameplayStatics::GetPlayerControllerID(this), EMainItemState::Dropped);
	}
}

void APlayerCharacterController::OnCharacterFell(ERoomSpawn Room, const FVector Position)
{
	PlayerWidget->SetVisibility(ESlateVisibility::Hidden);
	btd::Delay(this, RespawnTime, [=]()
	{
		if (!GetPawn() || !this || !IsValid(PlayerCharacter))
			return;

		const auto playerScale = PlayerCharacter->GetActorScale3D();
		const auto playerRot = PlayerCharacter->GetActorRotation().Quaternion();

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

		const auto Spawnpos = ButlerGameMode->GetRandomSpawnPos(Position - playerRot * FVector::ForwardVector * 50.f);
		RespawnCharacter(FTransform{playerRot, Spawnpos, playerScale});
	});	
}

void APlayerCharacterController::CheckIfTasksAreDone(TArray<ATaskObject*>& Inventory)
{
	for (int i = 0; i < Inventory.Num(); ++i)
	{
		if (Inventory[i] == nullptr)
			continue;

		if (Inventory[i]->GetIsMainItem())
		{
			Inventory[i]->Destroy();
			Inventory[i] = nullptr;

			PlayerCharacter->bHasMainItem = false;

			++Score;

			const auto ID = UGameplayStatics::GetPlayerControllerID(this);
			OnDeliveredItem.ExecuteIfBound(ID);
		}
	}
}

void APlayerCharacterController::RespawnCharacter(ASpawnpoint* Spawnpoint)
{
	if (!IsValid(PlayerCharacterClass))
	{
		UE_LOG(LogTemp, Error, TEXT("Player Controller: Player Character Class not set!"));
		return;
	}
	
	if (Spawnpoint)
	{
		const auto SpawnTransform = Spawnpoint->GetTransform();

		RespawnCharacter(SpawnTransform);

		PlayerCharacter->CurrentRoom = Spawnpoint->RoomSpawn;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Player Controller: Null spawnpoint provided!"));
	}
}

void APlayerCharacterController::RespawnCharacter(const FTransform& Spawntrans)
{
	if (!IsValid(PlayerCharacterClass))
	{
		UE_LOG(LogTemp, Error, TEXT("Player Controller: Player Character Class not set!"));
		return;
	}

	PlayerCharacter = GetWorld()->SpawnActorDeferred<APlayerCharacter>(PlayerCharacterClass, Spawntrans, this, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

	if (bUseCustomSpringArmLength)
		PlayerCharacter->SetCustomSpringArmLength();

	UGameplayStatics::FinishSpawningActor(PlayerCharacter, Spawntrans);

	Possess(PlayerCharacter);
}

void APlayerCharacterController::ShowKeybinds()
{
	if (PlayerWidget)
	{
		PlayerWidget->ShowKeybinds();
	}
}

void APlayerCharacterController::HideKeybinds()
{
	if (PlayerWidget)
	{
		PlayerWidget->HideKeybinds();
	}
}