// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacterMovementComponent.h"
#include "PlayerCharacter.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/SplineComponent.h"
#include "Tasks/TaskObject.h"
#include "Utils/btd.h"
#include "Curves/CurveFloat.h"
#include "Kismet/GameplayStatics.h"

FSplineInfo::FSplineInfo(USplineComponent* Spline)
	: SplineDir{1}, PlayerState{static_cast<uint8>(EGrindingMovementState::STATE_GodKnowsWhere)}, PointCount{0}
{
	if (IsValid(Spline))
	{
		SkateboardSplineReference = Spline;
		PointCount = Spline->GetNumberOfSplinePoints();

		OnSplineChanged.AddLambda([&](EGrindingMovementState state){
			if (state == EGrindingMovementState::STATE_Leaving)
			{
				SkateboardSplineReference = nullptr;
			}
		});
	}
}

void FSplineInfo::SetState(EGrindingMovementState NewState)
{
	PlayerState = static_cast<uint8>(NewState);
	OnSplineChanged.Broadcast(NewState);
}

void FSplineInfo::Invalidate()
{
	SetState(EGrindingMovementState::STATE_Leaving);
}



UPlayerCharacterMovementComponent::UPlayerCharacterMovementComponent()
{
	DefaultLandMovementMode = EMovementMode::MOVE_Custom;
	DefaultWaterMovementMode = EMovementMode::MOVE_Custom;
	MaxCustomMovementSpeed = 4196.f;
	JumpZVelocity = 3052.5f;
	AirControl = 0.f;
	AirControlBoostMultiplier = 0.f;
	AirControlBoostVelocityThreshold = 0.f;
	MaxAcceleration = 1160.f;
	GravityScale = 10.5f;
	BrakingFrictionFactor = 2.f;

	SetMovementMode(EMovementMode::MOVE_Custom, static_cast<int>(DefaultCustomMovementMode));
}

bool UPlayerCharacterMovementComponent::IsMovingOnGround() const
{
	return ((MovementMode == MOVE_Custom && DefaultCustomMovementMode == ECustomMovementType::MOVE_Skateboard) || (MovementMode == MOVE_Walking) || (MovementMode == MOVE_NavWalking)) && UpdatedComponent;
}

float UPlayerCharacterMovementComponent::GetAudioVolumeMult() const
{
	return IsStandstill() ? 0.f :
			Velocity.Size() / GetMaxSpeed();
}

void UPlayerCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	PlayerCharacter = Cast<APlayerCharacter>(GetOwner());
	SetMovementMode(EMovementMode::MOVE_Custom, static_cast<int>(DefaultCustomMovementMode));

	OnCustomMovementEnd.AddLambda([&](uint8 movementMode){
		if (static_cast<ECustomMovementType>(movementMode) == ECustomMovementType::MOVE_Grinding)
		{
			if (static_cast<EGrindingMovementState>(CurrentSpline.PlayerState) == EGrindingMovementState::STATE_Entering)
			{
				// If exiting before entering spline, just revert to old speed.
				Velocity = CurrentSpline.StartVelocity;
			}

			// Reset curve
			CurrentSpline.Invalidate();
		}
	});

	OnMovementStart.AddLambda([&](EMovementMode movementMode){
		if (movementMode == EMovementMode::MOVE_Falling)
		{
			AirTime = GetWorld()->GetTimeSeconds();
		}
	});

	OnMovementEnd.AddLambda([&](EMovementMode movementMode){
		if (movementMode == EMovementMode::MOVE_Falling)
		{
			AirTime = GetWorld()->GetTimeSeconds() - AirTime;
			BurstApplyTimer = 0.f;
			BurstVerticalEnergy = FMath::Abs(Velocity.Z);
		}
	});
}

void UPlayerCharacterMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	UpdateInput();

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UPlayerCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	if (PreviousMovementMode != MovementMode)
	{
		OnMovementStart.Broadcast(MovementMode);

		OnMovementEnd.Broadcast(PreviousMovementMode);
	}

	if (PreviousCustomMode != CustomMovementMode)
	{
		// Don't generate oncustommovementmodestart events when changing to MOVE_None
		if (CustomMovementMode != static_cast<uint8>(ECustomMovementType::MOVE_None))
		{
			OnCustomMovementStart.Broadcast(CustomMovementMode);
		}

		// Don't generate oncustommovementmodeend events when changing from MOVE_None
		if (PreviousCustomMode != static_cast<uint8>(ECustomMovementType::MOVE_None))
		{
			OnCustomMovementEnd.Broadcast(PreviousCustomMode);
		}
	}

	// If changed to custom movement and default custom movement is not zero, switch to default custom movement.
	if (MovementMode == MOVE_Custom && CustomMovementMode == static_cast<uint8>(ECustomMovementType::MOVE_None) &&
		DefaultCustomMovementMode != ECustomMovementType::MOVE_None)
	{
		SetMovementMode(MOVE_Custom, static_cast<uint8>(DefaultCustomMovementMode));
	}
}

void UPlayerCharacterMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
	Super::PhysCustom(DeltaTime, Iterations);

	switch (CustomMovementMode)
	{
		case static_cast<uint8>(ECustomMovementType::MOVE_Skateboard):
			PhysSkateboard(DeltaTime, Iterations);
			break;
		case static_cast<uint8>(ECustomMovementType::MOVE_Grinding):
			PhysGrinding(DeltaTime, Iterations);
			break;
		default:
			break;
	}

	// Clamp all movement to custom max movement speed
	if (!Velocity.IsNearlyZero() && MaxCustomMovementSpeed * MaxCustomMovementSpeed < Velocity.SizeSquared())
	{
		Velocity = Velocity.GetSafeNormal() * MaxCustomMovementSpeed;
	}
}

void UPlayerCharacterMovementComponent::PhysSkateboard(float DeltaTime, int32 Iterations)
{
	if (DeltaTime < MIN_TICK_TIME)
	{
		return;
	}

	if (HasAnimRootMotion() || (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy) || CurrentRootMotion.HasOverrideVelocity())
	{
		UE_LOG(LogTemp, Error, TEXT("We don't know how to handle root motion or simulated proxies. :s"));
		return;
	}

	check(!Velocity.ContainsNaN());

	if (!CharacterOwner || (!CharacterOwner->GetController() && !bRunPhysicsWithNoController && !HasAnimRootMotion()))
	{
		Acceleration = FVector::ZeroVector;
		Velocity = FVector::ZeroVector;
		return;
	}

	bStandstill = Velocity.Size() < StandstillThreshold;
	bJustTeleported = false;
	bool bCheckedFall = false;
	float RemainingTime = DeltaTime;

	// Perform the move
	while ((RemainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations) && (CharacterOwner->Controller || bRunPhysicsWithNoController))
	{
		Iterations++;
		bJustTeleported = false;
		const float TimeTick = GetSimulationTimeStep(RemainingTime, Iterations);
		RemainingTime -= TimeTick;

		// Save current values
		UPrimitiveComponent* const OldBase = GetMovementBase();
		const FVector PreviousBaseLocation = (OldBase != NULL) ? OldBase->GetComponentLocation() : FVector::ZeroVector;
		const FVector OldLocation = UpdatedComponent->GetComponentLocation();
		const FFindFloorResult OldFloor = CurrentFloor;

		// Ensure velocity is horizontal.
		MaintainHorizontalGroundVelocity();

		CalcSkateboardVelocity(OldFloor.HitResult, TimeTick);
		check(!Velocity.ContainsNaN());

		// Check if player should fall off
		TryFallOff();

		// Compute move parameters
		const FVector MoveVelocity = Velocity;
		const FVector Delta = TimeTick * MoveVelocity;
		const bool bZeroDelta = Delta.IsNearlyZero();
		FStepDownResult StepDownResult;

		if (bZeroDelta)
		{
			RemainingTime = 0.f;
		}
		else
		{
			// try to move forward
			MoveAlongFloor(MoveVelocity, TimeTick, &StepDownResult);

			if (IsFalling())
			{
				// pawn decided to jump up
				const float DesiredDist = Delta.Size();
				if (DesiredDist > KINDA_SMALL_NUMBER)
				{
					const float ActualDist = (UpdatedComponent->GetComponentLocation() - OldLocation).Size2D();
					RemainingTime += TimeTick * (1.f - FMath::Min(1.f, ActualDist / DesiredDist));
				}
				StartNewPhysics(RemainingTime, Iterations);
				return;
			}
		}

		// Update floor.
		// StepUp might have already done it for us.
		if (StepDownResult.bComputedFloor)
		{
			CurrentFloor = StepDownResult.FloorResult;
		}
		else
		{
			FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, bZeroDelta, NULL);
		}

		// check for ledges here
		if (CurrentFloor.IsWalkableFloor())
		{
				AdjustFloorHeight();
				SetBase(CurrentFloor.HitResult.Component.Get(), CurrentFloor.HitResult.BoneName);
		}
		else
		{
				// see if it is OK to jump
				// @todo collision : only thing that can be problem is that oldbase has world collision on
				bool bMustJump = bJustTeleported || bZeroDelta || (OldBase == NULL || (!OldBase->IsQueryCollisionEnabled() && MovementBaseUtility::IsDynamicBase(OldBase)));
				if ((bMustJump || !bCheckedFall) && CheckFall(OldFloor, CurrentFloor.HitResult, Delta, OldLocation, RemainingTime, TimeTick, Iterations, bMustJump))
				{
					return;
				}
				bCheckedFall = true;

				// revert this move
				RevertMove(OldLocation, OldBase, PreviousBaseLocation, OldFloor, true);
				RemainingTime = 0.f;
				break;
		}

		// If we didn't move at all this iteration then abort (since future iterations will also be stuck).
		if (UpdatedComponent->GetComponentLocation() == OldLocation)
		{
			RemainingTime = 0.f;
			break;
		}
	}

	if (IsMovingOnGround())
	{
		MaintainHorizontalGroundVelocity();
	}
}

