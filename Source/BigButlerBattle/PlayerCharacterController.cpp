// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacterController.h"
#include "PlayerCharacter.h"
#include "PlayerWidget.h"
#include "GameFramework/CharacterMovementComponent.h"

APlayerCharacterController::APlayerCharacterController()
{

}

void APlayerCharacterController::BeginPlay()
{
	Super::BeginPlay();

	//SetInputMode(FInputModeUIOnly());
	//if (PlayerWidgetType)
	//{
	//	PlayerWidget = Cast<UPlayerWidget>(CreateWidget(this, PlayerWidgetType));
	//	PlayerWidget->AddToViewport(0);
	//}
}

void APlayerCharacterController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ControlledPlayer = Cast<APlayerCharacter>(InPawn);
	check(ControlledPlayer != nullptr);
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
