// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacterMovementComponent.h"
#include "PlayerCharacter.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/SplineComponent.h"
#include "Tasks/TaskObject.h"
#include "PlayerCharacter.h"
#include "Utils/btd.h"

FSplineInfo::FSplineInfo(USplineComponent* Spline)
	: bHasValue{0}, SplineDir{1}, PlayerState{static_cast<uint8>(STATE_GodKnowsWhere)}, PointCount{0}
{
	if (IsValid(Spline))
	{
		SkateboardSplineReference = Spline;
		bHasValue = true;
		PointCount = Spline->GetNumberOfSplinePoints();
	}
}



UPlayerCharacterMovementComponent::UPlayerCharacterMovementComponent()
{
	DefaultLandMovementMode = EMovementMode::MOVE_Custom;
	DefaultWaterMovementMode = EMovementMode::MOVE_Custom;
	MaxCustomMovementSpeed = 4196.f;
	JumpZVelocity = 1600.f;
	AirControl = 0.f;
	AirControlBoostMultiplier = 0.f;
	AirControlBoostVelocityThreshold = 0.f;
	MaxAcceleration = 1800.f;
	GravityScale = 3.0f;

	SetMovementMode(EMovementMode::MOVE_Custom, static_cast<int>(DefaultCustomMovementMode));
}

bool UPlayerCharacterMovementComponent::IsMovingOnGround() const
{
	return ((MovementMode == MOVE_Custom && DefaultCustomMovementMode == ECustomMovementType::MOVE_Skateboard) || (MovementMode == MOVE_Walking) || (MovementMode == MOVE_NavWalking)) && UpdatedComponent;
}

void UPlayerCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	PlayerCharacter = Cast<APlayerCharacter>(GetOwner());
	SetMovementMode(EMovementMode::MOVE_Custom, static_cast<int>(DefaultCustomMovementMode));
}

void UPlayerCharacterMovementComponent::TickComponent(float deltaTime, enum ELevelTick TickType, FActorComponentTickFunction* thisTickFunction)
{
	UpdateInput();

	Super::TickComponent(deltaTime, TickType, thisTickFunction);
}

void UPlayerCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

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

void UPlayerCharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	Super::PhysCustom(deltaTime, Iterations);

	switch (CustomMovementMode)
	{
		case static_cast<uint8>(ECustomMovementType::MOVE_Skateboard):
			PhysSkateboard(deltaTime, Iterations);
			break;
		case static_cast<uint8>(ECustomMovementType::MOVE_Grinding):
			PhysGrinding(deltaTime, Iterations);
			break;
		default:
			break;
	}
}

