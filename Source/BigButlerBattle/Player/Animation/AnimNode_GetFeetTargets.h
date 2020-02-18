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

	FFeetTransform(const FTransform& left, const FTransform& right)
	 : Left{left}, Right{right}
	{}
};

USTRUCT(BlueprintType)
struct BIGBUTLERBATTLE_API FAnimNode_GetFeetTargets : public FAnimNode_Base
{
    GENERATED_BODY()

    /** Base Pose - This Can Be Entire Anim Graph Up To This Point, or Any Combination of Other Nodes*/
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Links)
    FComponentSpacePoseLink Pose;

public:
    // FAnimNode_Base interface
    virtual void Initialize_AnyThread(const FAnimationInitializeContext &Context) override;
    virtual void CacheBones_AnyThread(const FAnimationCacheBonesContext &Context) override;
    virtual void Update_AnyThread(const FAnimationUpdateContext &Context) override;
    virtual void EvaluateComponentSpace_AnyThread(FComponentSpacePoseContext& Output) override;
    // End of FAnimNode_Base interface


	/**
	 * Returns the locations of the skateboard feet sockets in world space.
	 */
	FFeetTransform GetSkateboardFeetTransform(const USkeletalMeshComponent& skateboardMesh) const;
	/**
	 * Returns the locations of the skateboard feet sockets in component space.
	 */
	FFeetTransform GetComponentSkateboardFeetTransform(const USkeletalMeshComponent& skateboardMesh) const;

	FFeetTransform GetSkateboardFeetTransformInButlerSpace(const USkeletalMeshComponent& butlerMesh, const USkeletalMeshComponent& skateboardMesh) const;

	FTransform GetLocalSkateboardToButlerTransform(const USkeletalMeshComponent& butlerMesh, const USkeletalMeshComponent& skateboardMesh) const;
	FTransform GetLocalButlerToSkateboardTransform(const USkeletalMeshComponent& butlerMesh, const USkeletalMeshComponent& skateboardMesh) const;

protected:
    FRotator GetSkateboardRotationOffset(const USkeletalMeshComponent& skateboardMesh) const;

    FVector GetFootLocation(const FVector& socketPos, FCSPose<FCompactPose>& pose, const USkeletalMeshComponent& skateboardMesh, bool left = true) const;
    FVector GetFootLocation(const FVector& socketPos, FCSPose<FCompactPose>& pose, FQuat feetRotationOffset, bool left = true) const;

    FVector GetSocketPos(const USkeletalMeshComponent& butlerMesh, const USkeletalMeshComponent& skateboardMesh, bool left = true) const;

};