void UPlayerCharacterMovementComponent::ApplySkateboardVelocityBraking(float DeltaTime, float BreakingForwardDeceleration, float BreakingSidewaysDeceleration)
{
	if (Velocity.IsZero() || !HasValidData() || HasAnimRootMotion() || DeltaTime < MIN_TICK_TIME)
	{
		return;
	}

	const float Friction = FMath::Max(0.f, BrakingFriction * BrakingFrictionFactor);
	BreakingForwardDeceleration = FMath::Max(0.f, BreakingForwardDeceleration);
	BreakingSidewaysDeceleration = FMath::Max(0.f, BreakingSidewaysDeceleration);
	const bool bZeroFriction = FMath::IsNearlyZero(Friction);
	const bool bZeroForwardBraking = FMath::IsNearlyZero(BreakingForwardDeceleration);
	const bool bZeroSidewaysBraking = FMath::IsNearlyZero(BreakingSidewaysDeceleration);

	if (bZeroFriction && bZeroForwardBraking && bZeroSidewaysBraking)
	{
		return;
	}

	const FVector OldVel = Velocity;

	// subdivide braking to get reasonably consistent results at lower frame rates
	// (important for packet loss situations w/ networking)
	float RemainingTime = DeltaTime;
	const float MaxTimeStep = FMath::Clamp(BrakingSubStepTime, 1.0f / 75.0f, 1.0f / 20.0f);

	// Decelerate to brake to a stop
	const float ForwardFactor = FVector::DotProduct(GetOwner()->GetActorForwardVector(), Velocity.GetSafeNormal());
	const float SidewaysFactor = FVector::DotProduct(GetOwner()->GetActorRightVector(), Velocity.GetSafeNormal());
	const FVector RevForwardAcceleration = (bZeroForwardBraking ? FVector::ZeroVector : (-BreakingForwardDeceleration * (GetOwner()->GetActorForwardVector() * ForwardFactor)));
	const FVector RevSidewaysAcceleration = (bZeroSidewaysBraking ? FVector::ZeroVector : (-BreakingSidewaysDeceleration * (GetOwner()->GetActorRightVector() * SidewaysFactor)));
	SidewaysForce = RevSidewaysAcceleration.Size2D();

	while (RemainingTime >= MIN_TICK_TIME)
	{
		// Zero friction uses constant deceleration, so no need for iteration.
		const float DT = ((RemainingTime > MaxTimeStep && !bZeroFriction) ? FMath::Min(MaxTimeStep, RemainingTime * 0.5f) : RemainingTime);
		RemainingTime -= DT;

		// apply friction and braking
		Velocity = Velocity + (-Friction * Velocity + RevForwardAcceleration + RevSidewaysAcceleration) * DT;

		// Don't reverse direction
		if ((Velocity | OldVel) <= 0.f)
		{
			Velocity = FVector::ZeroVector;
			return;
		}
	}

	// Clamp to zero if nearly zero, or if below min threshold and braking.
	const float VSizeSq = Velocity.SizeSquared();
	if (VSizeSq <= KINDA_SMALL_NUMBER || (!(bZeroForwardBraking && bZeroSidewaysBraking) && VSizeSq <= FMath::Square(BRAKE_TO_STOP_VELOCITY)))
	{
		Velocity = FVector::ZeroVector;
	}
}

void UPlayerCharacterMovementComponent::TryFallOff() const
{
	if (!PlayerCharacter || !PlayerCharacter->CanFall())
		return;

	if (SidewaysForce > PlayerCharacter->GetSidewaysForceFallOffThreshold())
	{
		PlayerCharacter->EnableRagdoll();
	}
}

