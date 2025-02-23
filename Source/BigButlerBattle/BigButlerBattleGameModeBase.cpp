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
#include "Tasks/Task.h"
#include "Utils/btd.h"
#include "UI/GameFinishedWidget.h"
#include "Utils/Spawnpoint.h"
#include "UI/GameWidget.h"
#include "King/King.h"
#include "NavigationSystem.h"
#include "UI/PlayerWidget.h"

ABigButlerBattleGameModeBase::ABigButlerBattleGameModeBase()
{
	PrimaryActorTick.bCanEverTick = true;
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

		// Main item event
		Controllers[i]->OnMainItemStateChange.BindUObject(this, &ABigButlerBattleGameModeBase::OnMainItemStateChanged);
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
		// Don't think this will be used much
		GameWidget = CreateWidget<UGameWidget>(Controllers[0], GameWidgetClass);
		GameWidget->AddToViewport();

		const auto Time = FString::FromInt(static_cast<int>(TotalSecondsToHold));
		for (auto& Controller : Controllers)
		{
			Controller->GetPlayerWidget()->UpdateTimer(Time);
		}
	}

	// Spawnpoints

	SetupSpawnpoints();


	King = Cast<AKing>(UGameplayStatics::GetActorOfClass(GetWorld(), AKing::StaticClass()));
	if (!IsValid(King))
	{
		UE_LOG(LogTemp, Error, TEXT("No King present in map!"));
	}

	// Wait a bit for items to initialize
	btd::Delay(this, 0.1f, [=]()
	{
		// Iterate all first to check that the main item is not set in editor.
		for (TActorIterator<ATaskObject> Itr(GetWorld()); Itr; ++Itr)
		{
			if (!Itr)
				continue;

			if (Itr->GetIsMainItem())
			{
				for (auto& Controller : Controllers)
				{
					Controller->GetPlayerWidget()->OnMainItemSet(*Itr);
				}
				return;
			}
		}

		SetNewMainItem();
	});
}


void ABigButlerBattleGameModeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bTimeDone && bItemCurrentlyBeingHeld)
	{
		SecondsLeftToHold -= DeltaTime;

		if (SecondsLeftToHold <= 0.f)
		{
			if (King)
			{
				King->bCanReceiveMainItem = true;
				if(!King->CheckIfAnyInRange())
				{
					for (auto& Controller : Controllers)
					{
						Controller->GetPlayerWidget()->UpdateTimerText(ControllerIDHoldingItem, true);
					}
				}

				bTimeDone = true;
				return;
			}
		}
		const auto Time = FString::FromInt(static_cast<int>(SecondsLeftToHold));
		for (auto& Controller : Controllers)
		{
			Controller->GetPlayerWidget()->UpdateTimer(Time);
		}
	}
}

void ABigButlerBattleGameModeBase::OnPlayerPaused(const int ControllerID) const
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
		GameFinishedWidget->SetWonText(ControllerID, "Player " + FString::FromInt(ControllerID + 1) + " won!");
		GameFinishedWidget->SetVisibility(ESlateVisibility::Visible);
		const auto Controller = Cast<APlayerCharacterController>(UGameplayStatics::GetPlayerControllerFromID(GetWorld(), ControllerID));
		GameFinishedWidget->FocusWidget(Controller);
		UGameplayStatics::SetGamePaused(GetWorld(), true);
	}

	if (WinSound)
		UGameplayStatics::PlaySound2D(this, WinSound, 1.f);
}

