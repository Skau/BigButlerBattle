// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterAnimInstance.h"
#include "Player/PlayerCharacter.h"

UCharacterAnimInstance::UCharacterAnimInstance()
	: Super()
{
	bLeftLegIK = true;
	bRightLegIK = true;
}

void UCharacterAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	auto Character = Cast<APlayerCharacter>(TryGetPawnOwner());

	if (!IsValid(Character))
	{
		UE_LOG(LogTemp, Warning, TEXT("Character isn't valid!"));
		return;
	}

	Character->OnJumpEvent.AddUObject(this, &UCharacterAnimInstance::JumpAnim);
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