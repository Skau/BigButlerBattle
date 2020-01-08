// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PlayerCharacterMovementComponent.generated.h"

// Forward declarations
class APlayerCharacter;

UENUM(BlueprintType)
enum class ECustomMovementType : uint8
{
	MOVE_None			UMETA(DisplayName = "None"),
	MOVE_Skateboard		UMETA(DisplayName = "Skateboard")
};

/** Custom override of movement component
 * 
 */
UCLASS()
class BIGBUTLERBATTLE_API UPlayerCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Custom Movement")
	ECustomMovementType CurrentCustomMovementMode = ECustomMovementType::MOVE_Skateboard;

	bool bStandstill = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Skateboard Movement", meta = (DisplayName = "Standstill Threshold", ClampMin = "0", UIMin = "0"))
	float StandstillThreshold = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Skateboard Movement", meta = (DisplayName = "Forward Ground Deceleration", ClampMin = "0", UIMin = "0"))
	float SkateboardForwardGroundDeceleration = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Skateboard Movement", meta = (DisplayName = "Right Ground Deceleration", ClampMin = "0", UIMin = "0"))
	float SkateboardRightRollingGroundDeceleration = 4096.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Skateboard Movement", meta = (DisplayName = "Braking Deceleration", ClampMin = "0", UIMin = "0"))
	float SkateboardBreakingDeceleration = 1024.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Skateboard Movement", meta = (DisplayName = "Rotation Speed", ClampMin = "0", UIMin = "0"))
	float SkateboardRotationSpeed = 200.f;



public:
	UPlayerCharacterMovementComponent();

	UFUNCTION(BlueprintPure)
	bool IsStandstill() const { return bStandstill; }

	bool IsMovingOnGround() const override;

protected:
	void BeginPlay() override;

	void TickComponent(float deltaTime, enum ELevelTick TickType, FActorComponentTickFunction* thisTickFunction) override;

	/** @note Movement update functions should only be called through StartNewPhysics()*/
	void PhysCustom(float deltaTime, int32 Iterations) override;

	void PhysSkateboard(float deltaTime, int32 Iterations);

	void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

private:
	FVector InputDir;

	void CalcSkateboardVelocity(float DeltaTime);

	FORCEINLINE float GetRotationInput() const { return InputDir.Y; }
	FORCEINLINE float GetForwardInput() const { return InputDir.X; }
	FORCEINLINE FVector GetRightInput() const { return FVector{ 0, InputDir.Y, 0 }; }
	/** Calculates the forward/backwards acceleration in world space.
	 * @brief Calculates the forward/backwards acceleration in world space.
	 * @return Forward / backwards acceleration vector in world space.
	 */
	FORCEINLINE FVector CalcAcceleration() const;
	FORCEINLINE float CalcRotation() const;

	FVector ClampForwardVelocity();
};
