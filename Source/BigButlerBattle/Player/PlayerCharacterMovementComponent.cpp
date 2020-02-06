// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacterMovementComponent.h"
#include "PlayerCharacter.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/SplineComponent.h"

#include "PlayerCharacter.h"

UPlayerCharacterMovementComponent::UPlayerCharacterMovementComponent()
{
	DefaultLandMovementMode = EMovementMode::MOVE_Custom;
	DefaultWaterMovementMode = EMovementMode::MOVE_Custom;
	MaxAcceleration = 2048.f;
	MaxCustomMovementSpeed = 4196.f;
	JumpZVelocity = 600.f;
	AirControl = 0.f;
	AirControlBoostMultiplier = 0.f;
	AirControlBoostVelocityThreshold = 0.f;
	MaxAcceleration = 700.f;

	SetMovementMode(EMovementMode::MOVE_Custom, static_cast<int>(CurrentCustomMovementMode));
}

bool UPlayerCharacterMovementComponent::IsMovingOnGround() const
{
	return ((MovementMode == MOVE_Custom && CurrentCustomMovementMode == ECustomMovementType::MOVE_Skateboard) || (MovementMode == MOVE_Walking) || (MovementMode == MOVE_NavWalking)) && UpdatedComponent;
}

void UPlayerCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	PlayerCharacter = Cast<APlayerCharacter>(GetOwner());
	SetMovementMode(EMovementMode::MOVE_Custom, static_cast<int>(CurrentCustomMovementMode));
}

void UPlayerCharacterMovementComponent::TickComponent(float deltaTime, enum ELevelTick TickType, FActorComponentTickFunction* thisTickFunction)
{
	UpdateInput();

	Super::TickComponent(deltaTime, TickType, thisTickFunction);
}

void UPlayerCharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	Super::PhysCustom(deltaTime, Iterations);

	switch (CustomMovementMode)
	{
		case ECustomMovementType::MOVE_Skateboard:
			PhysSkateboard(deltaTime, Iterations);
			break;
		case ECustomMovementType::MOVE_Grinding:
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

		if (bHandbrake)
			PerformSickAssHandbraking(TimeTick);

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
	while( (remainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations) )
	{
		Iterations++;
		float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
		remainingTime -= timeTick;
		// Extra velocity for extra adjustments.
		FVector extraVelocity = FVector::ZeroVector;


		if (HasAnimRootMotion() || CurrentRootMotion.HasOverrideVelocity())
		{
			UE_LOG(LogTemp, Error, TEXT("Grinding motion doesn't know how to manage root motion!"));
			return;
		}

		if (!IsValid(SkateboardSplineReference) || !IsValid(CharacterOwner))
		{
			return;
		}
		

		// If just entering the spline, do a setup.
		if (SplinePos < 0.f)
		{
			SplinePos = SkateboardSplineReference->FindInputKeyClosestToWorldLocation(CharacterOwner->GetActorLocation());
			extraVelocity = SkateboardSplineReference->GetLocationAtSplineInputKey(SplinePos, ESplineCoordinateSpace::World) - CharacterOwner->GetActorLocation();
			if (!Velocity.IsNearlyZero())
			{
				auto splineDir = SkateboardSplineReference->GetDirectionAtSplineInputKey(SplinePos, ESplineCoordinateSpace::World);
				SplineDir = (FVector::DotProduct(splineDir, Velocity) > 0) ? 1 : -1;
			}
			else
				SplineDir = 1;
		}

		// 1. Find acceleration

		// 2. Find velocity
		FVector SplineWorldPos = SkateboardSplineReference->GetLocationAtSplineInputKey(SplinePos, ESplineCoordinateSpace::World);
		float NextSplinePos = SplinePos + timeTick * SplineDir;
		FVector SplineNextWorldPos;
		// If inside curve, use curve point.
		if (NextSplinePos <= 1.f)
		{
			SplineNextWorldPos = SkateboardSplineReference->GetLocationAtSplineInputKey(NextSplinePos, ESplineCoordinateSpace::World);
		}
		// If not inside curve, calculate a curve point using the curvedirection.
		else
		{
			auto dir = SkateboardSplineReference->GetDirectionAtSplineInputKey(SplinePos, ESplineCoordinateSpace::World) * SplineDir;
			SplineNextWorldPos = SplineWorldPos + dir * timeTick;
		}


		// Set new velocity
		Velocity = (SplineNextWorldPos - SplineWorldPos + extraVelocity) / timeTick;
		if (Velocity.ContainsNaN())
			Velocity = FVector::ZeroVector;

		auto newRot = UKismetMathLibrary::MakeRotFromZX(FVector::UpVector, Velocity.IsNearlyZero() ? FVector::ForwardVector : Velocity);

		if (Velocity.IsNearlyZero())
		{
			UE_LOG(LogTemp, Warning, TEXT("Calculated velocity is too small!"));
		}
		else
		{
			// 3. Move
			FHitResult Hit(1.f);
			auto moveResult = SafeMoveUpdatedComponent(Velocity * timeTick, newRot, true, Hit);
		}


		// 4. Check if outside curve.
		SplinePos += timeTick * SplineDir;
		if (SplinePos > 1.f || SplinePos < 0.f)
		{
			SplinePos = -1.f;

			// SetMovementMode(EMovementMode::MOVE_Custom, static_cast<int>(CurrentCustomMovementMode));
			SetMovementMode(EMovementMode::MOVE_Falling);
			StartNewPhysics(remainingTime, Iterations);
		}


		if (!HasValidData())
		{
			return;
		}
	}
}

