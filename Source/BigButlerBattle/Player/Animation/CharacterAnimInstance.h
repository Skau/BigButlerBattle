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

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector LeftFootTarget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector RightFootTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector LeftLegJointLocation = FVector{50.f, 100.f, 0.f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector RightLegJointLocation = FVector{-50.f, 100.f, 0.f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator SkateboardRotationOffset;

protected:
	void NativeBeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim montages")
	UAnimMontage* JumpMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim montages")
	UAnimMontage* ForwardMontage;
};
