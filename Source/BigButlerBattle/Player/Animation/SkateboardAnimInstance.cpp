// Fill out your copyright notice in the Description page of Project Settings.


#include "SkateboardAnimInstance.h"
#include "Player/PlayerCharacterMovementComponent.h"
#include "Player/PlayerCharacter.h"

void USkateboardAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

    const auto Owner = Cast<APlayerCharacter>(TryGetPawnOwner());
    if (Owner)
    {
	    const auto MoveComp = Owner->GetMovementComponent();
        MovementComponent = Cast<UPlayerCharacterMovementComponent>(MoveComp);
    }
}

void USkateboardAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
    Super::NativeUpdateAnimation(DeltaTime);

    if (!MovementComponent)
        return;

    InputRotation = MovementComponent->GetRotationInput();
    bIsFalling = MovementComponent->IsFalling();
    bIsGrinding = MovementComponent->IsGrinding();
    Velocity = MovementComponent->Velocity.ContainsNaN() ? 0.f : MovementComponent->Velocity.Size();
    bIsStandstill = MovementComponent->IsStandstill();
    // Same tick as Event Blueprint Update Animation in anim blueprint
}

float USkateboardAnimInstance::GetWheelPlaybackRate() const
{
    return !bIsStandstill * (Velocity / VelocityThreshold);
}

float USkateboardAnimInstance::GetFlipAmount() const
{
    return FMath::Abs(InputRotation) * bIsStandstill;
}

float USkateboardAnimInstance::GetRotationAmount() const
{
    return !bIsFalling * InputRotation;
}