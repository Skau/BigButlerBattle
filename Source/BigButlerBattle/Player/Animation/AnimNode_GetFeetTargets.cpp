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
            const auto& ButlerMesh = character->GetMesh();
            const auto& SkateboardMesh = character->GetSkateboardMesh();

            const bool bMeshValid = IsValid(ButlerMesh) && IsValid(SkateboardMesh);
            
            // Get rotation offset
            const auto RotationOffset = animInstance->SkateboardRotationOffset = bMeshValid ? GetSkateboardRotationOffset(*SkateboardMesh) : FRotator{};

            // Get left and right foot targets
            animInstance->LeftFootTarget = GetFootLocation(bMeshValid ? GetSocketPos(*ButlerMesh, *SkateboardMesh, true) : FVector{}, Output.Pose, RotationOffset.Quaternion(), true);
            animInstance->RightFootTarget = GetFootLocation(bMeshValid ? GetSocketPos(*ButlerMesh, *SkateboardMesh, false) : FVector{}, Output.Pose, RotationOffset.Quaternion(), false);
        }
    }
}

FFeetTransform FAnimNode_GetFeetTargets::GetSkateboardFeetTransform(const USkeletalMeshComponent& SkateboardMesh) const
{
	return {SkateboardMesh.GetSocketTransform("FootLeft"), SkateboardMesh.GetSocketTransform("FootRight")};
}

FFeetTransform FAnimNode_GetFeetTargets::GetComponentSkateboardFeetTransform(const USkeletalMeshComponent& SkateboardMesh) const
{
	return {
		FTransform{
			FRotator{},
			SkateboardMesh.GetSocketTransform("FootLeft", ERelativeTransformSpace::RTS_Component).GetTranslation(),
			FVector{1.f, 1.f, 1.f}
		},
		FTransform{
			FRotator{},
			SkateboardMesh.GetSocketTransform("FootRight", ERelativeTransformSpace::RTS_Component).GetTranslation(),
			FVector{1.f, 1.f, 1.f}
		}
	};
}

FFeetTransform FAnimNode_GetFeetTargets::GetSkateboardFeetTransformInButlerSpace(const USkeletalMeshComponent& ButlerMesh, const USkeletalMeshComponent& SkateboardMesh) const
{
	const auto FeetTrans = GetComponentSkateboardFeetTransform(SkateboardMesh);	
	return {FeetTrans.Left * GetLocalSkateboardToButlerTransform(ButlerMesh, SkateboardMesh), FeetTrans.Right * GetLocalSkateboardToButlerTransform(ButlerMesh, SkateboardMesh)};
}

FTransform FAnimNode_GetFeetTargets::GetLocalSkateboardToButlerTransform(const USkeletalMeshComponent& ButlerMesh, const USkeletalMeshComponent& SkateboardMesh) const
{
	return SkateboardMesh.GetComponentTransform().GetRelativeTransform(ButlerMesh.GetComponentTransform());
}

auto FAnimNode_GetFeetTargets::GetLocalButlerToSkateboardTransform(const USkeletalMeshComponent& ButlerMesh,
                                                                   const USkeletalMeshComponent& skateboardMesh) const
-> FTransform
{
    return skateboardMesh.GetComponentTransform().GetRelativeTransformReverse(ButlerMesh.GetComponentTransform());
}

FRotator FAnimNode_GetFeetTargets::GetSkateboardRotationOffset(const USkeletalMeshComponent& SkateboardMesh) const
{
    return SkateboardMesh.GetRelativeRotation();
}

FVector FAnimNode_GetFeetTargets::GetFootLocation(const FVector& SocketPos, FCSPose<FCompactPose>& PoseIn, const USkeletalMeshComponent& SkateboardMesh, const bool& bLeft) const
{
    return GetFootLocation(SocketPos, PoseIn, GetSkateboardRotationOffset(SkateboardMesh).Quaternion(), bLeft);
}

FVector FAnimNode_GetFeetTargets::GetFootLocation(const FVector& SocketPos, FCSPose<FCompactPose>& PoseIn, const FQuat& FeetRotationOffset, const bool& bLeft) const
{
	const auto BoneContainer = PoseIn.GetPose().GetBoneContainer();

    const FName FootBone{bLeft ? "ball_l" : "ball_r"};
    const FName AnkleBone{bLeft ? "foot_l" : "foot_r"};

    // Get foot bone transform
    FCompactPoseBoneIndex compactBoneIndex = BoneContainer.GetCompactPoseIndexFromSkeletonIndex(BoneContainer.GetPoseBoneIndexForBoneName(FootBone));
    const auto FootPos = PoseIn.GetComponentSpaceTransform(compactBoneIndex).GetTranslation();

    // Get ankle bone transform
    compactBoneIndex = BoneContainer.GetCompactPoseIndexFromSkeletonIndex(BoneContainer.GetPoseBoneIndexForBoneName(AnkleBone));
    const auto AnklePos = PoseIn.GetComponentSpaceTransform(compactBoneIndex).GetTranslation();
    
    // Get delta direction vector
    auto FootDir = FootPos - AnklePos;
    // Rotate direction vector with displacement rotation.
    FootDir = FeetRotationOffset * FootDir;
    return SocketPos - FootDir;
}

FVector FAnimNode_GetFeetTargets::GetSocketPos(const USkeletalMeshComponent& ButlerMesh, const USkeletalMeshComponent& SkateboardMesh, const bool& bLeft) const
{
    auto Trans = bLeft ? GetComponentSkateboardFeetTransform(SkateboardMesh).Left : GetComponentSkateboardFeetTransform(SkateboardMesh).Right;
    Trans = Trans * GetLocalSkateboardToButlerTransform(ButlerMesh, SkateboardMesh);
    return Trans.GetTranslation();
}
