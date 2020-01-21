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

protected:
	void NativeBeginPlay() override;

	void NativeUpdateAnimation(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector LeftFootTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector RightFootTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector LeftLegJointLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector RightLegJointLocation;

	// UFUNCTION(BlueprintPure)
	// TPair<FVector, FVector> GetFeetLocations() const;
	
	UFUNCTION(BlueprintPure)
	FVector GetFootLeftLocation(APlayerCharacter* character) const;

	UFUNCTION(BlueprintPure)
	FVector GetFootRightLocation(APlayerCharacter* character) const;

	UFUNCTION(BlueprintPure)
	FVector GetLeftLegJointLocation(APlayerCharacter* character) const;

	UFUNCTION(BlueprintPure)
	FVector GetRightLegJointLocation(APlayerCharacter* character) const;

private:
	FQuat PelvisStartRotation;
	FQuat LeftFootStartRotation;
	FQuat RightFootStartRotation;
};