void UPlayerCharacterMovementComponent::CalcSkateboardVelocity(const FHitResult &FloorHitResult, float DeltaTime)
{
	// Do not update velocity when using root motion or when SimulatedProxy and not simulating root motion - SimulatedProxy are repped their Velocity
	if (!HasValidData() || HasAnimRootMotion() || DeltaTime < MIN_TICK_TIME || (CharacterOwner && CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy && !bWasSimulatingRootMotion))
	{
		return;
	}

	float MaxSpeed = GetMaxSpeed();

	// Calculate and set acceleration
	Acceleration = GetClampedInputAcceleration(bBraking, DeltaTime);

	// Get the fully modified analog input value.
	MaxSpeed = FMath::Max(MaxSpeed * AnalogInputModifier, GetMinAnalogSpeed());

	// Apply braking or deceleration
	const bool bZeroAcceleration = Acceleration.IsZero();
	const bool bVelocityOverMax = IsExceedingMaxSpeed(MaxSpeed);

	const FVector OldVelocity = Velocity;
	// Don't allow braking to lower us below max speed if we started above it.
	if (bVelocityOverMax && Velocity.SizeSquared() < FMath::Square(MaxSpeed) && FVector::DotProduct(Acceleration, OldVelocity) > 0.0f)
	{
		Velocity = OldVelocity.GetSafeNormal() * MaxSpeed;
	}

	bIsStandstill = Velocity.Size() < StandstillThreshold;
	const bool bShouldStopCompletely = bBraking && bIsStandstill;

	// Apply acceleration if there is any, and a braking deceleration if trying to reverse
	if (!bZeroAcceleration)
	{
		if (bShouldStopCompletely)
		{
			Velocity = FVector::ZeroVector;
		}
		else
		{
			const float NewMaxSpeed = IsExceedingMaxSpeed(MaxSpeed) ? Velocity.Size() : MaxSpeed;
			Velocity += Acceleration * DeltaTime;
			Velocity = Velocity.GetClampedToMaxSize(NewMaxSpeed);
		}
	}

	// Apply slope acceleration (needs to be applied after input acceleration as to not get clamped)
	if (!bShouldStopCompletely)
	{
		const auto slopeAcceleration = GetSlopeAcceleration(FloorHitResult);
		Velocity += slopeAcceleration * DeltaTime;
	}

	ApplySpeedBurst(DeltaTime);

	const auto forwardDeceleration = (bApplySeparateDecelerationWhenAboveMaxInputSpeed && IsAboveInputSpeed())
		? SkateboardForwardGroundDecelerationAboveInputSpeed : SkateboardForwardGroundDeceleration;
	ApplySkateboardVelocityBraking(DeltaTime, forwardDeceleration, SkateboardSidewaysGroundDeceleration);

	ApplyRotation(DeltaTime);

	if (!Velocity.IsNearlyZero() && MaxSkateboardMovementSpeed * MaxSkateboardMovementSpeed < Velocity.SizeSquared())
	{
		Velocity = Velocity.GetSafeNormal() * MaxSkateboardMovementSpeed;
	}
}

void UPlayerCharacterMovementComponent::ApplyRotation(float DeltaTime)
{
	// Apply rotation based on input
	const auto rotAmount = CalcRotation() * DeltaTime;
	GetOwner()->AddActorWorldRotation(FRotator{0.f, rotAmount, 0.f});
	// Rotate velocity the same amount as forward dir.
	if (!Velocity.IsNearlyZero())
	{
		float vImpactFactor = FMath::Lerp(1.f, MinHandbrakeRotationVelocityImpact, GetHandbrakeAmount());
		Velocity = Velocity.RotateAngleAxis(rotAmount * vImpactFactor, FVector(0, 0, 1));
	}
}

FVector UPlayerCharacterMovementComponent::GetSlopeAcceleration(const FHitResult &FloorHitResult) const
{
	if (!FloorHitResult.bBlockingHit)
		return FVector::ZeroVector;

	const auto n = FloorHitResult.Normal;
	const FVector u = FVector(0, 0, 1);

	const float alpha = FMath::Acos(FVector::DotProduct(n, u));
	const float cosAlpha = FMath::Cos(alpha);

	// If it's zero, then there is no acceleration in the horizontal plane, because the slope is vertical.
	if (!cosAlpha)
		return FVector::ZeroVector;

	const float N = SlopeGravityMultiplier / cosAlpha;
	const float Nx = N * FMath::Cos((PI / 2) - alpha);

	const FVector d = FVector::CrossProduct(FVector::CrossProduct(u, n), u).GetSafeNormal();

	const FVector a = d * Nx;

	return a;
}

float UPlayerCharacterMovementComponent::GetMaxForwardAcceleration() const
{
	return FMath::Max(FMath::Abs(SkateboardKickingAcceleration) - FVector::DotProduct(Velocity, GetOwner()->GetActorForwardVector()) * SkateboardFwrdVelAccMult, 0.f);
}

inline float UPlayerCharacterMovementComponent::CalcSidewaysBreaking(const FVector &Forward) const
{
	return 1.f - FMath::Abs(FVector::DotProduct(Forward, Velocity));
}

bool UPlayerCharacterMovementComponent::CanKickWhileHandbraking() const
{
	return !bAllowBrakingWhileHandbraking && bAllowKickingWhileHandbraking;
}

bool UPlayerCharacterMovementComponent::CanForwardAccelerate(const FVector &AccelerationIn, const float DeltaTime) const
{
	const bool bMovingBackwards = FVector::DotProduct(Velocity, GetOwner()->GetActorForwardVector()) < 0.f;
	return CanForwardAccelerate(AccelerationIn, DeltaTime, bMovingBackwards);
}

