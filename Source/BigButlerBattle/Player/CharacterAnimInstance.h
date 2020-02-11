// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CharacterAnimInstance.generated.h"

class APlayerCharacter;
class UPlayerCharacterMovementComponent;

/**
 * 
 */
UCLASS()
class BIGBUTLERBATTLE_API UCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	void JumpAnim();

	void ForwardKick();

protected:
	void NativeBeginPlay() override;

	void NativeUpdateAnimation(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector LeftFootTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector RightFootTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector LeftLegJointLocation = FVector{50.f, 100.f, 0.f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector RightLegJointLocation = FVector{-50.f, 100.f, 0.f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator SkateboardRotationOffset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim montages")
	UAnimMontage* JumpMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim montages")
	UAnimMontage* ForwardMontage;

	/**
	 * Whether the root bone of the skeleton hierarchy is scaled by 100
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bSkeletonScaledByHundred = false;

	// UFUNCTION(BlueprintPure)
	// TPair<FVector, FVector> GetFeetLocations() const;


	UFUNCTION(BlueprintCallable, meta = (NotBlueprintThreadSafe))
	FVector GetLeftFootTarget() const;

	UFUNCTION(BlueprintCallable, meta = (NotBlueprintThreadSafe))
	FVector GetRightFootTarget() const;
	FVector GetFootLocation(APlayerCharacter* character, bool left = true) const;
	FVector GetFootLocation(APlayerCharacter* character, FQuat feetRotationOffset, bool left = true) const;

	FRotator GetSkateboardRotationOffset(APlayerCharacter* character) const;

	FTransform GetSkeletonRefBoneTransform(FName BoneName) const;
	bool GenerateRefSkeletonBoneTransforms(APlayerCharacter* character);
	int32 GetRefBoneIndex(FName BoneName) const;

private:
	FQuat PelvisStartRotation;
	FQuat LeftFootStartRotation;
	FQuat RightFootStartRotation;

	float PelvisStartYawOffset;
	float LeftFootStartYawOffset;
	float RightFootStartYawOffset;
	float LeftKneeStartYawOffset;
	float RightKneeStartYawOffset;

	// List of all reference bone transforms in component space
	TArray<FTransform> RefSkeletonBoneTransforms;
	TArray<FMeshBoneInfo> RefSkeletonBoneInfos;
};
