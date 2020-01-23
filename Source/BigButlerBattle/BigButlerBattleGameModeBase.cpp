// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.


#include "BigButlerBattleGameModeBase.h"
#include "ButlerGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "PlayerCharacterController.h"
#include "PlayerCharacter.h"
#include "PauseWidget.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "ButlerGameInstance.h"

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
