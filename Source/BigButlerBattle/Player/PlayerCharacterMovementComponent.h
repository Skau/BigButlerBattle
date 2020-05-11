// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PlayerCharacterMovementComponent.generated.h"

// Forward declarations
class APlayerCharacter;
class USplineComponent;
class UCurveFloat;

// Enums
UENUM(BlueprintType)
enum class ECustomMovementType : uint8
{
	MOVE_None			UMETA(DisplayName = "None"),
	MOVE_Skateboard		UMETA(DisplayName = "Skateboard"),
	MOVE_Grinding		UMETA(DisplayName = "Grinding")
};

UENUM(BlueprintType)
enum class EGrindingMovementState : uint8
{
	// Note: Manually specifying the 0. enum state to silence warnings in UENUM
	STATE_GodKnowsWhere = 0			UMETA(DisplayName = "God Knows Where"),
	STATE_Entering = 0b0001			UMETA(DisplayName = "Entering"),
	STATE_OnRail = 0b0010			UMETA(DisplayName = "On Rail"),
	STATE_Leaving = 0b0011			UMETA(DisplayName = "Leaving")
};

// Delegates
DECLARE_EVENT_OneParam(UPlayerCharacterMovementComponent, FCustomMovementChangedSignature, uint8);
DECLARE_EVENT_OneParam(UPlayerCharacterMovementComponent, FMovementChangedSignature, EMovementMode);
DECLARE_EVENT_OneParam(UPlayerCharacterMovementComponent, FSplineStateChangedSignature, EGrindingMovementState);

// Structs
struct FSplineInfo
{
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Grinding Movement", meta = (DisplayName = "Spline Reference"))
	USplineComponent* SkateboardSplineReference = nullptr;

	int SplineDir : 2;
	uint8 PlayerState : 2;
	uint8 bClosedLoop : 1;
	uint8 PointCount;

	float SplinePos = -1.f;
	float StartDistanceToCurve;
	FVector StartVelocity;
	float StartVelocitySize; // Cheaper to save length than to do a square root almost every physics frame
	FRotator StartRotation;
	float TravelTime{0.f};

	FSplineStateChangedSignature OnSplineChanged;

	FSplineInfo(USplineComponent* Spline = nullptr, bool bLooped = false);

	bool HasValue() const { return SkateboardSplineReference != nullptr; }

	void SetState(EGrindingMovementState NewState);

	// Helper function to reset curve. Just calls SetState(STATE_leaving);
	void Invalidate();
};

/** Custom override of movement component
 *
 */
UCLASS(hideCategories = ("Character Movement: Walking", "Character Movement: Swimming", "Character Movement (Networking)"),
	   autoExpandCategories = ("Character Movement: Custom Movement", "Character Movement: Skateboard Movement", "Character Movement: Grinding Movement",
							   "Character Movement: Grinding Movement|Start", "Character Movement: Grinding Movement|On Rail", "Character Movement: Skateboard Movement|Air"))
