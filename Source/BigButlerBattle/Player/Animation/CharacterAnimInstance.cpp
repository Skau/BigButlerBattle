// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterAnimInstance.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Player/PlayerCharacter.h"
#include "Utils/btd.h"
#include "Player/PlayerCharacterMovementComponent.h"

UCharacterAnimInstance::UCharacterAnimInstance()
	: Super()
{
	bLeftLegIK = true;
	bRightLegIK = true;
}

void UCharacterAnimInstance::ForwardKick()
{
	if (IsValid(ForwardMontage) && !Montage_IsPlaying(ForwardMontage))
		Montage_Play(ForwardMontage);
}

void UCharacterAnimInstance::ThrowAnim()
{
	if (IsValid(ThrowMontage) && !IsDoingAction())
		Montage_Play(ThrowMontage);
}

void UCharacterAnimInstance::TackleAnim()
{
	if (IsValid(TackleMontage) && !IsDoingAction())
		Montage_Play(TackleMontage);
}

bool UCharacterAnimInstance::IsDoingAction() const
{
	return Montage_IsPlaying(ThrowMontage) || Montage_IsPlaying(TackleMontage);
}

void UCharacterAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	Character = Cast<APlayerCharacter>(TryGetPawnOwner());

	if (!IsValid(Character))
	{
		UE_LOG(LogTemp, Warning, TEXT("Character isn't valid!"));
	}
}

void UCharacterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	if (!Character)
		return;

	const auto MoveComp = Character->GetPlayerCharacterMovementComponent();
	if (!MoveComp)
		return;

	bIsFalling = MoveComp->IsFalling();
	bIsGrinding = MoveComp->IsGrinding();

	auto NewInput = Character->GetInputAxis();
	if (NewInput.X < 0.f)
		NewInput.X = 0.f;

	if (bChangedDirections)
		bChangedDirections = false;
	else
	{
		bChangedDirections = btd::Sign(NewInput.Y, 0.01f) && btd::Sign(NewInput.Y, 0.01f) != btd::Sign(Input.Y, 0.01f);
	}

	Input = NewInput;
}