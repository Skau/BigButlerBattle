// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterAnimInstance.h"
#include "PlayerCharacter.h"
#include "PlayerCharacterMovementComponent.h"

void UCharacterAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	auto character = Cast<APlayerCharacter>(TryGetPawnOwner());

	if (!IsValid(character))
	{
		UE_LOG(LogTemp, Warning, TEXT("Character isn't valid!"));
		return;
	}

	PelvisStartRotation = character->GetCharacterRefPoseBoneTransform("pelvis").GetRotation();
	LeftFootStartRotation = character->GetCharacterRefPoseBoneTransform("foot_l").GetRotation();
	RightFootStartRotation = character->GetCharacterRefPoseBoneTransform("foot_r").GetRotation();
	PelvisStartYawOffset = character->GetCharacterRefPoseBoneTransform("pelvis", FTransform::Identity).Rotator().Yaw;
	LeftFootStartYawOffset = character->GetCharacterRefPoseBoneTransform("foot_l", FTransform::Identity).Rotator().Yaw;
	RightFootStartYawOffset = character->GetCharacterRefPoseBoneTransform("foot_r", FTransform::Identity).Rotator().Yaw;
	LeftKneeStartYawOffset = character->GetCharacterRefPoseBoneTransform("calf_l", FTransform::Identity).Rotator().Yaw;
	RightKneeStartYawOffset = character->GetCharacterRefPoseBoneTransform("calf_r", FTransform::Identity).Rotator().Yaw;

	character->OnJumpEvent.AddUObject(this, &UCharacterAnimInstance::JumpAnim);
}

void UCharacterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	// Same tick as Event Blueprint Update Animation in anim blueprint
	auto character = Cast<APlayerCharacter>(TryGetPawnOwner());

	if (!IsValid(character))
		return;

	LeftFootTarget = GetFootLeftLocation(character);
	RightFootTarget = GetFootRightLocation(character);
}

void UCharacterAnimInstance::JumpAnim()
{
	if (IsValid(JumpMontage))
        Montage_Play(JumpMontage);
}

// TPair<FVector, FVector> UCharacterAnimInstance::GetFeetLocations() const
// {
// 	return IsValid(Character) ? Character->GetSkateboardFeetLocations() : TPair<FVector, FVector>{FVector{}, FVector{}};
// }

FVector UCharacterAnimInstance::GetFootLeftLocation(APlayerCharacter* character) const
{
	auto pos = IsValid(character) ? character->GetSkateboardFeetLocations().Key : FVector::ZeroVector;
	auto delta = character->GetCharacterBoneTransform("foot_l").GetLocation() - character->GetCharacterBoneTransform("ball_l").GetLocation();
	pos += delta;
	return pos;
}

FVector UCharacterAnimInstance::GetFootRightLocation(APlayerCharacter* character) const
{
	auto pos = IsValid(character) ? character->GetSkateboardFeetLocations().Value : FVector::ZeroVector;
	auto delta = character->GetCharacterBoneTransform("foot_r").GetLocation() - character->GetCharacterBoneTransform("ball_r").GetLocation();
	pos += delta;
	return pos;
}