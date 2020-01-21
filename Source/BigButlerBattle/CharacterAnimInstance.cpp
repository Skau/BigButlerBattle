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

	character->GetMesh()->GetRefPosePosition

	PelvisStartRotation = character->GetCharacterBoneTransform("pelvis").GetRotation();
	LeftFootStartRotation = character->GetCharacterBoneTransform("foot_l").GetRotation();
	RightFootStartRotation = character->GetCharacterBoneTransform("foot_r").GetRotation();
	PelvisStartYawOffset = character->GetCharacterBoneTransform("pelvis", FTransform::Identity).Rotator().Yaw;
	LeftFootStartYawOffset = character->GetCharacterBoneTransform("foot_l", FTransform::Identity).Rotator().Yaw;
	RightFootStartYawOffset = character->GetCharacterBoneTransform("foot_r", FTransform::Identity).Rotator().Yaw;
	LeftKneeStartYawOffset = character->GetCharacterBoneTransform("calf_l", FTransform::Identity).Rotator().Yaw;
	RightKneeStartYawOffset = character->GetCharacterBoneTransform("calf_r", FTransform::Identity).Rotator().Yaw;
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
	auto legForward = legT.GetRotation() * FVector::ForwardVector;
	// legForward.Z = 0;
	// legForward.Normalize();
	// q1^-1 * q2 = qDiff
	// q^-1 = q.conj() / q.length()
	// q.length = dot(q, q)

	auto pelvisT = character->GetCharacterBoneTransform("pelvis", FTransform::Identity);
	auto pelvisForward = pelvisT.GetRotation() * FVector::ForwardVector;
	// 	pelvisForward.Z = 0;
	// pelvisForward.Normalize();

	auto fpDelta0 = LeftFootStartYawOffset - PelvisStartYawOffset;
	auto fpDelta1 = legT.Rotator().Yaw - pelvisT.Rotator().Yaw;
	
	auto kpDelta0 = LeftKneeStartYawOffset - PelvisStartYawOffset; 
	auto kpDelta1 = (!FMath::IsNearlyZero(fpDelta0)) ? (kpDelta0 * fpDelta1) / fpDelta0 : 0.f;

	auto kneeDir = (-FVector::RightVector).RotateAngleAxis(kpDelta1, FVector::UpVector);
	auto kneepos = legT.GetLocation() + (pelvisT.GetLocation() - legT.GetLocation()) * 0.5f + kneeDir * 100.f;
	UE_LOG(LogTemp, Warning, TEXT("kpDelta0: %f"), kpDelta0);
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