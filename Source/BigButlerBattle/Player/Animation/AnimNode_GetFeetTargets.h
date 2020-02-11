// Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.

#pragma once
#include "Animation/AnimNodeBase.h"
#include "BoneControllers/AnimNode_SkeletalControlBase.h"
#include "AnimNode_GetFeetTargets.generated.h"

USTRUCT(BlueprintType)
struct BIGBUTLERBATTLE_API FAnimNode_GetFeetTargets : public FAnimNode_Base
{
    GENERATED_BODY()

    //FPoseLink - this can be any combination
    //of other nodes, not just animation sequences
    //	so you could have an blend space leading into
    //a layer blend per bone to just use the arm
    //	and then pass that into the PoseLink

    /** Base Pose - This Can Be Entire Anim Graph Up To This Point, or Any Combination of Other Nodes*/
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Links)
    FComponentSpacePoseLink Pose;

    // FAnimNode_Base interface
public:
    // FAnimNode_Base interface
    virtual void Initialize_AnyThread(const FAnimationInitializeContext &Context) override;
    virtual void CacheBones_AnyThread(const FAnimationCacheBonesContext &Context) override;
    virtual void Update_AnyThread(const FAnimationUpdateContext &Context) override;
    virtual void EvaluateComponentSpace_AnyThread(FComponentSpacePoseContext& Output) override;
    // End of FAnimNode_Base interface

// protected:
//     virtual void EvaluateComponentPose_AnyThread(FComponentSpacePoseContext& Output);

};