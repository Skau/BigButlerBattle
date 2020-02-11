// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PlayerCharacterMovementComponent.generated.h"

// Forward declarations
class APlayerCharacter;
class USplineComponent;

UENUM(BlueprintType)
enum class ECustomMovementType : uint8
{
	MOVE_None			UMETA(DisplayName = "None"),
	MOVE_Skateboard		UMETA(DisplayName = "Skateboard"),
	MOVE_Grinding		UMETA(DisplayName = "Grinding")
};

/** Custom override of movement component
 * 
 */
UCLASS(hideCategories=("Character Movement: Walking", "Character Movement: Swimming", "Character Movement (Networking)"))
class BIGBUTLERBATTLE_API UPlayerCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	float GetMaxAccelerationVelocity() { return CustomMaxAccelerationVelocity; }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Custom Movement")
	ECustomMovementType CurrentCustomMovementMode = ECustomMovementType::MOVE_Skateboard;

	/**
	 * Max velocity to add input acceleration to. If velocity is higher, only acceleration from terrain get's applied.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Custom Movement", meta = (DisplayName = "Max Acceleration Velocity"))
	float CustomMaxAccelerationVelocity = 3600.f;

	bool bStandstill = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Skateboard Movement", meta = (DisplayName = "Standstill Threshold", ClampMin = "0", UIMin = "0"))
	float StandstillThreshold = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Skateboard Movement", meta = (DisplayName = "Forward Ground Deceleration", ClampMin = "0", UIMin = "0"))
	float SkateboardForwardGroundDeceleration = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Skateboard Movement", meta = (DisplayName = "Sideways Ground Deceleration", ClampMin = "0", UIMin = "0"))
	float SkateboardSidewaysGroundDeceleration = 4096.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Skateboard Movement", meta = (DisplayName = "Braking Deceleration", ClampMin = "0", UIMin = "0"))
	float SkateboardBreakingDeceleration = 2600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Skateboard Movement", meta = (DisplayName = "Rotation Speed", ClampMin = "0", UIMin = "0"))
	float SkateboardRotationSpeed = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Skateboard Movement",
		meta = (DisplayName = "Standtill Rotation Factor", ClampMin = "0", UIMin = "0"))
	float SkateboardStandstillRotationSpeed = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Skateboard Movement", meta = (DisplayName = "Slope Gravity Multiplier"))
	float SlopeGravityMultiplier = 2048.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Skateboard Movement", meta = (DisplayName = "Handbrake Rotation"))
	float HandbrakeRotationFactor = 300.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Skateboard Movement", meta = (DisplayName = "Handbrake Velocity Threshold"))
	float HandbrakeVelocityThreshold = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Skateboard Movement", meta = (DisplayName = "Allow Braking While Handbraking?"))
	bool bAllowBrakingWhileHandbraking = false;

	/// Grinding movement:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Grinding Movement", meta = (DisplayName = "Spline Reference"))
	USplineComponent* SkateboardSplineReference;

	float SplinePos = -1.f;
	int SplineDir = 1;

	UPROPERTY(BlueprintReadOnly)
	APlayerCharacter* PlayerCharacter = nullptr;

	FVector InputDir;
	float SidewaysForce = 0.0f;

	bool bBraking = false;

public:
	float bHandbrakeValue = 0.f;

public:
	UPlayerCharacterMovementComponent();

	UFUNCTION(BlueprintPure)
	bool IsStandstill() const { return bStandstill; }

	bool IsMovingOnGround() const override;

	UFUNCTION(BlueprintPure)
	float GetRotationInput() const { return InputDir.Y; }

	/**
	 * Returns true if character is moving forwards and velocity is greater than maxinputacceleration.
	 * Only valid if not braking.
	 */
	bool CanForwardAccelerate(const FVector& AccelerationIn, float DeltaTime) const;
	bool CanForwardAccelerate(const FVector &AccelerationIn, float DeltaTime, bool bMovingBackwards) const;
	/**
	 * Returns true if character is moving forwards and velocity is greater than maxinputacceleration.
	 * Parameter overload that doesn't calculate bMovingBackwards for you.
	 */
	bool CanAccelerate(const FVector &AccelerationIn, bool bBrakingIn, float DeltaTime) const;
	bool CanAccelerate(const FVector &AccelerationIn, bool bBrakingIn, float DeltaTime, bool bMovingBackwards) const;

	/** Calculates the total acceleration in world space.
	 * @brief Calculates the total acceleration in world space.
	 */
	FVector GetInputAcceleration(bool &bBrakingOut, bool &bMovingBackwardsOut, float input = 0.f);

	/**
	 * Multiplies kicking acceleration with a factor to make up for different tick speeds.
	 */
	FVector GetInputAccelerationTimeNormalized(const FVector& a, bool bBrakingIn, float DeltaTime) const;

	/**
	 * Returns GetInputAcceleration but zero-ed out if above max acceleration velocity.
	 */
	FVector GetClampedInputAcceleration(bool &bBreakingOut, float DeltaTime = 0.f, float input = 0.f);

	void HandleImpact(const FHitResult& Hit, float TimeSlice = 0.f, const FVector& MoveDelta = FVector::ZeroVector) override;

protected:
	void BeginPlay() override;

	void TickComponent(float deltaTime, enum ELevelTick TickType, FActorComponentTickFunction* thisTickFunction) override;

	/** @note Movement update functions should only be called through StartNewPhysics()*/
	void PhysCustom(float deltaTime, int32 Iterations) override;

	void PhysSkateboard(float deltaTime, int32 Iterations);
	void PhysGrinding(float deltaTime, int32 Iterations);

	/** Handle falling movement. */
	void PhysFalling(float deltaTime, int32 Iterations) override;

	void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	void ApplySkateboardVelocityBraking(float DeltaTime, float BreakingForwardDeceleration, float BreakingSidewaysDeceleration);

	void PerformSickAssHandbraking(float DeltaTime);

	void UpdateInput() { InputDir = GetPendingInputVector(); }

	void TryFallOff();

	void CalcSkateboardVelocity(const FHitResult &FloorHitResult, float DeltaTime);

	bool IsHandbraking() const { return bHandbrakeValue != 0; }

	FVector GetSlopeAcceleration(const FHitResult &FloorHitResult) const;
	float GetForwardInput() const { return InputDir.X; }
	FVector GetRightInput() const { return FVector{ 0, InputDir.Y, 0 }; }
	float CalcSidewaysBreaking(const FVector& forward) const;
	float CalcRotation() const;
	float CalcHandbrakeRotation() const;
};
