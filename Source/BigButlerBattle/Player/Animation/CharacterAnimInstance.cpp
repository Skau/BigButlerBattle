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

	// // Same tick as Event Blueprint Update Animation in anim blueprint
	// auto character = Cast<APlayerCharacter>(TryGetPawnOwner());

	// if (!IsValid(character))
	// 	return;

	// if (!RefSkeletonBoneInfos.Num())
	// {
	// 	if (!GenerateRefSkeletonBoneTransforms(character))
	// 		UE_LOG(LogTemp, Error, TEXT("Failed to create reference bone hierarchy, in CharacterAnimInstance!"));
	// }

	// SkateboardRotationOffset = GetSkateboardRotationOffset(character);

	// LeftFootTarget = GetFootLocation(character, SkateboardRotationOffset.Quaternion(), true);
	// RightFootTarget = GetFootLocation(character, SkateboardRotationOffset.Quaternion(), false);
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

// TPair<FVector, FVector> UCharacterAnimInstance::GetFeetLocations() const
// {
// 	return IsValid(Character) ? Character->GetSkateboardFeetLocations() : TPair<FVector, FVector>{FVector{}, FVector{}};
// }

FVector UCharacterAnimInstance::GetLeftFootTarget() const
{
	auto character = Cast<APlayerCharacter>(TryGetPawnOwner());
	return IsValid(character) ? GetFootLocation(character, true) : FVector{};
}

FVector UCharacterAnimInstance::GetRightFootTarget() const
{
	auto character = Cast<APlayerCharacter>(TryGetPawnOwner());
	return IsValid(character) ? GetFootLocation(character, false) : FVector{};
}

FVector UCharacterAnimInstance::GetFootLocation(APlayerCharacter* character, bool left) const
{
	return GetFootLocation(character, GetSkateboardRotationOffset(character).Quaternion(), left); 
}

FVector UCharacterAnimInstance::GetFootLocation(APlayerCharacter *character, FQuat feetRotationOffset, bool left) const
{
	FVector socketPos = FVector::ZeroVector;
	if (IsValid(character))
	{
		auto trans = left ? character->GetComponentSkateboardFeetTransform().Left : character->GetComponentSkateboardFeetTransform().Right;
		trans = character->LocalSkateboardToButler(trans);
		socketPos = trans.GetTranslation();
		// socketPos = character->LocalSkateboardToButler(FTransform{}).GetTranslation();

		// UE_LOG(LogTemp, Warning, TEXT("trans: %s"), *trans.ToString());
	}

	FName footBone{ left ? "ball_l" : "ball_r"};
	FName ankleBone{left ? "foot_l" : "foot_r"};
	auto footPos = character->GetCharacterRefPoseBoneTransformRec(footBone).GetTranslation();
	auto anklePos = character->GetCharacterRefPoseBoneTransformRec(ankleBone).GetTranslation();
	auto footDir = footPos - anklePos;
	// Rotate direction vector with displacement rotation.
	footDir = feetRotationOffset * footDir;
	UE_LOG(LogTemp, Warning, TEXT("footBone is: %s, footDir: %s"), *footBone.ToString(), *footDir.ToString());
	// socketPos += FVector{10.f, 0.f, 0.f};
	// Scaled by 100 since skeleton is scaled by 100.
	socketPos -= bSkeletonScaledByHundred ? footDir * 100.f : footDir;
	return socketPos;
}

FRotator UCharacterAnimInstance::GetSkateboardRotationOffset(APlayerCharacter* character) const
{
	return IsValid(character) ? character->GetSkateboardRotation() : FRotator{};
}

FTransform UCharacterAnimInstance::GetSkeletonRefBoneTransform(FName BoneName) const
{
	if (0 < RefSkeletonBoneTransforms.Num())
	{
		auto boneIndex = GetRefBoneIndex(BoneName);
		if (boneIndex > -1)	
			return RefSkeletonBoneTransforms[boneIndex];
	}
	return FTransform{};
}

bool UCharacterAnimInstance::GenerateRefSkeletonBoneTransforms(APlayerCharacter* character)
{
	// Empty out the array first (even if we return out on the next line)
	RefSkeletonBoneInfos.Empty();
	RefSkeletonBoneTransforms.Empty();

	if (!IsValid(character))
		return false;

	// Cache a reference to real reference skeleton.
	const auto& refSkeletonRef = character->GetMesh()->SkeletalMesh->RefSkeleton;

	// Assign all bone infos to bone info array.
	RefSkeletonBoneInfos = refSkeletonRef.GetRefBoneInfo();
	// Assign all relative transforms to transform array.
	RefSkeletonBoneTransforms = refSkeletonRef.GetRefBonePose();

	if (RefSkeletonBoneInfos.Num() != RefSkeletonBoneTransforms.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("RefSkeletonBoneInfos and RefSkeletonBoneTransforms not equal length!"));
		return false;
	}

	if (RefSkeletonBoneTransforms.Num() != refSkeletonRef.GetNum())
	{
		UE_LOG(LogTemp, Error, TEXT("RefSkeletonBoneTransforms.Num() and number of bones not equal!"));
		return false;
	}

	/** 
	 * The bone hierarchy is sorted to be greater the further into the hierarchy you go, so by 
	 * increasing iteratively we can assure that we never add the child / sibling of a node to
	 * a node.
	 */
	for (int i{0}; i < refSkeletonRef.GetNum(); ++i)
	{
		if (i != 0 && (RefSkeletonBoneInfos[i].ParentIndex < 0 || RefSkeletonBoneInfos[i].ParentIndex >= RefSkeletonBoneTransforms.Num()))
		{
			UE_LOG(LogTemp, Error, TEXT("Bone %i with parent out of bounds!"), i);
			return false;
		}

		// Add the parent relative transform to child transform
		if (i != 0)
		{
			RefSkeletonBoneTransforms[i] = RefSkeletonBoneTransforms[i] * RefSkeletonBoneTransforms[RefSkeletonBoneInfos[i].ParentIndex];
		}
	}

	return true;
}

int32 UCharacterAnimInstance::GetRefBoneIndex(FName BoneName) const
{
	for (int i{0}; i < RefSkeletonBoneInfos.Num(); ++i)
		if (RefSkeletonBoneInfos[i].Name == BoneName)
			return i;

	return -1;
}