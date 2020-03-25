// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterAnimInstance.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Player/PlayerCharacter.h"
#include "Utils/btd.h"

UCharacterAnimInstance::UCharacterAnimInstance()
	: Super()
{
	bLeftLegIK = true;
	bRightLegIK = true;
}

void UCharacterAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	Character = Cast<APlayerCharacter>(TryGetPawnOwner());

	if (!IsValid(Character))
	{
		UE_LOG(LogTemp, Warning, TEXT("Character isn't valid!"));
		return;
	}

	Character->OnJumpEvent.AddUObject(this, &UCharacterAnimInstance::JumpAnim);
}

void UCharacterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	if (!Character)
		return;

	bIsFalling = Character->GetMovementComponent()->IsFalling();

	auto newInput = Character->GetInputAxis();
	if (newInput.X < 0.f)
		newInput.X = 0.f;

	if (bChangedDirections)
		bChangedDirections = false;
	else
	{
		bChangedDirections = btd::sign(newInput.X, 0.01f) != btd::sign(Input.X, 0.01f) || btd::sign(newInput.Y, 0.01f) != btd::sign(Input.Y, 0.01f);
	}

	Input = newInput;
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