// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacterMovementComponent.h"
#include "PlayerCharacter.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/KismetMathLibrary.h"

#include "PlayerCharacter.h"

UPlayerCharacterMovementComponent::UPlayerCharacterMovementComponent()
{
	DefaultLandMovementMode = EMovementMode::MOVE_Custom;
	DefaultWaterMovementMode = EMovementMode::MOVE_Custom;
	MaxAcceleration = 400.f;
	MaxCustomMovementSpeed = 2048.f;
	JumpZVelocity = 600.f;

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
	InputDir = GetPendingInputVector();

	Super::TickComponent(deltaTime, TickType, thisTickFunction);
}

void UPlayerCharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	Super::PhysCustom(deltaTime, Iterations);

	if (CustomMovementMode)
	{
		PhysSkateboard(deltaTime, Iterations);
	}
}

void UPlayerCharacterMovementComponent::PhysSkateboard(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	if (Velocity.ContainsNaN())
		UE_LOG(LogTemp, Error, TEXT("PhysSkateboard: Velocity contains NaN before Iteration (%s)\n%s"), *GetPathNameSafe(this))

	bStandstill = Velocity.Size() < StandstillThreshold;

	if (!CharacterOwner || (!CharacterOwner->GetController() && !bRunPhysicsWithNoController && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy)))
	{
		Acceleration = FVector::ZeroVector;
		Velocity = FVector::ZeroVector;
		return;
	}

	bJustTeleported = false;
	bool bCheckedFall = false;
	float RemainingTime = deltaTime;

	// Perform the move
	while ((RemainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations) && CharacterOwner && (CharacterOwner->Controller || bRunPhysicsWithNoController || HasAnimRootMotion() || CurrentRootMotion.HasOverrideVelocity() || (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)))
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

		RestorePreAdditiveRootMotionVelocity();

		// Ensure velocity is horizontal.
		MaintainHorizontalGroundVelocity();

		CalcSkateboardVelocity(TimeTick);
		if (Velocity.ContainsNaN())
			UE_LOG(LogTemp, Error, TEXT("PhysSkateboard: Velocity contains NaN after CalcVelocity (%s)\n%s"), *GetPathNameSafe(this))


		if (PlayerCharacter && PlayerCharacter->CanFall() && SidewaysForce > PlayerCharacter->GetSidewaysForceFallOffThreshold())
		{
			PlayerCharacter->EnableRagdoll();
		}

		ApplyRootMotionToVelocity(TimeTick);
		if (Velocity.ContainsNaN())
			UE_LOG(LogTemp, Error, TEXT("PhysSkateboard: Velocity contains NaN after Root Motion application (%s)\n%s"), *GetPathNameSafe(this))

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

		// Note: Commented out because it cancels velocity if teleported, but teleportation bool didn't change anyway. Meaning it's useless.
		//// Allow overlap events and such to change physics state and velocity
		//if (IsMovingOnGround())
		//{
		//	// Make velocity reflect actual move
		//	if (!bJustTeleported && timeTick >= MIN_TICK_TIME)
		//	{
		//		// TODO-RootMotionSource: Allow this to happen during partial override Velocity, but only set allowed axes?
		//		Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / timeTick;
		//	}
		//}

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

