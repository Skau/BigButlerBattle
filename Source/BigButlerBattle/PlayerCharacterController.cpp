// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacterController.h"
#include "PlayerCharacter.h"
#include "PlayerWidget.h"


APlayerCharacterController::APlayerCharacterController()
{

}

void APlayerCharacterController::BeginPlay()
{
	Super::BeginPlay();

	SetInputMode(FInputModeUIOnly());

	if (PlayerWidgetType)
	{
		PlayerWidget = Cast<UPlayerWidget>(CreateWidget(this, PlayerWidgetType));
		PlayerWidget->AddToViewport(0);
	}

	ControlledPlayer = Cast<APlayerCharacter>(GetPawn());
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

	// Action Mappings
	InputComponent->BindAction("Jump", EInputEvent::IE_Pressed, ControlledPlayer, &APlayerCharacter::Jump);


	// Axis Mappings
	InputComponent->BindAxis("Forward", this, &APlayerCharacterController::MoveForward);
	InputComponent->BindAxis("Right", this, &APlayerCharacterController::MoveRight);

}

void APlayerCharacterController::MoveForward(float Value)
{
	if (Value != 0)
	{
		ControlledPlayer->AddMovementInput(ControlledPlayer->GetActorForwardVector() * Value);
	}
}

void APlayerCharacterController::MoveRight(float Value)
{
	if (Value != 0)
	{
		ControlledPlayer->AddMovementInput(ControlledPlayer->GetActorRightVector() * Value);
	}
}