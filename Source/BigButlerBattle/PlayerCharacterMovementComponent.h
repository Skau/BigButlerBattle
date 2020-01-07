// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PlayerCharacterMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class BIGBUTLERBATTLE_API UPlayerCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UPlayerCharacterMovementComponent();

protected:
	void TickComponent(float deltaTime, enum ELevelTick TickType, FActorComponentTickFunction* thisTickFunction) override;
	
};