void UPlayerCharacterMovementComponent::PhysSkateboard(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
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
	float RemainingTime = deltaTime;

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
		// Extra velocity for extra adjustments.
		FVector extraVelocity = FVector::ZeroVector;

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
			CurrentSpline.bHasValue = false;
			SetMovementMode(EMovementMode::MOVE_Falling);
			StartNewPhysics(remainingTime, Iterations);
			return;
		}

		auto& SplineRef = *CurrentSpline.SkateboardSplineReference;


		// 1. If just entering the spline, do a setup.
		if (CurrentSpline.PlayerState == static_cast<uint8>(FSplineInfo::STATE_GodKnowsWhere))
		{
			CurrentSpline.PlayerState = static_cast<uint8>(FSplineInfo::STATE_Entering);
			CurrentSpline.StartWorldPos = CharacterOwner->GetActorLocation();
			CurrentSpline.SplinePos = SplineRef.FindInputKeyClosestToWorldLocation(CurrentSpline.StartWorldPos);
			UE_LOG(LogTemp, Warning, TEXT("Started grinding movement! Startingpos: %f"), CurrentSpline.SplinePos);
			extraVelocity = SplineRef.GetLocationAtSplineInputKey(CurrentSpline.SplinePos, ESplineCoordinateSpace::World) - CharacterOwner->GetActorLocation();
			if (!Velocity.IsNearlyZero())
			{
				auto splineDir = SplineRef.GetDirectionAtSplineInputKey(CurrentSpline.SplinePos, ESplineCoordinateSpace::World);
				CurrentSpline.SplineDir = (FVector::DotProduct(splineDir, Velocity) > 0) ? 1 : -1;
			}
		}

		// 2. Find acceleration

		// 3. Find velocity
		FVector SplineWorldPos = SplineRef.GetLocationAtSplineInputKey(CurrentSpline.SplinePos, ESplineCoordinateSpace::World);
		FRotator newRot;
		// Grinding velocity
		if (CurrentSpline.PlayerState == static_cast<uint8>(FSplineInfo::STATE_OnRail))
		{
			float NextSplinePos = CurrentSpline.SplinePos + timeTick * CurrentSpline.SplineDir;
			FVector SplineNextWorldPos;
			// If inside curve, use curve point.
			if (NextSplinePos < CurrentSpline.PointCount)
			{
				SplineNextWorldPos = SplineRef.GetLocationAtSplineInputKey(NextSplinePos, ESplineCoordinateSpace::World);
			}
			// If not inside curve, calculate a curve point using the curvedirection.
			else
			{
				auto dir = SplineRef.GetDirectionAtSplineInputKey(CurrentSpline.SplinePos, ESplineCoordinateSpace::World) * CurrentSpline.SplineDir;
				UE_LOG(LogTemp, Warning, TEXT("dir is %f"), dir.Size());
				SplineNextWorldPos = SplineWorldPos + dir * timeTick;
			}

			// Set new velocity
			Velocity = (SplineNextWorldPos - SplineWorldPos + extraVelocity) / timeTick;
			// Check velocity
			if (Velocity.ContainsNaN())
				Velocity = FVector::ZeroVector;

			newRot = UKismetMathLibrary::MakeRotFromZX(FVector::UpVector, Velocity.IsNearlyZero() ? FVector::ForwardVector : Velocity);
		}
		// Enter velocity
		else if (CurrentSpline.PlayerState == static_cast<uint8>(FSplineInfo::STATE_Entering))
		{
			const FVector EnterDistance = SplineWorldPos - CurrentSpline.StartWorldPos;
			const float vSize = Velocity.Size();
			const float StepsToCurve = EnterDistance.Size() / (Velocity.Size();
			auto dist = SplineWorldPos - GetOwner()->GetActorLocation();
			const bool bVClamped = dist.SizeSquared() < Velocity.SizeSquared() * timeTick;

			// If we needed to clamp velocity to the distance to the spline, we have arrived on the spline.
			if (bVClamped)
			{
				Velocity = dist / timeTick;
				CurrentSpline.PlayerState = static_cast<uint8>(FSplineInfo::STATE_OnRail);
			}
			else
			{
				Velocity = dist.GetSafeNormal() * Velocity.Size();
			}

			// Check velocity
			if (Velocity.ContainsNaN())
				Velocity = FVector::ZeroVector;

			newRot
		}


		



		// 4. Move
		if (Velocity.IsNearlyZero())
		{
			UE_LOG(LogTemp, Warning, TEXT("Calculated velocity is too small!"));
		}
		else
		{
			
			FHitResult Hit(1.f);
			bool bMoveResult = SafeMoveUpdatedComponent(Velocity * timeTick, newRot, true, Hit);
		}

		UE_LOG(LogTemp, Warning, TEXT("Current pos: %f"), CurrentSpline.SplinePos);

		// 5. Check if outside curve.
		if (CurrentSpline.PlayerState != FSplineInfo::STATE_Entering)
		{
			CurrentSpline.SplinePos += timeTick * CurrentSpline.SplineDir;
			if (CurrentSpline.SplinePos >= CurrentSpline.PointCount || CurrentSpline.SplinePos < 0.f)
			{
				CurrentSpline.PlayerState = FSplineInfo::STATE_Leaving;
				// Reset curve
				CurrentSpline.bHasValue = false;

				UE_LOG(LogTemp, Warning, TEXT("Outside of curve, so switching to falling movement."));
				SetMovementMode(EMovementMode::MOVE_Falling);
				StartNewPhysics(remainingTime, Iterations);
			}
		}
	}
}