bool UPlayerCharacterMovementComponent::CanForwardAccelerate(const FVector &AccelerationIn, const float DeltaTime, const bool bMovingBackwards) const
{
	return (!IsHandbraking() || CanKickWhileHandbraking())  && (bMovingBackwards || (DeltaTime >= MIN_TICK_TIME && (Velocity + AccelerationIn * DeltaTime).SizeSquared() < FMath::Square(MaxInputSpeed)));
}

bool UPlayerCharacterMovementComponent::CanAccelerate(const FVector &AccelerationIn, const bool bBrakingIn, const float DeltaTime) const
{
	return bBrakingIn || CanForwardAccelerate(AccelerationIn, DeltaTime);
}

bool UPlayerCharacterMovementComponent::CanAccelerate(const FVector &AccelerationIn, const bool bBrakingIn, const float DeltaTime, const bool bMovingBackwards) const
{
	return bBrakingIn || CanForwardAccelerate(AccelerationIn, DeltaTime, bMovingBackwards);
}

FVector UPlayerCharacterMovementComponent::GetInputAcceleration(bool &bBrakingOut, bool &bMovingBackwardsOut, float Input)
{
	if (Input == 0)
	{
		Input = GetForwardInput();
		if (Input == 0)
		{
			return FVector::ZeroVector;
		}
	}

	// If input is negative, we are currently braking on the controller.
	bBrakingOut = Input < 0.f;

	// Scale braking with rotation, 0% rotation equals 100% braking
	if (bBrakingOut)
		Input *= 1.f - FMath::Abs(GetRotationInput());

	// Remove vertical input if handbraking and not normal braking with bAllowBrakingWhileHandbraking enabled.
	const bool bCanMoveVertically = !IsHandbraking() || CanKickWhileHandbraking() || (bAllowBrakingWhileHandbraking && bBraking);
	const float Factor = bCanMoveVertically * Input * (bBrakingOut ? FMath::Abs(SkateboardBreakingDeceleration) : GetMaxForwardAcceleration());
	auto a = UpdatedComponent->GetForwardVector().GetSafeNormal() * Factor;

	if (a.IsNearlyZero())
		a = FVector::ZeroVector;

	bMovingBackwardsOut = FVector::DotProduct(Velocity, GetOwner()->GetActorForwardVector()) < 0.f;

	// If we have backwards velocity we need to flip acceleration so we still brake (backwards acceleration should be forward until velocity is 0 again)
	if (bBrakingOut && bMovingBackwardsOut)
	{
		a = -a;
	}

	if (a.IsNearlyZero())
		a = FVector::ZeroVector;

	return a;
}

FVector UPlayerCharacterMovementComponent::GetInputAccelerationTimeNormalized(const FVector &a, const bool bBrakingIn, const float DeltaTime) const
{
	return (bBrakingIn || DeltaTime < MIN_TICK_TIME) ? a : a * (1.f / DeltaTime);
}

FVector UPlayerCharacterMovementComponent::GetClampedInputAcceleration(bool &bBrakingOut, const float DeltaTime, const float Input)
{
	bool bMovingBackwards;
	auto a = GetInputAcceleration(bBrakingOut, bMovingBackwards, Input);
	a = GetInputAccelerationTimeNormalized(a, bBrakingOut, DeltaTime);
	return CanAccelerate(a, bBrakingOut, DeltaTime, bMovingBackwards) ? a : FVector::ZeroVector;
}

void UPlayerCharacterMovementComponent::HandleImpact(const FHitResult& Hit, const float TimeSlice, const FVector& MoveDelta)
{
	if (Hit.GetActor()->IsA(ATaskObject::StaticClass()))
	{
		Super::HandleImpact(Hit, TimeSlice, MoveDelta);
		return;
	}

	const auto Angle = FMath::RadiansToDegrees(btd::FastAcos(FMath::Abs(FVector::DotProduct(Velocity.GetSafeNormal(), Hit.ImpactNormal))));

	if (PlayerCharacter && Angle < PlayerCharacter->GetCrashAngleThreshold () && Velocity.Size() > PlayerCharacter->GetCrashVelocityFallOffThreshold())
	{
		PlayerCharacter->EnableRagdoll();

		if (auto Other = Cast<APlayerCharacter>(Hit.GetActor()))
		{
			if (Other->CanFall())
			{
				Other->EnableRagdoll();
			}
		}
	}
	else
	{
		Super::HandleImpact(Hit, TimeSlice, MoveDelta);
	}
}

float UPlayerCharacterMovementComponent::CalcRotation() const
{
	const float StandstillRotationSpeed = SkateboardRotationSpeed * SkateboardStandstillRotationSpeed;

	if (IsFalling())
	{
		return GetRotationInput() * SkateboardAirRotationSpeed;
	}
	else
	{
		const auto speed = (bIsStandstill ? StandstillRotationSpeed : FMath::Lerp(SkateboardRotationSpeed, HandbrakeRotationSpeed, GetHandbrakeAmount()));
		return GetRotationInput() * speed;
	}
}





