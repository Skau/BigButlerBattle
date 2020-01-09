// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacterController.h"
#include "PlayerCharacter.h"

APlayerCharacterController::APlayerCharacterController()
{

}

void APlayerCharacterController::BeginPlay()
{
	Super::BeginPlay();

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
	InputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &APlayerCharacterController::Jump);
	InputComponent->BindAction("Handbrake", EInputEvent::IE_Pressed, this, &APlayerCharacterController::Handbrake);
	InputComponent->BindAction("Handbrake", EInputEvent::IE_Released, this, &APlayerCharacterController::LetGoHandBrake);

	// Axis Mappings
	InputComponent->BindAxis("Forward", this, &APlayerCharacterController::MoveForward);
	InputComponent->BindAxis("Right", this, &APlayerCharacterController::MoveRight);

}

void APlayerCharacterController::MoveForward(float Value)
{
	if ((bAllowBrakingWhileHandbraking && Value < 0.0f) || (!bHoldingHandbrake && Value != 0))
	{
		ControlledPlayer->AddMovementInput(FVector::ForwardVector * Value);
	}
}

void APlayerCharacterController::MoveRight(float Value)
{
	if (bHoldingHandbrake)
	{
		ControlledPlayer->SetRightAxisValue(Value);
	}
	if (!bHoldingHandbrake)
	{
		ControlledPlayer->SetRightAxisValue(0);
		ControlledPlayer->AddMovementInput(FVector::RightVector * Value);
	}
}

void APlayerCharacterController::Jump()
{
	if (IsValid(ControlledPlayer) && !bHoldingHandbrake)
		ControlledPlayer->Jump();
}

void APlayerCharacterController::Handbrake()
{
	bHoldingHandbrake = true;
	ControlledPlayer->ToggleHoldingHandbrake(true);
}

void APlayerCharacterController::LetGoHandBrake()
{
	bHoldingHandbrake = false;
	ControlledPlayer->ToggleHoldingHandbrake(false);
}