void UPlayerCharacterMovementComponent::PhysFalling(float deltaTime, int32 Iterations)
{
	Super::PhysFalling(deltaTime, Iterations);

	// Apply rotation based on input
	const auto rotAmount = CalcRotation() * deltaTime;
	if (!FMath::IsNearlyZero(rotAmount))
	{
		GetOwner()->AddActorWorldRotation(FRotator{0.f, rotAmount, 0.f});

		// Set velocity to be facing same direction as forward dir.
		if (!Velocity.IsNearlyZero())
			Velocity = Velocity.RotateAngleAxis(rotAmount, FVector(0, 0, 1));
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

void UPlayerCharacterMovementComponent::TryFallOff()
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

	ApplySkateboardVelocityBraking(DeltaTime, SkateboardForwardGroundDeceleration, SkateboardSidewaysGroundDeceleration);


	ApplyRotation(DeltaTime);
}

void UPlayerCharacterMovementComponent::ApplyRotation(float DeltaTime)
{
	// Apply rotation based on input
	const auto rotAmount = CalcRotation() * DeltaTime;
	GetOwner()->AddActorWorldRotation(FRotator{0.f, rotAmount, 0.f});
	// Rotate velocity the same amount as forward dir.
	if (!IsHandbraking() && !Velocity.IsNearlyZero())
		Velocity = Velocity.RotateAngleAxis(rotAmount, FVector(0, 0, 1));
}

FVector UPlayerCharacterMovementComponent::GetSlopeAcceleration(const FHitResult &FloorHitResult) const
{
	if (!FloorHitResult.bBlockingHit)
		return FVector::ZeroVector;

	auto n = FloorHitResult.Normal;
	FVector u = FVector(0, 0, 1);

	float alpha = FMath::Acos(FVector::DotProduct(n, u));
	float cosAlpha = FMath::Cos(alpha);

	// If it's zero, then there is no acceleration in the horizontal plane, because the slope is vertical.
	if (!cosAlpha)
		return FVector::ZeroVector;

	float N = SlopeGravityMultiplier / cosAlpha;
	float Nx = N * FMath::Cos((PI / 2) - alpha);

	FVector d = FVector::CrossProduct(FVector::CrossProduct(u, n), u).GetSafeNormal();

	FVector a = d * Nx;

	return a;
}

inline float UPlayerCharacterMovementComponent::CalcSidewaysBreaking(const FVector &forward) const
{
	return 1.f - FMath::Abs(FVector::DotProduct(forward, Velocity));
}

float UPlayerCharacterMovementComponent::GetMaxForwardAcceleration() const
{
	return FMath::Max(FMath::Abs(GetMaxAcceleration()) - FVector::DotProduct(Velocity, GetOwner()->GetActorForwardVector()) * SkateboardFwrdVelAccMult, 0.f);
}

bool UPlayerCharacterMovementComponent::CanForwardAccelerate(const FVector &AccelerationIn, float DeltaTime) const
{
	const bool bMovingBackwards = FVector::DotProduct(Velocity, GetOwner()->GetActorForwardVector()) < 0.f;
	return CanForwardAccelerate(AccelerationIn, DeltaTime, bMovingBackwards);
}

bool UPlayerCharacterMovementComponent::CanForwardAccelerate(const FVector &AccelerationIn, float DeltaTime, bool bMovingBackwards) const
{
	return !IsHandbraking() && (bMovingBackwards || (DeltaTime >= MIN_TICK_TIME && (Velocity + AccelerationIn * DeltaTime).SizeSquared() < FMath::Square(CustomMaxAccelerationVelocity)));
}

bool UPlayerCharacterMovementComponent::CanAccelerate(const FVector &AccelerationIn, bool bBrakingIn, float DeltaTime) const
{
	return bBrakingIn || CanForwardAccelerate(AccelerationIn, DeltaTime);
}

bool UPlayerCharacterMovementComponent::CanAccelerate(const FVector &AccelerationIn, bool bBrakingIn, float DeltaTime, bool bMovingBackwards) const
{
	return bBrakingIn || CanForwardAccelerate(AccelerationIn, DeltaTime, bMovingBackwards);
}

FVector UPlayerCharacterMovementComponent::GetInputAcceleration(bool &bBrakingOut, bool &bMovingBackwardsOut, float input)
{
	if (input == 0)
	{
		input = GetForwardInput();
		if (input == 0)
		{
			return FVector::ZeroVector;
		}
	}

	// If input is negative, we are currently braking on the controller.
	bBrakingOut = input < 0.f;
	
	// Scale braking with rotation, 0% rotation equals 100% braking
	if (bBrakingOut)
		input *= 1.f - FMath::Abs(GetRotationInput());

	// Remove vertical input if handbraking and not normal braking with bAllowBrakingWhileHandbraking enabled.
	const bool bCanMoveVertically = !IsHandbraking() || (bAllowBrakingWhileHandbraking && bBrakingOut);
	const float factor = bCanMoveVertically * input * (bBrakingOut ? FMath::Abs(SkateboardBreakingDeceleration) : GetMaxForwardAcceleration());
	auto a = UpdatedComponent->GetForwardVector().GetSafeNormal() * factor;

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

FVector UPlayerCharacterMovementComponent::GetInputAccelerationTimeNormalized(const FVector &a, bool bBrakingIn, float DeltaTime) const
{
	return (bBrakingIn || DeltaTime < MIN_TICK_TIME) ? a : a * (1.f / DeltaTime);
}

FVector UPlayerCharacterMovementComponent::GetClampedInputAcceleration(bool &bBrakingOut, float DeltaTime, float input)
{
	bool bMovingBackwards;
	auto a = GetInputAcceleration(bBrakingOut, bMovingBackwards, input);
	a = GetInputAccelerationTimeNormalized(a, bBrakingOut, DeltaTime);
	return CanAccelerate(a, bBrakingOut, DeltaTime, bMovingBackwards) ? a : FVector::ZeroVector;
}

void UPlayerCharacterMovementComponent::HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	if (Hit.GetActor()->IsA(ATaskObject::StaticClass()))
	{
		Super::HandleImpact(Hit, TimeSlice, MoveDelta);
		return;
	}

	auto angle = FMath::RadiansToDegrees(btd::FastAcos(FMath::Abs(FVector::DotProduct(Velocity.GetSafeNormal(), Hit.ImpactNormal))));

	if (PlayerCharacter && angle < PlayerCharacter->GetCrashAngleThreshold () && Velocity.Size() > PlayerCharacter->GetCrashVelocityFallOffThreshold())
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
	const float standstillRotationSpeed = SkateboardRotationSpeed * SkateboardStandstillRotationSpeed;

	if (IsHandbraking() && !IsFalling())
	{
		float alpha = Velocity.Size() / HandbrakeVelocityThreshold;
		const bool bWithinThreshold = alpha <= 1.f;
		// If above threshold remember to clamp to threshold.
		if (!bWithinThreshold)
			alpha = 1.f;

		const float rotSpeed = FMath::Lerp(standstillRotationSpeed, HandbrakeRotationFactor, alpha);
		return GetHandbrakeAmount() * GetRotationInput() * rotSpeed;
	}
	else
	{
		return GetRotationInput() * (bIsStandstill ? standstillRotationSpeed : SkateboardRotationSpeed);
	}
}