// ================================== Air Movement =================================================
void UPlayerCharacterMovementComponent::PhysFalling(float DeltaTime, int32 Iterations)
{
	Super::PhysFalling(DeltaTime, Iterations);

	// Apply rotation based on input
	const auto rotAmount = CalcRotation() * DeltaTime;
	if (!FMath::IsNearlyZero(rotAmount))
	{
		GetOwner()->AddActorWorldRotation(FRotator{0.f, rotAmount, 0.f});

		// Set velocity to be facing same direction as forward dir.
		if (!Velocity.IsNearlyZero())
			Velocity = Velocity.RotateAngleAxis(rotAmount, FVector(0, 0, 1));
	}
}

void UPlayerCharacterMovementComponent::ApplySpeedBurst(float DeltaTime)
{
	if (BurstApplyTimer >= AirBurstLength)
		return;

	if (!AirBurstEffectCurve)
	{
		AirBurstEffectCurve = NewObject<UCurveFloat>(this, TEXT("Default Curve"));
		const auto errors = AirBurstEffectCurve->CreateCurveFromCSVString("0,1\n1,0\n");
		for (const auto &err : errors)
		{
			UE_LOG(LogTemp, Error, TEXT("Err: %s"), *err);
		}
		if (0 < errors.Num())
			return;
	}
	
	const float factor = FMath::Clamp(BurstApplyTimer / AirBurstLength, 0.f, 1.f);
	const float speedBurst = BurstVerticalEnergy * AirBurstVelocityMultiplier * AirBurstEffectCurve->GetFloatValue(factor);

	Velocity += Velocity.GetSafeNormal() * speedBurst;

	BurstApplyTimer += DeltaTime;
}






// ================================== Grinding =================================================
bool UPlayerCharacterMovementComponent::IsGrinding() const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == static_cast<uint8>(ECustomMovementType::MOVE_Grinding);
}

FVector UPlayerCharacterMovementComponent::GetRailNormal() const
{
	if (!IsGrinding() || !CurrentSpline.HasValue())
		return FVector::ZeroVector;

	auto owner = GetOwner();
	auto splineDir = CurrentSpline.SkateboardSplineReference->GetDirectionAtSplineInputKey(CurrentSpline.SplinePos, ESplineCoordinateSpace::World);
	return FVector::CrossProduct(splineDir * CurrentSpline.SplineDir, owner->GetActorRightVector()).GetSafeNormal();
}

void UPlayerCharacterMovementComponent::PhysGrinding(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	float remainingTime = deltaTime;
	while( (remainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations) && IsValid(CharacterOwner) && HasValidData())
	{
		Iterations++;
		float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
		remainingTime -= timeTick;
		const FVector oldVelocity = Velocity;


		if (HasAnimRootMotion() || CurrentRootMotion.HasOverrideVelocity())
		{
			UE_LOG(LogTemp, Error, TEXT("Grinding motion doesn't know how to manage root motion! PANIC!!"));
			return;
		}

		if (!CurrentSpline.HasValue())
		{
			UE_LOG(LogTemp, Error, TEXT("Spline is not valid!"));
			return;
		}



		// If there's less than 2 points along the curve, curve cannot be traversed. Return to falling movement.
		if (CurrentSpline.PointCount < 2)
		{
			UE_LOG(LogTemp, Error, TEXT("Not enough points for grinding movement!"));
			SetMovementMode(EMovementMode::MOVE_Falling);
			StartNewPhysics(remainingTime, Iterations);
			return;
		}

		auto& SplineRef = *CurrentSpline.SkateboardSplineReference;


		// 1. If just entering the spline, do a setup.
		if (CurrentSpline.PlayerState == static_cast<uint8>(EGrindingMovementState::STATE_GodKnowsWhere))
		{
			CurrentSpline.SetState(EGrindingMovementState::STATE_Entering);
		}



		// 2. Find acceleration

		// 3. Find velocity
		CalcGrindingVelocity(timeTick);
		const bool bAtEnd = IsAtCurveEnd(timeTick);

		// 4. Move
		if (Velocity.IsNearlyZero())
		{
			// Reset velocity
			Velocity = oldVelocity;
			SetMovementMode(EMovementMode::MOVE_Falling);
			StartNewPhysics(remainingTime, Iterations);
		}
		else
		{

			FHitResult Hit(1.f);
			bool bMoveResult = SafeMoveUpdatedComponent(Velocity * timeTick, GrindingRotation, true, Hit);
		}





		// 5. Check if outside curve.
		if (static_cast<EGrindingMovementState>(CurrentSpline.PlayerState) == EGrindingMovementState::STATE_OnRail)
		{
			CurrentSpline.SplinePos = GetNewCurvePoint();
			if (bAtEnd || CurrentSpline.SplinePos >= CurrentSpline.PointCount || CurrentSpline.SplinePos < 0.f)
			{
				SetMovementMode(EMovementMode::MOVE_Falling);
				StartNewPhysics(remainingTime, Iterations);
			}
		}
	}
}

