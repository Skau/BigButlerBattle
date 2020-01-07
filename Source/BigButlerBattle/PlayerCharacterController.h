// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PlayerCharacterController.generated.h"

class APlayerCharacter;

/**
 * 
 */
UCLASS()
class BIGBUTLERBATTLE_API APlayerCharacterController : public APlayerController
{
	GENERATED_BODY()

public:
	APlayerCharacterController();

protected:
	void BeginPlay() override;

	void Tick(float DeltaTime) override;

	void SetupInputComponent() override;
	
private:
	APlayerCharacter* Character;

	void MoveForward(float Value);
	void MoveRight(float Value);
};
