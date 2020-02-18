// Fill out your copyright notice in the Description page of Project Settings.


#include "SkateboardAnimInstance.h"
#include "Player/PlayerCharacterMovementComponent.h"
#include "Player/PlayerCharacter.h"

void USkateboardAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

    auto owner = Cast<APlayerCharacter>(TryGetPawnOwner());
    if (owner)
    {
        auto moveComp = owner->GetMovementComponent();
        movementComponent = Cast<UPlayerCharacterMovementComponent>(moveComp);

        owner->OnJumpEvent.AddUObject(this, &USkateboardAnimInstance::JumpAnim);
    }
}

void USkateboardAnimInstance::JumpAnim()
{
    if (IsValid(JumpMontage))
        Montage_Play(JumpMontage);
}

void USkateboardAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
    Super::NativeUpdateAnimation(DeltaTime);

    if (!movementComponent)
        return;

    InputRotation = movementComponent->GetRotationInput();
    bInAir = movementComponent->IsFalling();
    Velocity = movementComponent->Velocity.ContainsNaN() ? 0.f : movementComponent->Velocity.Size();
    bIsStandstill = movementComponent->IsStandstill();
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
    return !bInAir * InputRotation;
}