void UPlayerCharacterMovementComponent::CalcGrindingVelocity(float DeltaTime)
{
	switch(CurrentSpline.PlayerState)
	{
		case static_cast<uint8>(EGrindingMovementState::STATE_Entering):
			CalcGrindingEnteringVelocity(DeltaTime);
			break;

		case static_cast<uint8>(EGrindingMovementState::STATE_OnRail):
			CalcGrindingRailVelocity(DeltaTime);
			break;

		default:
			break;
	}
}

void UPlayerCharacterMovementComponent::CalcGrindingEnteringVelocity(float DeltaTime)
{
	auto owner = Cast<APlayerCharacter>(GetOwner());
	if (!owner || DeltaTime < MIN_TICK_TIME)
	{
		UE_LOG(LogTemp, Error, TEXT("Could'nt get playercharacter!"));
		return;
	}

	// Get velocity size
	const float vSize = bUseConstantEnteringSpeed ? GrindingEnteringSpeed : Velocity.Size();
	auto& SplineRef = *CurrentSpline.SkateboardSplineReference;


	// Figure out start distance to curve.
	float startVel = bUseConstantEnteringSpeed ? vSize : CurrentSpline.StartVelocitySize;
	/*
		v = s / t
		s / v = s / (s / t) = (s * t) / s = t
	*/
	const float SecondsToHitCurve = FMath::IsNearlyZero(startVel) ?
	0.f : CurrentSpline.StartDistanceToCurve / startVel;


	// Get how far into the entering process we are (CurrentTimeStep).
	CurrentSpline.TravelTime += DeltaTime;
	const float CurrentTimeStep = FMath::IsNearlyZero(SecondsToHitCurve) ? 1.f : CurrentSpline.TravelTime / SecondsToHitCurve;

	FVector SplineWorldPos = SplineRef.GetLocationAtSplineInputKey(CurrentSpline.SplinePos, ESplineCoordinateSpace::World);
	// We subtract the skateboard offset because we want the character centre to be the skateboard centre on the curve.
	auto dist = (SplineWorldPos - owner->GetSkateboardLocation()) - owner->GetActorLocation();
	const bool bArrived = FMath::IsNearlyZero(SecondsToHitCurve) || dist.Size() < vSize * DeltaTime;

	// If we needed to clamp velocity to the distance to the spline, we have arrived on the spline.
	if (bArrived)
	{
		Velocity = dist / DeltaTime;
		CurrentSpline.SetState(EGrindingMovementState::STATE_OnRail);
	}
	else
	{
		Velocity = dist.GetSafeNormal() * vSize;
	}

	// Check velocity
	if (Velocity.ContainsNaN())
		Velocity = FVector::ZeroVector;


	// Rotation
	auto SplineDir = SplineRef.GetDirectionAtSplineInputKey(CurrentSpline.SplinePos, ESplineCoordinateSpace::World) * CurrentSpline.SplineDir;
	auto TargetRotation = UKismetMathLibrary::MakeRotFromZX(FVector::UpVector, SplineDir);
	GrindingRotation = FQuat::Slerp(CurrentSpline.StartRotation.Quaternion(), TargetRotation.Quaternion(), CurrentTimeStep);
}

void UPlayerCharacterMovementComponent::CalcGrindingRailVelocity(float DeltaTime)
{
	CurrentSpline.TravelTime += DeltaTime;

	auto& SplineRef = *CurrentSpline.SkateboardSplineReference;
	FVector SplineWorldPos = SplineRef.GetLocationAtSplineInputKey(CurrentSpline.SplinePos, ESplineCoordinateSpace::World);
	FVector Dir = SplineRef.GetDirectionAtSplineInputKey(CurrentSpline.SplinePos, ESplineCoordinateSpace::World) * CurrentSpline.SplineDir;

	float Speed = (bUseConstantGrindingSpeed ? RailGrindingSpeed : CurrentSpline.StartVelocitySize)
		+ CurrentSpline.TravelTime * GrindingAcceleration;
	float NextSplinePos = SplineRef.FindInputKeyClosestToWorldLocation(SplineWorldPos + Dir * DeltaTime * Speed);
	
	FVector CurveCorrectedDir = SplineRef.GetLocationAtSplineInputKey(NextSplinePos, ESplineCoordinateSpace::World) - SplineWorldPos;

	// Set new velocity
	Velocity = CurveCorrectedDir / DeltaTime;
	// Check velocity
	if (Velocity.ContainsNaN())
		Velocity = FVector::ZeroVector;


	const auto vDir = Velocity.GetSafeNormal();
	// Add acceleration:
	// Velocity += vDir * GrindingAcceleration * DeltaTime;

	// Clamp Velocity
	Velocity = vDir * FMath::Clamp(Velocity.Size(), 0.f, GrindingMaxSpeed);

	GrindingRotation = UKismetMathLibrary::MakeRotFromZX(FVector::UpVector, Velocity.IsNearlyZero() ? FVector::ForwardVector : Velocity).Quaternion();
}

