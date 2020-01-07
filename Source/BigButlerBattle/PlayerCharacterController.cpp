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