void UPlayerCharacterMovementComponent::PhysFalling(float deltaTime, int32 Iterations)
{
	Super::PhysFalling(deltaTime, Iterations);

	// Apply rotation based on input
	const auto rotAmount = CalcRotation() * deltaTime;
	GetOwner()->AddActorWorldRotation(FRotator{0.f, rotAmount, 0.f});

	// Set velocity to be facing same direction as forward dir.
	if (!Velocity.IsNearlyZero())
		Velocity = Velocity.RotateAngleAxis(rotAmount, FVector(0, 0, 1));
}

void UPlayerCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	if (MovementMode == MOVE_Custom && CustomMovementMode == static_cast<int>(ECustomMovementType::MOVE_None) && CurrentCustomMovementMode != ECustomMovementType::MOVE_None)
		SetMovementMode(MOVE_Custom, static_cast<int>(CurrentCustomMovementMode));
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

void UPlayerCharacterMovementComponent::PerformSickAssHandbraking(float DeltaTime)
{
	if (Velocity.Size() > HandbrakeVelocityThreshold && !IsFalling())
		UpdatedComponent->AddLocalRotation({0.f, CalcHandbrakeRotation() * DeltaTime, 0.f});	
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

	const bool bIsStandstill = Velocity.Size() < StandstillThreshold;
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

	// Apply rotation based on input
	auto forwardDir = GetOwner()->GetActorForwardVector();
	const auto rotAmount = CalcRotation() * ((bIsStandstill) ? SkateboardStandstillRotationSpeed : 1.f) * DeltaTime;
	GetOwner()->AddActorWorldRotation(FRotator{0.f, rotAmount, 0.f});
	// Rotate velocity the same amount as forward dir.
	if (!Velocity.IsNearlyZero())
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

bool UPlayerCharacterMovementComponent::CanForwardAccelerate(const FVector &AccelerationIn, float DeltaTime) const
{
	const bool bMovingBackwards = FVector::DotProduct(Velocity, GetOwner()->GetActorForwardVector()) < 0.f;
	return CanForwardAccelerate(AccelerationIn, DeltaTime, bMovingBackwards);
}

bool UPlayerCharacterMovementComponent::CanForwardAccelerate(const FVector &AccelerationIn, float DeltaTime, bool bMovingBackwards) const
{
	return !bHandbrake && (bMovingBackwards || (DeltaTime >= MIN_TICK_TIME && (Velocity + AccelerationIn * DeltaTime).SizeSquared() < FMath::Square(CustomMaxAccelerationVelocity)));
}

bool UPlayerCharacterMovementComponent::CanAccelerate(const FVector &AccelerationIn, bool bBrakingIn, float DeltaTime) const
{
	return bBrakingIn || CanForwardAccelerate(AccelerationIn, DeltaTime);
}

bool UPlayerCharacterMovementComponent::CanAccelerate(const FVector &AccelerationIn, bool bBrakingIn, float DeltaTime, bool bMovingBackwards) const
{
	return bBrakingIn || CanForwardAccelerate(AccelerationIn, DeltaTime, bMovingBackwards);
}

FVector UPlayerCharacterMovementComponent::GetInputAcceleration(bool &bBreakingOut, bool &bMovingBackwardsOut, float input)
{
	if (input == 0)
	{
		input = GetForwardInput();
		if (input == 0)
		{
			return FVector::ZeroVector;
		}
	}

	// Remove vertical input if handbraking and not normal braking with bAllowBrakingWhileHandbraking enabled.
	const bool bCanMoveVertically = !bHandbrake || (bAllowBrakingWhileHandbraking && input < 0.f);
	const float factor = bCanMoveVertically * input * ((input >= 0) ? FMath::Abs(GetMaxAcceleration()) : SkateboardBreakingDeceleration);
	auto a = UpdatedComponent->GetForwardVector().GetSafeNormal() * factor;

	if (a.IsNearlyZero())
		a = FVector::ZeroVector;

	bBreakingOut = FMath::IsNegativeFloat(FVector::DotProduct(a, GetOwner()->GetActorForwardVector()));
	bMovingBackwardsOut = FVector::DotProduct(Velocity, GetOwner()->GetActorForwardVector()) < 0.f;

	// If we have backwards velocity we need to flip acceleration so we still brake (backwards acceleration should be forward until velocity is 0 again)
	if (bBreakingOut && bMovingBackwardsOut)
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
	if (PlayerCharacter && Velocity.Size() > PlayerCharacter->GetCrashVelocityFallOffThreshold())
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
	// Remove horizontal input if handbraking and not in air.
	const bool bCanMoveHorizontally = !bHandbrake || IsFalling();
	return bCanMoveHorizontally * SkateboardRotationSpeed * GetRotationInput();
}

float UPlayerCharacterMovementComponent::CalcHandbrakeRotation() const
{
	const bool bShouldHandbrake = bHandbrake && !IsFalling();
	return bShouldHandbrake * HandbrakeRotationFactor * GetRotationInput();
}
