// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacterController.h"
#include "PlayerCharacter.h"


APlayerCharacterController::APlayerCharacterController()
{

}

void APlayerCharacterController::BeginPlay()
{
	Super::BeginPlay();

	Character = Cast<APlayerCharacter>(GetPawn());

	if(Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("Found character"))
	}

}

void APlayerCharacterController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlayerCharacterController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (InputComponent == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("InputComponent is NULL"))
		return;
	}

	// Action Mappings
	InputComponent->BindAction("Jump", EInputEvent::IE_Pressed, Character, &APlayerCharacter::Jump);


	// Axis Mappings
	InputComponent->BindAxis("Forward", this, &APlayerCharacterController::MoveForward);
	InputComponent->BindAxis("Right", this, &APlayerCharacterController::MoveRight);

}

void APlayerCharacterController::MoveForward(float Value)
{
	if (Value != 0)
	{
		Character->AddMovementInput(Character->GetActorForwardVector() * Value);
	}
}

void APlayerCharacterController::MoveRight(float Value)
{
	if (Value != 0)
	{
		Character->AddMovementInput(Character->GetActorRightVector() * Value);
	}
}