void UPlayerCharacterMovementComponent::PhysFalling(float deltaTime, int32 Iterations)
{
	Super::PhysFalling(deltaTime, Iterations);

	// Apply rotation based on input
	auto forwardDir = GetOwner()->GetActorForwardVector();
	const auto rotAmount = CalcRotation() * deltaTime;
	GetOwner()->AddActorWorldRotation(FRotator{ 0.f, rotAmount, 0.f });

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

void UPlayerCharacterMovementComponent::ApplyVelocityBraking(float DeltaTime, float Friction, float BreakingForwardDeceleration, float BreakingSidewaysDeceleration)
{
	if (Velocity.IsZero() || !HasValidData() || HasAnimRootMotion() || DeltaTime < MIN_TICK_TIME)
	{
		return;
	}

	const float FrictionFactor = FMath::Max(0.f, BrakingFrictionFactor);
	Friction = FMath::Max(0.f, Friction * FrictionFactor);
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

void UPlayerCharacterMovementComponent::CalcSkateboardVelocity(float DeltaTime)
{
	// Do not update velocity when using root motion or when SimulatedProxy and not simulating root motion - SimulatedProxy are repped their Velocity
	if (!HasValidData() || HasAnimRootMotion() || DeltaTime < MIN_TICK_TIME || (CharacterOwner && CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy && !bWasSimulatingRootMotion))
	{
		return;
	}

	const float MaxAcceleration = GetMaxAcceleration();
	float MaxSpeed = GetMaxSpeed();

	Acceleration = CalcAcceleration();

	// Check if path following requested movement
	bool bZeroRequestedAcceleration = true;
	FVector RequestedAcceleration = FVector::ZeroVector;
	float RequestedSpeed = 0.0f;
	if (ApplyRequestedMove(DeltaTime, MaxAcceleration, MaxSpeed, BrakingFriction, SkateboardForwardGroundDeceleration, RequestedAcceleration, RequestedSpeed))
	{
		bZeroRequestedAcceleration = false;
	}

	if (bForceMaxAccel)
	{
		
		// Force acceleration at full speed.
		// In consideration order for direction: Acceleration, then Velocity, then Pawn's rotation.
		if (Acceleration.SizeSquared() > SMALL_NUMBER)
		{
			Acceleration = Acceleration.GetSafeNormal() * MaxAcceleration;
		}
		else
		{
			Acceleration = MaxAcceleration * (Velocity.SizeSquared() < SMALL_NUMBER ? UpdatedComponent->GetForwardVector() : Velocity.GetSafeNormal());
		}

		AnalogInputModifier = 1.f;
	}

	// Path following above didn't care about the analog modifier, but we do for everything else below, so get the fully modified value.
	// Use max of requested speed and max speed if we modified the speed in ApplyRequestedMove above.
	const float MaxInputSpeed = FMath::Max(MaxSpeed * AnalogInputModifier, GetMinAnalogSpeed());
	MaxSpeed = FMath::Max(RequestedSpeed, MaxInputSpeed);

	// Apply braking or deceleration
	const bool bZeroAcceleration = Acceleration.IsZero();
	const bool bVelocityOverMax = IsExceedingMaxSpeed(MaxSpeed);
	
	ApplyVelocityBraking(DeltaTime, BrakingFriction, SkateboardForwardGroundDeceleration, SkateboardSidewaysGroundDeceleration);
	const FVector OldVelocity = Velocity;
	// Don't allow braking to lower us below max speed if we started above it.
	if (bVelocityOverMax && Velocity.SizeSquared() < FMath::Square(MaxSpeed) && FVector::DotProduct(Acceleration, OldVelocity) > 0.0f)
	{
		Velocity = OldVelocity.GetSafeNormal() * MaxSpeed;
	}
	
	//else if (!bZeroAcceleration)
	//{
	//	// Friction affects our ability to change direction. This is only done for input acceleration, not path following.
	//	const FVector AccelDir = Acceleration.GetSafeNormal();
	//	const float VelSize = Velocity.Size();
	//	Velocity = Velocity - (Velocity - AccelDir * VelSize) * FMath::Min(DeltaTime * SkateboardGroundFriction, 1.f);
	//}

	const bool bIsStandstill = Velocity.Size() < StandstillThreshold;

	// Apply acceleration if there is any, and a braking deceleration if trying to reverse
	if(!bZeroAcceleration)
	{
		bool bNegativeAcceleration = FMath::IsNegativeFloat(FVector::DotProduct(Acceleration, GetOwner()->GetActorForwardVector()));
		const bool bMovingBackwards = FVector::DotProduct(Velocity, GetOwner()->GetActorForwardVector()) < 0.f;
		
		// If we have backwards velocity we need to flip acceleration so we still brake (backwards acceleration should be forward until velocity is 0 again)
		if (bNegativeAcceleration && bMovingBackwards)
		{
			Acceleration = -Acceleration;
			bNegativeAcceleration = !bNegativeAcceleration;
		}

		const bool bShouldStopCompletely = bNegativeAcceleration && bIsStandstill;
				
		if (bShouldStopCompletely)
		{
			Velocity = FVector::ZeroVector;
		}
		else
		{
			const float NewMaxInputSpeed = IsExceedingMaxSpeed(MaxInputSpeed) ? Velocity.Size() : MaxInputSpeed;
			Velocity += Acceleration * DeltaTime;
			Velocity = Velocity.GetClampedToMaxSize(NewMaxInputSpeed);
		}
	}

	// Apply rotation based on input
	auto forwardDir = GetOwner()->GetActorForwardVector();
	const auto rotAmount = CalcRotation() * ((bIsStandstill) ? SkateboardStandstillRotationSpeed : 1.f) * DeltaTime;
	GetOwner()->AddActorWorldRotation(FRotator{ 0.f, rotAmount, 0.f});

	// Set velocity to be facing same direction as forward dir.
	if (!Velocity.IsNearlyZero())
		Velocity = Velocity.RotateAngleAxis(rotAmount, FVector(0, 0, 1));

	// Apply additional requested acceleration
	if (!bZeroRequestedAcceleration)
	{
		const float NewMaxRequestedSpeed = IsExceedingMaxSpeed(RequestedSpeed) ? Velocity.Size() : RequestedSpeed;
		Velocity += RequestedAcceleration * DeltaTime;
		Velocity = Velocity.GetClampedToMaxSize(NewMaxRequestedSpeed);
	}

	if (bUseRVOAvoidance)
	{
		CalcAvoidanceVelocity(DeltaTime);
	}
}

inline float UPlayerCharacterMovementComponent::CalcSidewaysBreaking(const FVector& forward) const
{
	return 1.f - FMath::Abs(FVector::DotProduct(forward, Velocity));
}

inline FVector UPlayerCharacterMovementComponent::CalcAcceleration() const
{
	const auto input = GetForwardInput();
	const float factor = input * ((input >= 0) ? FMath::Abs(GetMaxAcceleration()) : SkateboardBreakingDeceleration);
	return UpdatedComponent->GetForwardVector().GetSafeNormal() * factor;
}

float UPlayerCharacterMovementComponent::CalcRotation() const
{
	return SkateboardRotationSpeed * GetRotationInput();
}

FVector UPlayerCharacterMovementComponent::ClampForwardVelocity()
{
	return FVector();
}
