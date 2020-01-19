// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacterController.h"
#include "PlayerWidget.h"
#include "BaseUserWidget.h"
#include "PlayerCharacter.h"

APlayerCharacterController::APlayerCharacterController() {}

void APlayerCharacterController::BeginPlay()
{
	Super::BeginPlay();

	// Add in-game UI if we're actually in the game and not main menu
	PlayerCharacter = Cast<APlayerCharacter>(GetPawn());
	if (PlayerCharacter && PlayerWidgetType)
	{
		PlayerWidget = CreateWidget<UPlayerWidget>(this, PlayerWidgetType);
		PlayerWidget->AddToPlayerScreen();
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
	PauseGame.ExecuteIfBound(this);
}