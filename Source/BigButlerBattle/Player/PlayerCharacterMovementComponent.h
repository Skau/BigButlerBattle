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
public:
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Grinding Movement", meta = (DisplayName = "Spline Reference"))
	USplineComponent* SkateboardSplineReference = nullptr;

	int SplineDir : 2;
	uint8 PlayerState : 2;
	uint8 PointCount;

	float SplinePos = -1.f;
	float StartDistanceToCurve;
	FVector StartVelocity;
	float StartVelocitySize; // Cheaper to save length than to do a square root almost every physics frame
	FRotator StartRotation;
	float TravelTime{0.f};

	FSplineStateChangedSignature OnSplineChanged;

public:
	FSplineInfo(USplineComponent* Spline = nullptr);

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
	float GetMaxAccelerationVelocity() const { return CustomMaxAccelerationVelocity; }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Custom Movement")
	ECustomMovementType DefaultCustomMovementMode = ECustomMovementType::MOVE_Skateboard;

	/**
	 * Max velocity to add input acceleration to. If velocity is higher, only acceleration from terrain get's applied.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Custom Movement", meta = (DisplayName = "Max Acceleration Velocity"))
	float CustomMaxAccelerationVelocity = 3600.f;

	bool bStandstill = false;





	// ================================== Skateboard Movement =================================================
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Skateboard Movement", meta = (DisplayName = "Standstill Threshold", ClampMin = "0", UIMin = "0"))
	float StandstillThreshold = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Skateboard Movement", meta = (DisplayName = "Forward Ground Deceleration", ClampMin = "0", UIMin = "0"))
	float SkateboardForwardGroundDeceleration = 306.f;

	/**
	 * How much current velocity will have an impact on acceleration
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Skateboard Movement", meta = (DisplayName = "Velocity Acceleration Multiplier", ClampMin = "0", UIMin = "0"))
	float SkateboardFwrdVelAccMult = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Skateboard Movement", meta = (DisplayName = "Sideways Ground Deceleration", ClampMin = "0", UIMin = "0"))
	float SkateboardSidewaysGroundDeceleration = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Skateboard Movement", meta = (DisplayName = "Braking Deceleration", ClampMin = "0", UIMin = "0"))
	float SkateboardBreakingDeceleration = 1356.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Skateboard Movement", meta = (DisplayName = "Rotation Speed", ClampMin = "0", UIMin = "0"))
	float SkateboardRotationSpeed = 155.f;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Skateboard Movement")
	float HandbrakeAcceleration = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Skateboard Movement")
	float HandbrakeMaxVelocity = 1000.f;

	UPROPERTY(BlueprintReadOnly)
	APlayerCharacter* PlayerCharacter = nullptr;

	FVector InputDir;
	float SidewaysForce = 0.0f;

	bool bBraking = false;
	bool bIsStandstill = false;

public:
	bool bHandbrake = false;

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

	float GetAudioVolumeMult() const;

	UFUNCTION(BlueprintPure)
	float GetRotationInput() const { return InputDir.Y; }

	float GetMaxForwardAcceleration() const;

	float GetMaxAccelerationVelocity() { return CustomMaxAccelerationVelocity; }

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

	FVector GetClampedHandbrakeAcceleration(float DeltaTime, float Input = 0.f);

	void HandleImpact(const FHitResult& Hit, float TimeSlice = 0.f, const FVector& MoveDelta = FVector::ZeroVector) override;

protected:
	void BeginPlay() override;

	void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	/** @note Movement update functions should only be called through StartNewPhysics()*/
	void PhysCustom(float DeltaTime, int32 Iterations) override;

	void PhysSkateboard(float deltaTime, int32 Iterations);

	void ApplySkateboardVelocityBraking(float DeltaTime, float BreakingForwardDeceleration, float BreakingSidewaysDeceleration);

	void UpdateInput() { InputDir = GetPendingInputVector(); }

	void TryFallOff() const;

	void CalcSkateboardVelocity(const FHitResult &FloorHitResult, float DeltaTime);

	// Applies this frames current rotation multiplied by deltaTime
	void ApplyRotation(float DeltaTime);

	bool IsHandbraking() const { return GetHandbrakeAmount() != 0; }

	FVector GetSlopeAcceleration(const FHitResult &FloorHitResult) const;
	float GetForwardInput() const { return InputDir.X; }
	float GetHandbrakeAmount() const { return InputDir.X <= 0.f ? -InputDir.X : 0.f; }
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
	float SkateboardAirRotationSpeed = 77.f;

	/**
	 * Time in seconds to apply the speed burst gained after doing airtime.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Skateboard Movement|Air", meta = (DisplayName = "Burst Effect Length"))
	float AirBurstLength = 0.105f;

	/**
	 * How big impact vertical velocity has on the speed burst.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Skateboard Movement|Air", meta = (DisplayName = "Burst Effect Velocity Multiplier"))
	float AirBurstVelocityMultiplier = 0.03f;

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

	UPROPERTY(EditDefaultsOnly, Category = "Character Movement: Grinding Movement|Start")
	bool bUseConstantEnteringSpeed = true;

	UPROPERTY(EditDefaultsOnly, Category = "Character Movement: Grinding Movement|Start", meta = (DisplayName = "Entering Velocity", EditCondition="bUseConstantEnteringSpeed"))
	float GrindingEnteringSpeed = 1600.f;

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
	float GrindingAcceleration = 200.f;

	UPROPERTY(EditDefaultsOnly, Category = "Character Movement: Grinding Movement|On Rail", meta = (DisplayName = "Max Speed"))
	float GrindingMaxSpeed = 1440.f;

	void PhysGrinding(float deltaTime, int32 Iterations);

	void CalcGrindingVelocity(float DeltaTime);

	void CalcGrindingEnteringVelocity(float DeltaTime);
	void CalcGrindingRailVelocity(float DeltaTime);
	float GetNewCurvePoint();
	bool InEndInterval() const;
	bool InEndInterval(int32 LastIndex, bool bForward) const;
	bool IsAtCurveEnd(float DeltaTime) const;

	FVector GetSkateboardLocation(APlayerCharacter* Owner = nullptr);

	UFUNCTION()
	void OnSplineChangedImplementation(EGrindingMovementState NewState);
};


template <typename... Args>
void UPlayerCharacterMovementComponent::SetSpline(Args&&... args)
{
	CurrentSpline = FSplineInfo{std::move(args)...};
	CurrentSpline.OnSplineChanged.AddUObject(this, &UPlayerCharacterMovementComponent::OnSplineChangedImplementation);
}