void ABigButlerBattleGameModeBase::OnPlayerQuit()
{
	bChangingLevels = true;
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

void ABigButlerBattleGameModeBase::OnMainItemStateChanged(const int ControllerID, const EMainItemState NewState)
{
	bItemCurrentlyBeingHeld = NewState == EMainItemState::PickedUp;

	// Play sounds on state change here
	switch (NewState)
	{
	case EMainItemState::PickedUp:
	{
		if (PickupMainItemSound)
			UGameplayStatics::PlaySound2D(this, PickupMainItemSound, 1.f);

		ControllerIDHoldingItem = ControllerID;
			
		for (auto& Controller : Controllers)
		{
			Controller->GetPlayerWidget()->UpdateTimerText(ControllerID, false);
		}
	}
	break;
	case EMainItemState::Dropped:
	{
		if(King)
			King->bCanReceiveMainItem = false;

		bTimeDone = false;

		if (DropMainItemSound)
			UGameplayStatics::PlaySound2D(this, DropMainItemSound, 1.f);

		SecondsLeftToHold = TotalSecondsToHold;
		const auto Time = FString::FromInt(static_cast<int>(SecondsLeftToHold));
		for (auto& Controller : Controllers)
		{
			Controller->GetPlayerWidget()->UpdateTimer(Time);
		}
	}
	break;
	case EMainItemState::Delivered:
	{
		if (DeliverMainItemSound)
			UGameplayStatics::PlaySound2D(this, DeliverMainItemSound, 1.f);

		const auto PlayerController = Cast<APlayerCharacterController>(UGameplayStatics::GetPlayerControllerFromID(GetWorld(), ControllerID));

		// Player won
		if (PlayerController->GetScore() >= TotalPointsToWin)
		{
			OnGameFinished(ControllerID);
			return;
		}

		SetNewMainItem(2.f);
		bTimeDone = false;
		if(King)
			King->bCanReceiveMainItem = false;
			
		SecondsLeftToHold = TotalSecondsToHold;
		const auto Time = FString::FromInt(static_cast<int>(SecondsLeftToHold));
		for (auto& Controller : Controllers)
		{
			Controller->GetPlayerWidget()->UpdateTimer(Time);
			Controller->GetPlayerWidget()->UpdateScore(ControllerID, PlayerController->GetScore());
		}
	}
	break;
	case EMainItemState::Destroyed:
		SetNewMainItem();
		break;
	}

	for (auto& Controller : Controllers)
	{
		Controller->GetPlayerWidget()->OnMainItemStateChanged(ControllerID, NewState);
	}
}

void ABigButlerBattleGameModeBase::SetNewMainItem(const float Delay)
{
	if (bChangingLevels)
		return;

	// Give it a second to let all task objects be in correct state
	btd::Delay(this, Delay, [=]()
	{
		// Set main item (get random one from all task objects)
		TArray<AActor*> Actors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATaskObject::StaticClass(), Actors);
		if (Actors.Num())
		{
			ATaskObject* Object = nullptr;
			while (!Object || Object->GetIsRespawning())
			{
				const int Index = FMath::RandRange(0, Actors.Num() - 1);
				Object = Cast<ATaskObject>(Actors[Index]);
			}

			UE_LOG(LogTemp, Warning, TEXT("%s is new main item"), *Object->GetName());
			Object->SetAsMainItem();

			for (auto& Controller : Controllers)
			{
				Controller->GetPlayerWidget()->OnMainItemSet(Object);
			}
		}

		if (NewMainItemSound)
			UGameplayStatics::PlaySound2D(this, NewMainItemSound, 1.f);
	});
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

FVector ABigButlerBattleGameModeBase::GetRandomSpawnPos(const FVector& Position) const
{
	auto World = GetWorld();
	if (!IsValid(World))
		return FVector::ZeroVector;

	const auto NavSys = Cast<UNavigationSystemV1>(World->GetNavigationSystem());
	if (!IsValid(NavSys))
		return FVector::ZeroVector;

	FNavLocation NavPoint;
	if (!IsValid(NavSys->MainNavData))
	{
		UE_LOG(LogTemp, Error, TEXT("MainNavSysData is missing!"));
		return FVector::ZeroVector;
	}

	if (!NavSys->GetRandomPointInNavigableRadius(Position, RespawnRadius, NavPoint))
	{
		UE_LOG(LogTemp, Error, TEXT("Could'nt find a point in navigable space!"));
		return FVector::ZeroVector;
	}

	return NavPoint.Location;
}
