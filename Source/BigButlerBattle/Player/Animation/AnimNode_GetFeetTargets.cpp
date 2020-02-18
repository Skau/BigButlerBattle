// Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.

#include "AnimNode_GetFeetTargets.h"
#include "Animation/AnimInstanceProxy.h"
#include "CharacterAnimInstance.h"
#include "Player/PlayerCharacter.h"
#include "BonePose.h"

void FAnimNode_GetFeetTargets::Initialize_AnyThread(const FAnimationInitializeContext &Context)
{
    //Init the Inputs
    Pose.Initialize(Context);
}

void FAnimNode_GetFeetTargets::CacheBones_AnyThread(const FAnimationCacheBonesContext &Context)
{
    Pose.CacheBones(Context);
    
}
void FAnimNode_GetFeetTargets::Update_AnyThread(const FAnimationUpdateContext &Context)
{
    //***************************************
    // Evaluate Graph, see AnimNode_Base, AnimNodeBase.h
    GetEvaluateGraphExposedInputs().Execute(Context);
    //***************************************

    //************************************************
    // FPoseLinkBase::Update Active Pose - this is what makes
    //the glowing line thing happen and animations loop
    //***********************************************
    Pose.Update(Context);
}

void FAnimNode_GetFeetTargets::EvaluateComponentSpace_AnyThread(FComponentSpacePoseContext &Output)
{
    // Return Base Pose, Un Modified
    Pose.EvaluateComponentSpace(Output);

    auto animInstance = Cast<UCharacterAnimInstance>(Output.AnimInstanceProxy->GetAnimInstanceObject());
    if (IsValid(animInstance))
    {
        auto character = Cast<APlayerCharacter>(animInstance->TryGetPawnOwner());
        if (IsValid(character))
        {
            const auto& butlerMesh = character->GetMesh();
            const auto& skateboardMesh = character->GetSkateboardMesh();

            const bool bMeshValid = IsValid(butlerMesh) && IsValid(skateboardMesh);
            
            // Get rotation offset
            auto rotationOffset = animInstance->SkateboardRotationOffset = bMeshValid ? GetSkateboardRotationOffset(*skateboardMesh) : FRotator{};

            // Get left and right foot targets
            animInstance->LeftFootTarget = GetFootLocation(bMeshValid ? GetSocketPos(*butlerMesh, *skateboardMesh, true) : FVector{}, Output.Pose, rotationOffset.Quaternion(), true);
            animInstance->RightFootTarget = GetFootLocation(bMeshValid ? GetSocketPos(*butlerMesh, *skateboardMesh, false) : FVector{}, Output.Pose, rotationOffset.Quaternion(), false);
        }
    }
}

FFeetTransform FAnimNode_GetFeetTargets::GetSkateboardFeetTransform(const USkeletalMeshComponent& skateboardMesh) const
{
	return {skateboardMesh.GetSocketTransform("FootLeft"), skateboardMesh.GetSocketTransform("FootRight")};
}

FFeetTransform FAnimNode_GetFeetTargets::GetComponentSkateboardFeetTransform(const USkeletalMeshComponent& skateboardMesh) const
{
	return {
		FTransform{
			FRotator{},
			skateboardMesh.GetSocketTransform("FootLeft", ERelativeTransformSpace::RTS_Component).GetTranslation(),
			FVector{1.f, 1.f, 1.f}
		},
		FTransform{
			FRotator{},
			skateboardMesh.GetSocketTransform("FootRight", ERelativeTransformSpace::RTS_Component).GetTranslation(),
			FVector{1.f, 1.f, 1.f}
		}
	};
}

FFeetTransform FAnimNode_GetFeetTargets::GetSkateboardFeetTransformInButlerSpace(const USkeletalMeshComponent& butlerMesh, const USkeletalMeshComponent& skateboardMesh) const
{
	auto feetTrans = GetComponentSkateboardFeetTransform(skateboardMesh);	
	return {feetTrans.Left * GetLocalSkateboardToButlerTransform(butlerMesh, skateboardMesh), feetTrans.Right * GetLocalSkateboardToButlerTransform(butlerMesh, skateboardMesh)};
}

FTransform FAnimNode_GetFeetTargets::GetLocalSkateboardToButlerTransform(const USkeletalMeshComponent& butlerMesh, const USkeletalMeshComponent& skateboardMesh) const
{
	return skateboardMesh.GetComponentTransform().GetRelativeTransform(butlerMesh.GetComponentTransform());
}

FTransform FAnimNode_GetFeetTargets::GetLocalButlerToSkateboardTransform(const USkeletalMeshComponent& butlerMesh, const USkeletalMeshComponent& skateboardMesh) const
{
    return skateboardMesh.GetComponentTransform().GetRelativeTransformReverse(butlerMesh.GetComponentTransform());
}

FRotator FAnimNode_GetFeetTargets::GetSkateboardRotationOffset(const USkeletalMeshComponent& skateboardMesh) const
{
    return skateboardMesh.GetRelativeRotation();
}

FVector FAnimNode_GetFeetTargets::GetFootLocation(const FVector& socketPos, FCSPose<FCompactPose>& pose, const USkeletalMeshComponent& skateboardMesh, bool left) const
{
    return GetFootLocation(socketPos, pose, GetSkateboardRotationOffset(skateboardMesh).Quaternion(), left);
}

FVector FAnimNode_GetFeetTargets::GetFootLocation(const FVector& socketPos, FCSPose<FCompactPose>& pose, FQuat feetRotationOffset, bool left) const
{
    auto boneContainer = pose.GetPose().GetBoneContainer();

    FName footBone{left ? "ball_l" : "ball_r"};
    FName ankleBone{left ? "foot_l" : "foot_r"};

    // Get foot bone transform
    FCompactPoseBoneIndex compactBoneIndex = boneContainer.GetCompactPoseIndexFromSkeletonIndex(boneContainer.GetPoseBoneIndexForBoneName(footBone));
    auto footPos = pose.GetComponentSpaceTransform(compactBoneIndex).GetTranslation();

    // Get ankle bone transform
    compactBoneIndex = boneContainer.GetCompactPoseIndexFromSkeletonIndex(boneContainer.GetPoseBoneIndexForBoneName(ankleBone));
    auto anklePos = pose.GetComponentSpaceTransform(compactBoneIndex).GetTranslation();
    
    // Get delta direction vector
    auto footDir = footPos - anklePos;
    // Rotate direction vector with displacement rotation.
    footDir = feetRotationOffset * footDir;
    return socketPos - footDir;
}

FVector FAnimNode_GetFeetTargets::GetSocketPos(const USkeletalMeshComponent& butlerMesh, const USkeletalMeshComponent& skateboardMesh, bool left) const
{
    auto trans = left ? GetComponentSkateboardFeetTransform(skateboardMesh).Left : GetComponentSkateboardFeetTransform(skateboardMesh).Right;
    trans = trans * GetLocalSkateboardToButlerTransform(butlerMesh, skateboardMesh);
    return trans.GetTranslation();
}
