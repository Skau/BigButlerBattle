// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterAnimInstance.h"
#include "Player/PlayerCharacter.h"
#include "Player/PlayerCharacterMovementComponent.h"

void UCharacterAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	auto character = Cast<APlayerCharacter>(TryGetPawnOwner());

	if (!IsValid(character))
	{
		UE_LOG(LogTemp, Warning, TEXT("Character isn't valid!"));
		return;
	}

	character->OnJumpEvent.AddUObject(this, &UCharacterAnimInstance::JumpAnim);
}

void UCharacterAnimInstance::ForwardKick()
{
	if (IsValid(ForwardMontage) && !IsAnyMontagePlaying())
		Montage_Play(ForwardMontage);
}

void UCharacterAnimInstance::JumpAnim()
{
	if (IsValid(JumpMontage))
        Montage_Play(JumpMontage);
}