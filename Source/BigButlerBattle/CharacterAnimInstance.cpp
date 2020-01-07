// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterAnimInstance.h"
#include "PlayerCharacter.h"
#include "PlayerCharacterMovementComponent.h"

void UCharacterAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	Character = Cast<APlayerCharacter>(TryGetPawnOwner());
	check(Character != nullptr);
	MovementComp = Cast<UPlayerCharacterMovementComponent>(Character->GetMovementComponent());
	check(MovementComp != nullptr);
}

bool UCharacterAnimInstance::isReady()
{
	return IsValid(Character) && IsValid(MovementComp);
}

void UCharacterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (!isReady())
		return;

	// Same tick as Event Blueprint Update Animation in anim blueprint
}