float UPlayerCharacterMovementComponent::GetNewCurvePoint()
{
	auto playerCharacter = Cast<APlayerCharacter>(GetOwner());
	if (!playerCharacter)
		return CurrentSpline.SplinePos;

	return CurrentSpline.SkateboardSplineReference->FindInputKeyClosestToWorldLocation(GetSkateboardLocation(playerCharacter));
}

bool UPlayerCharacterMovementComponent::InEndInterval() const
{
	const bool bForward = CurrentSpline.SplineDir > 0;
	const auto lastIndex = bForward ? CurrentSpline.PointCount - 1 : 0;
	return InEndInterval(lastIndex, bForward);
}

bool UPlayerCharacterMovementComponent::InEndInterval(int32 LastIndex, bool bForward) const
{
	const bool bTwoPoints = CurrentSpline.PointCount == 2;
	/**
	 * If pointcount is 2, check if splinepos >= 0 and splinepos <= 1, otherwise:
	 * 
	 * If moving forward splinepos has to be greater than lastindex - 1 and less than lastindex,
	 * if moving backwards splinepos has to be greater than 0 and less than 1.
	 */
	return CurrentSpline.SplinePos >= (bTwoPoints ? 0 : LastIndex - bForward) && CurrentSpline.SplinePos <= (bTwoPoints ? 1 : LastIndex + !bForward);
}

bool UPlayerCharacterMovementComponent::IsAtCurveEnd(float DeltaTime) const
{
	const bool bForward = CurrentSpline.SplineDir > 0;
	const auto lastIndex = bForward ? CurrentSpline.PointCount - 1 : 0;

	if (!InEndInterval(lastIndex, bForward))
		return false;

	auto splineWorldPos = CurrentSpline.SkateboardSplineReference->GetLocationAtSplineInputKey(CurrentSpline.SplinePos, ESplineCoordinateSpace::World);
	auto lastPoint = CurrentSpline.SkateboardSplineReference->GetLocationAtSplinePoint(lastIndex, ESplineCoordinateSpace::World);
	auto nextPoint = splineWorldPos + Velocity * DeltaTime;
	return FVector::DotProduct(nextPoint - lastPoint, nextPoint - splineWorldPos) > 0.f;
}

FVector UPlayerCharacterMovementComponent::GetSkateboardLocation(APlayerCharacter* Owner)
{
	if (!Owner)
		Owner = Cast<APlayerCharacter>(GetOwner());

	return Owner ? Owner->GetActorLocation() + Owner->GetSkateboardLocation() : FVector{};
}

UFUNCTION()
void UPlayerCharacterMovementComponent::OnSplineChangedImplementation(EGrindingMovementState NewState)
{
	if (NewState == EGrindingMovementState::STATE_Entering)
	{
		auto owner = Cast<APlayerCharacter>(GetOwner());
		if (!IsValid(owner))
			return;

		auto &SplineRef = *CurrentSpline.SkateboardSplineReference;

		auto StartWorldPos = owner->GetActorLocation();
		CurrentSpline.StartVelocity = Velocity;
		CurrentSpline.StartVelocitySize = bOnlyUseHorizontalVelocityInGrindingEntering ? Velocity.Size2D() : Velocity.Size();
		CurrentSpline.StartRotation = owner->GetActorRotation();
		CurrentSpline.SplinePos = SplineRef.FindInputKeyClosestToWorldLocation(StartWorldPos);
		// We subtract the skateboard offset because we want the character centre to be the skateboard centre on the curve.
		FVector SplineWorldPos = SplineRef.GetLocationAtSplineInputKey(CurrentSpline.SplinePos, ESplineCoordinateSpace::World) - owner->GetSkateboardLocation();
		CurrentSpline.StartDistanceToCurve = (SplineWorldPos - StartWorldPos).Size();
		// UE_LOG(LogTemp, Warning, TEXT("Started grinding movement! Startingpos: %f"), CurrentSpline.SplinePos);
		if (!Velocity.IsNearlyZero())
		{
			auto splineDir = SplineRef.GetDirectionAtSplineInputKey(CurrentSpline.SplinePos, ESplineCoordinateSpace::World);
			CurrentSpline.SplineDir = (FVector::DotProduct(splineDir, Velocity) > 0) ? 1 : -1;
		}
	}
	else if (NewState == EGrindingMovementState::STATE_OnRail)
	{
		CurrentSpline.TravelTime = 0.f;

		if (bOnlyUseHorizontalVelocityInGrindingEntering ^ bOnlyUseHorizontalVelocityInGrindingStart)
			CurrentSpline.StartVelocitySize = bOnlyUseHorizontalVelocityInGrindingStart ? CurrentSpline.StartVelocity.Size2D() : CurrentSpline.StartVelocity.Size();
	}
}
