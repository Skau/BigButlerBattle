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

    // //Try Again if not found
    // if (!OwningActor)
    //     OwningActor =
    //         Context.AnimInstanceProxy->GetSkelMeshComponent()->GetOwner();

    // //Not found
    // if (!OwningActor)
    // {
    //     UE_LOG(LogAnimation, Warning,
    //             TEXT("FAnimNode_GetFeetTargets::Update() Owning Actor was not found"));
    //     return;
    //     //~
    // }

    //Do Stuff Based On Actor Owner

    // Do Updates

    //Try To Update As Few of the Inputs As You Can

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
            animInstance->SkateboardRotationOffset = GetSkateboardRotationOffset(character);

            animInstance->LeftFootTarget = GetFootLocation(character, Output.Pose, animInstance->SkateboardRotationOffset.Quaternion(), true);
            animInstance->RightFootTarget = GetFootLocation(character, Output.Pose, animInstance->SkateboardRotationOffset.Quaternion(), false);
        }
    }

    
    // Output.AnimInstanceProxy->GetSkelMeshComponent()->GetBoneIndex()

    //Evaluate is returning the Output to this function,
    //which is returning the Output to the rest of the Anim Graph

    //In this case, we are passing the Output out variable into the Pose

    //Basically saying, give us back the unmodified Base Pose

    //i.e, the bulk of your anim tree.
}

FRotator FAnimNode_GetFeetTargets::GetSkateboardRotationOffset(APlayerCharacter *character)
{
    return IsValid(character) ? character->GetSkateboardRotation() : FRotator{};
}

FVector FAnimNode_GetFeetTargets::GetFootLocation(APlayerCharacter *character, FCSPose<FCompactPose>& pose, bool left)
{
    return GetFootLocation(character, pose, GetSkateboardRotationOffset(character).Quaternion(), left);
}

FVector FAnimNode_GetFeetTargets::GetFootLocation(APlayerCharacter *character, FCSPose<FCompactPose>& pose, FQuat feetRotationOffset, bool left)
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
    // socketPos += FVector{10.f, 0.f, 0.f};
    // Scaled by 100 since skeleton is scaled by 100.
    socketPos -= footDir;
    return socketPos;
}