class BIGBUTLERBATTLE_API UPlayerCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	float GetMaxInputSpeed() const { return MaxInputSpeed; }

	bool IsAboveInputSpeed() const { return FMath::Square(MaxInputSpeed) < Velocity.SizeSquared(); }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Custom Movement")
	ECustomMovementType DefaultCustomMovementMode = ECustomMovementType::MOVE_Skateboard;

	/**
	 * Max velocity to add input acceleration to. If velocity is higher, only acceleration from other sources get's applied.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Custom Movement", meta = (DisplayName = "Max Input Speed"))
	float MaxInputSpeed = 2000.f;

	bool bStandstill = false;





	// ================================== Skateboard Movement =================================================
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Skateboard Movement", meta = (DisplayName = "Standstill Threshold", ClampMin = "0", UIMin = "0"))
	float StandstillThreshold = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Skateboard Movement", meta = (DisplayName = "Forward Ground Deceleration", ClampMin = "0", UIMin = "0"))
	float SkateboardForwardGroundDeceleration = 201.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Skateboard Movement")
	bool bApplySeparateDecelerationWhenAboveMaxInputSpeed = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Skateboard Movement", meta = (DisplayName = "Ground Forward Deceleration Above Max Input Speed", EditCondition = "bApplySeparateDecelerationWhenAboveMaxInputSpeed"))
	float SkateboardForwardGroundDecelerationAboveInputSpeed = 1516.f;

	/**
	 * The acceleration force that is applied by kicking, scaled by input strength
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Skateboard Movement", meta = (DisplayName = "Kicking Acceleration"))
	float SkateboardKickingAcceleration = 900.f;

	/**
	 * How much current velocity will have an impact on acceleration
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Skateboard Movement", meta = (DisplayName = "Velocity Acceleration Multiplier", ClampMin = "0", UIMin = "0"))
	float SkateboardFwrdVelAccMult = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Skateboard Movement", meta = (DisplayName = "Sideways Ground Deceleration", ClampMin = "0", UIMin = "0"))
	float SkateboardSidewaysGroundDeceleration = 3400.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Skateboard Movement")
	bool bUseSeparateSidewasDecelerationWhenHandbraking = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Skateboard Movement",
	meta = (DisplayName = "Handbrake Sideways Ground Deceleration", EditCondition = "bUseSeparateSidewasDecelerationWhenHandbraking", ClampMin = "0", UIMin = "0"))
	float SkateboardHandbrakeSidewaysGroundDeceleration = 1400.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Skateboard Movement", meta = (DisplayName = "Braking Deceleration", ClampMin = "0", UIMin = "0"))
	float SkateboardBreakingDeceleration = 4000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Skateboard Movement", meta = (DisplayName = "Rotation Speed", ClampMin = "0", UIMin = "0"))
	float SkateboardRotationSpeed = 126.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Skateboard Movement",
		meta = (DisplayName = "Standtill Rotation Factor", ClampMin = "0", UIMin = "0"))
	float SkateboardStandstillRotationSpeed = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Skateboard Movement", meta = (DisplayName = "Slope Gravity Multiplier"))
	float SlopeGravityMultiplier = 1800.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Skateboard Movement", meta = (DisplayName = "Handbrake Rotation"))
	float HandbrakeRotationSpeed = 330.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Skateboard Movement", meta = (DisplayName = "Handbrake Velocity Threshold"))
	float HandbrakeVelocityThreshold = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Skateboard Movement", meta = (DisplayName = "Allow Braking While Handbraking?"))
	bool bAllowBrakingWhileHandbraking = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Skateboard Movement", meta = (EditCondition = "!bAllowBrakingWhileHandbraking"))
	bool bAllowKickingWhileHandbraking = true;

	/**
	 * The minimun amount of impact handbrake rotation will have on velocity. Default is 0.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Skateboard Movement", meta = (ClampMin = "0", UIMin = "0", ClampMax = "1", UIMax = "1"))
	float MinHandbrakeRotationVelocityImpact = 0.3f;

	/**
	 * How much of sideways velocity that should contribute to
	 * forward velocity (sliding sideways will give a forward boost)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Skateboard Movement")
	float SidewaysVelocityAccelerationGain = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Skateboard Movement", meta = (DisplayName = "Max Movement Speed"))
	float MaxSkateboardMovementSpeed = 3600.f;

	UPROPERTY(BlueprintReadOnly)
	APlayerCharacter* PlayerCharacter = nullptr;

	FVector InputDir;
	float SidewaysForce = 0.0f;

	bool bBraking = false;
	bool bIsStandstill = false;

	float ForwardVelocityFactor{0.f};
	float RightVelocityFactor{0.f};

public:
	// Parameter is mode that started
	FCustomMovementChangedSignature OnCustomMovementStart;
	// Parameter is mode that ended
	FCustomMovementChangedSignature OnCustomMovementEnd;

	FMovementChangedSignature OnMovementStart;
	FMovementChangedSignature OnMovementEnd;

	UPlayerCharacterMovementComponent();

	UFUNCTION(BlueprintPure)
	bool IsStandstill() const { return bStandstill; }

	bool IsMovingOnGround() const override;

	bool IsHandbraking() const { return GetHandbrakeAmount() != 0; }

	float GetRollingAudioVolumeMult() const;
	float GetGrindingAudioVolumeMult() const;

	UFUNCTION(BlueprintPure)
	float GetRotationInput() const { return InputDir.Y; }

	float GetRightVelocityFactor() const { return RightVelocityFactor; }
	float GetForwardVelocityFactor() const { return ForwardVelocityFactor; }

	float GetMaxForwardAcceleration() const;

	float GetMaxInputSpeed() { return MaxInputSpeed; }

	virtual float GetMaxSpeed() const override;

	bool CanKickWhileHandbraking() const;

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
	FVector GetInputAcceleration(bool &bBrakingOut, bool &bMovingBackwardsOut, float Input = 0.f);

	/**
	 * Multiplies kicking acceleration with a factor to make up for different tick speeds.
	 */
	FVector GetInputAccelerationTimeNormalized(const FVector& a, bool bBrakingIn, float DeltaTime) const;

	/**
	 * Returns GetInputAcceleration but zero-ed out if above max acceleration velocity.
	 */
	FVector GetClampedInputAcceleration(bool &bBrakingOut, float DeltaTime = 0.f, float Input = 0.f);

	void HandleImpact(const FHitResult& Hit, float TimeSlice = 0.f, const FVector& MoveDelta = FVector::ZeroVector) override;

