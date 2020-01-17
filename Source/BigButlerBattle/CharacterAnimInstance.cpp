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
	{
		return;
	}

	// Same tick as Event Blueprint Update Animation in anim blueprint

	LeftFootTarget = GetFootLeftLocation();
	RightFootTarget = GetFootRightLocation();

	UE_LOG(LogTemp, Warning, TEXT("Foot left pos: %s"), *LeftFootTarget.ToString());
	UE_LOG(LogTemp, Warning, TEXT("Foot right pos: %s"), *RightFootTarget.ToString());
}

// TPair<FVector, FVector> UCharacterAnimInstance::GetFeetLocations() const
// {
// 	return IsValid(Character) ? Character->GetSkateboardFeetLocations() : TPair<FVector, FVector>{FVector{}, FVector{}};
// }

FVector UCharacterAnimInstance::GetFootLeftLocation() const
{
	return IsValid(Character) ? Character->GetSkateboardFeetLocations().Key : FVector::ZeroVector;
}

FVector UCharacterAnimInstance::GetFootRightLocation() const
{
	return IsValid(Character) ? Character->GetSkateboardFeetLocations().Value : FVector::ZeroVector;
}

