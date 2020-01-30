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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim montages")
	UAnimMontage* JumpMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim montages")
	UAnimMontage* ForwardMontage;

	// UFUNCTION(BlueprintPure)
	// TPair<FVector, FVector> GetFeetLocations() const;
	
	UFUNCTION(BlueprintPure)
	FVector GetFootLeftLocation(APlayerCharacter* character) const;

	UFUNCTION(BlueprintPure)
	FVector GetFootRightLocation(APlayerCharacter* character) const;

	UFUNCTION()
	void JumpAnim();

private:
	FQuat PelvisStartRotation;
	FQuat LeftFootStartRotation;
	FQuat RightFootStartRotation;

	float PelvisStartYawOffset;
	float LeftFootStartYawOffset;
	float RightFootStartYawOffset;
	float LeftKneeStartYawOffset;
	float RightKneeStartYawOffset;
};
