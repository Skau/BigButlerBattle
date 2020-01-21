// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterAnimInstance.h"
#include "PlayerCharacter.h"
#include "PlayerCharacterMovementComponent.h"

void UCharacterAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	auto character = Cast<APlayerCharacter>(TryGetPawnOwner());

	if (!IsValid(character))
		return;

	PelvisStartRotation = character->GetCharacterBoneTransform("pelvis").GetRotation();
	LeftFootStartRotation = character->GetCharacterBoneTransform("foot_l").GetRotation();
	RightFootStartRotation = character->GetCharacterBoneTransform("foot_r").GetRotation();
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

	LeftLegJointLocation = GetLeftLegJointLocation(character);
	RightLegJointLocation = GetRightLegJointLocation(character);
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

FVector UCharacterAnimInstance::GetLeftLegJointLocation(APlayerCharacter* character) const
{
	auto legT = character->GetCharacterBoneTransform("foot_l", FTransform::Identity);
	auto legDiff = LeftFootStartRotation.Inverse() * legT.GetRotation();
	auto legForward = legDiff * FVector::RightVector;
	// q1^-1 * q2 = qDiff
	// q^-1 = q.conj() / q.length()
	// q.length = dot(q, q)

	auto pelvisT = character->GetCharacterBoneTransform("pelvis", FTransform::Identity);
	auto pelvisDiff = PelvisStartRotation.Inverse() * pelvisT.GetRotation();
	auto pelvisForward = pelvisDiff * FVector::RightVector;

	auto kneeDir = (legForward + pelvisForward).GetSafeNormal();
	// UE_LOG(LogTemp, Warning, TEXT("kneeDir: %s"), *kneeDir.ToString());
	auto kneepos = legT.GetLocation() + (pelvisT.GetLocation() - legT.GetLocation()) * 0.5f + kneeDir * 100.f;
	return kneepos;
}

FVector UCharacterAnimInstance::GetRightLegJointLocation(APlayerCharacter* character) const
{
	auto legT = character->GetCharacterBoneTransform("foot_r", FTransform::Identity);
	auto legDiff = LeftFootStartRotation.Inverse() * legT.GetRotation();
	auto legForward = legDiff * FVector::RightVector;

	auto pelvisT = character->GetCharacterBoneTransform("pelvis", FTransform::Identity);
	auto pelvisDiff = PelvisStartRotation.Inverse() * pelvisT.GetRotation();
	auto pelvisForward = pelvisDiff * FVector::RightVector;

	auto kneeDir = (legForward + pelvisForward).GetSafeNormal();
	auto kneepos = legT.GetLocation() + (pelvisT.GetLocation() - legT.GetLocation()) * 0.5f + kneeDir * 100.f;
	return kneepos;
}