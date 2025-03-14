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

    auto AnimInstance = Cast<UCharacterAnimInstance>(Output.AnimInstanceProxy->GetAnimInstanceObject());
    if (IsValid(AnimInstance) && (AnimInstance->bLeftLegIK || AnimInstance->bRightLegIK))
    {
        const auto Character = Cast<APlayerCharacter>(AnimInstance->TryGetPawnOwner());
        if (IsValid(Character))
        {
            const auto& ButlerMesh = Character->GetMesh();
            const auto& SkateboardMesh = Character->GetSkateboardMesh();

            const bool bMeshValid = IsValid(ButlerMesh) && IsValid(SkateboardMesh);
            
            // Get rotation offset
            const auto RotationOffset = AnimInstance->SkateboardRotationOffset = bMeshValid ? GetSkateboardRotationOffset(*SkateboardMesh) : FRotator{};


            // Get left and right foot targets
            if (AnimInstance->bLeftLegIK)
                AnimInstance->LeftFootTarget = GetFootLocation(bMeshValid ? GetSocketPos(*ButlerMesh, *SkateboardMesh, true) : FVector{}, Output.Pose, RotationOffset.Quaternion(), true);

            if (AnimInstance->bRightLegIK)
                AnimInstance->RightFootTarget = GetFootLocation(bMeshValid ? GetSocketPos(*ButlerMesh, *SkateboardMesh, false) : FVector{}, Output.Pose, RotationOffset.Quaternion(), false);


            // Get left and right joint targets
            if (AnimInstance->bLeftLegIK)
                AnimInstance->LeftLegJointLocationFinal = GetKneeLocation(Output.Pose, true);
                
            if (AnimInstance->bRightLegIK)
                AnimInstance->RightLegJointLocationFinal = GetKneeLocation(Output.Pose, false);
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
                                                                   const USkeletalMeshComponent& SkateboardMesh) const
-> FTransform
{
    return SkateboardMesh.GetComponentTransform().GetRelativeTransformReverse(ButlerMesh.GetComponentTransform());
}

FRotator FAnimNode_GetFeetTargets::GetSkateboardRotationOffset(const USkeletalMeshComponent& SkateboardMesh) const
{
    return SkateboardMesh.GetRelativeRotation();
}

FVector FAnimNode_GetFeetTargets::GetFootLocation(const FVector& SocketPos, FCSPose<FCompactPose>& PoseIn, const USkeletalMeshComponent& SkateboardMesh, const bool bLeft) const
{
    return GetFootLocation(SocketPos, PoseIn, GetSkateboardRotationOffset(SkateboardMesh).Quaternion(), bLeft);
}

FVector FAnimNode_GetFeetTargets::GetFootLocation(const FVector& SocketPos, FCSPose<FCompactPose>& PoseIn, const FQuat& FeetRotationOffset, const bool bLeft) const
{
	const auto BoneContainer = PoseIn.GetPose().GetBoneContainer();

    const FName FootBone{bLeft ? "ball_l" : "ball_r"};
    const FName AnkleBone{bLeft ? "foot_l" : "foot_r"};

    // Get foot bone transform
    FCompactPoseBoneIndex CompactBoneIndex = BoneContainer.GetCompactPoseIndexFromSkeletonIndex(BoneContainer.GetPoseBoneIndexForBoneName(FootBone));
    const auto FootPos = PoseIn.GetComponentSpaceTransform(CompactBoneIndex).GetTranslation();

    // Get ankle bone transform
    CompactBoneIndex = BoneContainer.GetCompactPoseIndexFromSkeletonIndex(BoneContainer.GetPoseBoneIndexForBoneName(AnkleBone));
    const auto AnklePos = PoseIn.GetComponentSpaceTransform(CompactBoneIndex).GetTranslation();
    
    // Get delta direction vector
    auto FootDir = FootPos - AnklePos;
    // Rotate direction vector with displacement rotation.
    FootDir = FeetRotationOffset * FootDir;
    return SocketPos - FootDir;
}

FVector FAnimNode_GetFeetTargets::GetKneeLocation(FCSPose<FCompactPose>& PoseIn, const bool bLeft) const
{
    const auto BoneContainer = PoseIn.GetPose().GetBoneContainer();

    const FName KneeBone{bLeft ? "calf_l" : "calf_r"};
    const FName ThighBone{bLeft ? "thigh_l" : "thigh_r"};

    // Get foot bone transform
    FCompactPoseBoneIndex CompactBoneIndex = BoneContainer.GetCompactPoseIndexFromSkeletonIndex(BoneContainer.GetPoseBoneIndexForBoneName(ThighBone));
    const auto ThighPos = PoseIn.GetComponentSpaceTransform(CompactBoneIndex).GetTranslation();

    // Get ankle bone transform
    CompactBoneIndex = BoneContainer.GetCompactPoseIndexFromSkeletonIndex(BoneContainer.GetPoseBoneIndexForBoneName(KneeBone));
    const auto KneePos = PoseIn.GetComponentSpaceTransform(CompactBoneIndex).GetTranslation();

    const auto KneeDir = KneePos - ThighPos;
    return KneeDir.GetSafeNormal2D() * 100.f;
}

FVector FAnimNode_GetFeetTargets::GetSocketPos(const USkeletalMeshComponent& ButlerMesh, const USkeletalMeshComponent& SkateboardMesh, const bool bLeft) const
{
    auto Trans = bLeft ? GetComponentSkateboardFeetTransform(SkateboardMesh).Left : GetComponentSkateboardFeetTransform(SkateboardMesh).Right;
    Trans = Trans * GetLocalSkateboardToButlerTransform(ButlerMesh, SkateboardMesh);
    return Trans.GetTranslation();
}