protected:
	void BeginPlay() override;

	void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	/** @note Movement update functions should only be called through StartNewPhysics()*/
	void PhysCustom(float DeltaTime, int32 Iterations) override;

	void PhysSkateboard(float DeltaTime, int32 Iterations);

	float GetSidewaysDeceleration() const;

	FVector GetSidewaysToForwardAcceleration() const;

	void ApplySkateboardVelocityBraking(float DeltaTime, float BreakingForwardDeceleration, float BreakingSidewaysDeceleration);

	void UpdateInput() { InputDir = GetPendingInputVector(); }

	void TryFallOff() const;

	void CalcSkateboardVelocity(const FHitResult &FloorHitResult, float DeltaTime);

	// Applies this frames current rotation multiplied by deltaTime
	void ApplyRotation(float DeltaTime);

	FVector GetSlopeAcceleration(const FHitResult &FloorHitResult) const;
	float GetForwardInput() const { return InputDir.X; }
	float GetHandbrakeAmount() const { return InputDir.Z; }
	FVector GetRightInput() const { return FVector{ 0, InputDir.Y, 0 }; }
	float CalcSidewaysBreaking(const FVector& Forward) const;

	/**
	 * Returns current rotation amount.
	 * Both normal rotation and handbraking rotation.
	 */
	float CalcRotation() const;





	// ================================== Air Movement =================================================
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Skateboard Movement|Air", meta = (DisplayName = "Rotation Speed"))
	float SkateboardAirRotationSpeed = 120.f;

	/**
	 * Time in seconds to apply the speed burst gained after doing airtime.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Skateboard Movement|Air", meta = (DisplayName = "Burst Effect Length"))
	float AirBurstLength = 0.04f;

	/**
	 * How big impact vertical velocity has on the speed burst.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Skateboard Movement|Air", meta = (DisplayName = "Burst Effect Velocity Multiplier"))
	float AirBurstVelocityMultiplier = 0.15f;

	/**
	 * Optional curve to use when applying speed burst.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Skateboard Movement|Air", meta = (DisplayName = "Burst Effect Curve"))
	UCurveFloat* AirBurstEffectCurve = nullptr;

	float AirTime;
	float BurstApplyTimer;
	float BurstVerticalEnergy;

	/** Handle falling movement. */
	void PhysFalling(float DeltaTime, int32 Iterations) override;

	void ApplySpeedBurst(float DeltaTime);





	// ================================== Grinding =================================================
