// Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.

#pragma once
#include "Animation/AnimNodeBase.h"
#include "BoneControllers/AnimNode_SkeletalControlBase.h"
#include "AnimNode_GetFeetTargets.generated.h"

class APlayerCharacter;

struct FFeetTransform
{
	FTransform Left;
	FTransform Right;

	FFeetTransform(const FTransform& Left, const FTransform& Right)
	 : Left{Left}, Right{Right}
	{}
};

USTRUCT(BlueprintType)
struct BIGBUTLERBATTLE_API FAnimNode_GetFeetTargets : public FAnimNode_Base
{
    GENERATED_BODY()

    /** Base Pose - This Can Be Entire Anim Graph Up To This Point, or Any Combination of Other Nodes*/
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Links)
    FComponentSpacePoseLink Pose;

    // FAnimNode_Base interface
    void Initialize_AnyThread(const FAnimationInitializeContext &Context) override;
    void CacheBones_AnyThread(const FAnimationCacheBonesContext &Context) override;
    void Update_AnyThread(const FAnimationUpdateContext &Context) override;
    void EvaluateComponentSpace_AnyThread(FComponentSpacePoseContext& Output) override;
    // End of FAnimNode_Base interface


	/**
	 * Returns the locations of the skateboard feet sockets in world space.
	 */
	FFeetTransform GetSkateboardFeetTransform(const USkeletalMeshComponent& SkateboardMesh) const;
	/**
	 * Returns the locations of the skateboard feet sockets in component space.
	 */
	FFeetTransform GetComponentSkateboardFeetTransform(const USkeletalMeshComponent& SkateboardMesh) const;

	FFeetTransform GetSkateboardFeetTransformInButlerSpace(const USkeletalMeshComponent& ButlerMesh, const USkeletalMeshComponent& SkateboardMesh) const;

	FTransform GetLocalSkateboardToButlerTransform(const USkeletalMeshComponent& ButlerMesh, const USkeletalMeshComponent& SkateboardMesh) const;
	FTransform GetLocalButlerToSkateboardTransform(const USkeletalMeshComponent& ButlerMesh, const USkeletalMeshComponent& SkateboardMesh) const;

protected:
    FRotator GetSkateboardRotationOffset(const USkeletalMeshComponent& SkateboardMesh) const;

    FVector GetFootLocation(const FVector& SocketPos, FCSPose<FCompactPose>& PoseIn, const USkeletalMeshComponent& SkateboardMesh, bool bLeft
	                            = true) const;
    FVector GetFootLocation(const FVector& SocketPos, FCSPose<FCompactPose>& PoseIn, const FQuat& FeetRotationOffset, bool bLeft = true) const;

	FVector GetKneeLocation(FCSPose<FCompactPose>& PoseIn, bool bLeft = true) const;

    FVector GetSocketPos(const USkeletalMeshComponent& ButlerMesh, const USkeletalMeshComponent& SkateboardMesh, bool bLeft = true) const;

};