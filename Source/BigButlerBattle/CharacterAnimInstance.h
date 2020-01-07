// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CharacterAnimInstance.generated.h"

class APlayerCharacter;
class UPlayerCharacterMovementComponent;

/**
 * 
 */
UCLASS()
class BIGBUTLERBATTLE_API UCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
	void NativeBeginPlay() override;

	UFUNCTION(BlueprintPure)
	bool isReady();

	void NativeUpdateAnimation(float DeltaTime) override;


	UPROPERTY(BlueprintReadOnly)
	APlayerCharacter* Character;

	UPROPERTY(BlueprintReadOnly)
	UPlayerCharacterMovementComponent* MovementComp;
};