public:
	template<typename... Args>
	void SetSpline(Args&&... args);

	bool IsGrinding() const;

	FVector GetRailNormal() const;

protected:
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Grinding Movement")
	FSplineInfo CurrentSpline;

	FQuat GrindingRotation;

	float GrindingPositionValidationTimer;
	unsigned int GrindingPositionValidationCount = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Character Movement: Grinding Movement|Start")
	bool bUseConstantEnteringSpeed = true;

	UPROPERTY(EditDefaultsOnly, Category = "Character Movement: Grinding Movement|Start", meta = (DisplayName = "Entering Velocity", EditCondition="bUseConstantEnteringSpeed"))
	float GrindingEnteringSpeed = 2435.f;

	/**
	 * Whether to only use velocity in x and y direction (horizontal velocity) as the entering speed
	 * for grinding, as opposed to also counting speed in the z direction (from falling and jumping).
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Character Movement: Grinding Movement|Start", meta = (EditCondition = "!bUseConstantEnteringSpeed"))
	bool bOnlyUseHorizontalVelocityInGrindingEntering = false;

	UPROPERTY(EditDefaultsOnly, Category = "Character Movement: Grinding Movement|On Rail")
	bool bUseConstantGrindingSpeed = false;

	UPROPERTY(EditDefaultsOnly, Category = "Character Movement: Grinding Movement|On Rail", meta = (DisplayName = "Constant Grinding Speed", EditCondition = "bUseConstantGrindingSpeed"))
	float RailGrindingSpeed = 800.f;

	/**
	 * Whether to only use velocity in x and y direction (horizontal velocity) as the start speed
	 * for grinding, as opposed to also counting speed in the z direction (from falling and jumping).
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Character Movement: Grinding Movement|On Rail", meta = (EditCondition = "!bUseConstantGrindingSpeed"))
	bool bOnlyUseHorizontalVelocityInGrindingStart = true;

	/**
	 * Speed acceleration applied when grinding on the rail (not applied when entering rail)
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Character Movement: Grinding Movement|On Rail")
	float GrindingAcceleration = 135.f;

	/**
	 * How often the movement component will validate it's position on a rail in order
	 * to make up for floating point precision errors.
	 * Lower values will check often, increasing precision but also cost.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Character Movement: Grinding Movement|On Rail", meta = (DisplayName = "Position Validation Interval"))
	float GrindingPositionValidationInterval = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Character Movement: Grinding Movement|On Rail", meta = (DisplayName = "Max Speed"))
	float GrindingMaxSpeed = 2065.f;

	void PhysGrinding(float DeltaTime, int32 Iterations);

	void CalcGrindingVelocity(float DeltaTime);

	void CalcGrindingEnteringVelocity(float DeltaTime);
	void CalcGrindingRailVelocity(float DeltaTime);
	float GetNewCurvePoint() const;
	bool InEndInterval() const;
	bool InEndInterval(int32 LastIndex, bool bForward) const;
	bool IsAtCurveEnd(float DeltaTime) const;
	void CorrectGrindingPosError();

	FVector GetSkateboardLocation(APlayerCharacter* Owner = nullptr) const;

	UFUNCTION()
	void OnSplineChangedImplementation(EGrindingMovementState NewState);
};


template <typename... Args>
void UPlayerCharacterMovementComponent::SetSpline(Args&&... args)
{
	CurrentSpline = FSplineInfo{std::move(args)...};
	CurrentSpline.OnSplineChanged.AddUObject(this, &UPlayerCharacterMovementComponent::OnSplineChangedImplementation);
	GrindingPositionValidationCount = 0;
}