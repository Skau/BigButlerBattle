// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "SkateboardAnimInstance.generated.h"

// Forward declarations
class UPlayerCharacterMovementComponent;
class UAnimMontage;

/**
 * 
 */
UCLASS() class BIGBUTLERBATTLE_API USkateboardAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float InputRotation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bInAir;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Velocity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsStandstill;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float VelocityThreshold = 100.f;

	UPROPERTY(BlueprintReadOnly)
	UPlayerCharacterMovementComponent* MovementComponent = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim montages")
	UAnimMontage* JumpMontage;

	void NativeBeginPlay() override;

	void NativeUpdateAnimation(float DeltaTime) override;

	UFUNCTION()
	void JumpAnim();

	UFUNCTION(BlueprintPure)
	float GetWheelPlaybackRate() const;

	UFUNCTION(BlueprintPure)
	float GetFlipAmount() const;

	UFUNCTION(BlueprintPure)
	float GetRotationAmount() const;
};
