// Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.

#include "AnimNode_GetFeetTargets.h"
#include "Animation/AnimInstanceProxy.h"
#include "CharacterAnimInstance.h"
#include "Player/PlayerCharacter.h"

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

    auto animInstance = Cast<UCharacterAnimInstance>(Context.AnimInstanceProxy->GetAnimInstanceObject());
    if (IsValid(animInstance))
    {
        auto character = Cast<APlayerCharacter>(animInstance->TryGetPawnOwner());
        if (IsValid(character))
        {
            animInstance->Skateboard    RotationOffset = animInstance->GetSkateboardRotationOffset(character);

            animInstance->LeftFootTarget = animInstance->GetFootLocation(character, animInstance->SkateboardRotationOffset.Quaternion(), true);
            animInstance->RightFootTarget = animInstance->GetFootLocation(character, animInstance->SkateboardRotationOffset.Quaternion(), false);
        }
    }

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

    //Evaluate is returning the Output to this function,
    //which is returning the Output to the rest of the Anim Graph

    //In this case, we are passing the Output out variable into the Pose

    //Basically saying, give us back the unmodified Base Pose

    //i.e, the bulk of your anim tree.
}

// void FAnimNode_GetFeetTargets::EvaluateComponentPose_AnyThread(FComponentSpacePoseContext& Output)
// {
//     Super::EvaluateComponentPose_AnyThread(Output);

//     UE_LOG(LogTemp, Warning, TEXT("Hello animation world!"));
// }
