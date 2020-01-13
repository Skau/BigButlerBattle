// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacterMovementComponent.h"
#include "PlayerCharacter.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/SplineComponent.h"
#include "PlayerCharacter.h"
#include "DrawDebugHelpers.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStaticsTypes.h"
#include "Kismet/GameplayStatics.h"
#include "BigButlerBattleGameModeBase.h"
#include "Components/SkeletalMeshComponent.h"

UPlayerCharacterMovementComponent::UPlayerCharacterMovementComponent()
{
	DefaultLandMovementMode = EMovementMode::MOVE_Custom;
	DefaultWaterMovementMode = EMovementMode::MOVE_Custom;
	MaxAcceleration = 2048.f;
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

		//bool bShouldFall = UpdateMeshRotation(RemainingTime);
		//if (bShouldFall)
		//{
		//	SetMovementMode(EMovementMode::MOVE_Falling);
		//	StartNewPhysics(RemainingTime, Iterations);
		//	break;
		//}

		// Ensure velocity is horizontal.
		MaintainHorizontalGroundVelocity();

		AdjustSlopeVelocity(OldFloor.HitResult, deltaTime);
		CalcSkateboardVelocity(TimeTick);

		check(!Velocity.ContainsNaN());

		if (PlayerCharacter && PlayerCharacter->CanFall() && SidewaysForce > PlayerCharacter->GetSidewaysForceFallOffThreshold())
			PlayerCharacter->EnableRagdoll();

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
	auto forwardDir = GetOwner()->GetActorForwardVector();
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

void UPlayerCharacterMovementComponent::CalcSkateboardVelocity(float DeltaTime)
{
	// Do not update velocity when using root motion or when SimulatedProxy and not simulating root motion - SimulatedProxy are repped their Velocity
	if (!HasValidData() || HasAnimRootMotion() || DeltaTime < MIN_TICK_TIME || (CharacterOwner && CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy && !bWasSimulatingRootMotion))
	{
		return;
	}

	float MaxSpeed = GetMaxSpeed();
	Acceleration = CalcAcceleration();

	// Get the fully modified analog input value.
	MaxSpeed = FMath::Max(MaxSpeed * AnalogInputModifier, GetMinAnalogSpeed());

	// Apply braking or deceleration
	const bool bZeroAcceleration = Acceleration.IsZero();
	const bool bVelocityOverMax = IsExceedingMaxSpeed(MaxSpeed);

	ApplySkateboardVelocityBraking(DeltaTime, SkateboardForwardGroundDeceleration, SkateboardSidewaysGroundDeceleration);

	const FVector OldVelocity = Velocity;
	// Don't allow braking to lower us below max speed if we started above it.
	if (bVelocityOverMax && Velocity.SizeSquared() < FMath::Square(MaxSpeed) && FVector::DotProduct(Acceleration, OldVelocity) > 0.0f)
	{
		Velocity = OldVelocity.GetSafeNormal() * MaxSpeed;
	}

	const bool bIsStandstill = Velocity.Size() < StandstillThreshold;

	// Apply acceleration if there is any, and a braking deceleration if trying to reverse
	if (!bZeroAcceleration)
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
			const float NewMaxSpeed = IsExceedingMaxSpeed(MaxSpeed) ? Velocity.Size() : MaxSpeed;
			Velocity += Acceleration * DeltaTime;
			Velocity = Velocity.GetClampedToMaxSize(NewMaxSpeed);
		}
	}

	// Apply rotation based on input
	auto forwardDir = GetOwner()->GetActorForwardVector();
	const auto rotAmount = CalcRotation() * ((bIsStandstill) ? SkateboardStandstillRotationSpeed : 1.f) * DeltaTime;
	GetOwner()->AddActorWorldRotation(FRotator{0.f, rotAmount, 0.f});
	// Set velocity to be facing same direction as forward dir.
	if (!Velocity.IsNearlyZero())
		Velocity = Velocity.RotateAngleAxis(rotAmount, FVector(0, 0, 1));
}

void UPlayerCharacterMovementComponent::AdjustSlopeVelocity(FHitResult FloorHitResult, float DeltaTime)
{
	if (!FloorHitResult.bBlockingHit)
		return;

	auto n = FloorHitResult.Normal;
	FVector u = FVector(0, 0, 1);

	float alpha = FMath::Acos(FVector::DotProduct(n, u));
	float cosAlpha = FMath::Cos(alpha);

	// If it's zero, then there is no acceleration in the horizontal plane, because the slope is vertical.
	if (!cosAlpha)
		return;

	float N = SlopeGravityMultiplier / cosAlpha;
	float Nx = N * FMath::Cos((PI / 2) - alpha);

	FVector d = FVector::CrossProduct(FVector::CrossProduct(u, n), u).GetSafeNormal();

	FVector a = d * Nx;

	Velocity += a * DeltaTime;
}

inline float UPlayerCharacterMovementComponent::CalcSidewaysBreaking(const FVector &forward) const
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

bool UPlayerCharacterMovementComponent::UpdateMeshRotation(float deltaTime)
{
	auto Mesh = PlayerCharacter->GetSkateboardMesh();
	FTransform LinetraceFront = LinetraceSocketFront->GetSocketTransform(Mesh);
	FTransform LinetraceBack = LinetraceSocketBack->GetSocketTransform(Mesh);

	bool bShouldChangeToFalling = false;
	
	// Case 1: In air
	if (IsFalling())
	{
		FVector Start = LinetraceFront.GetLocation() + (LinetraceBack.GetLocation() - LinetraceFront.GetLocation()) * 0.5f;

		FPredictProjectilePathParams Params;
		Params.LaunchVelocity = Velocity;
		Params.MaxSimTime = 1.f;
		Params.StartLocation = Start;
		Params.TraceChannel = ECollisionChannel::ECC_WorldStatic;
		Params.bTraceWithChannel = true;
		Params.bTraceWithCollision = true;
		if (bDebugMovement)
		{
			Params.DrawDebugTime = 0.1f;
			Params.DrawDebugType = EDrawDebugTrace::ForDuration;
		}

		FPredictProjectilePathResult Result;
		if (UGameplayStatics::PredictProjectilePath(GetWorld(), Params, Result))
		{
			auto SkateboardMesh = PlayerCharacter->GetSkateboardMesh();
			FVector LandNormal = Result.HitResult.ImpactNormal;
			float Angle = ABigButlerBattleGameModeBase::GetAngleBetweenNormals(LandNormal, SkateboardMesh->GetUpVector());

			if (bDebugMovement)
				DrawDebugSphere(GetWorld(), Result.HitResult.ImpactPoint, 10.f, 10, FColor::Green, false, 0.1f);

			if (Angle < GetWalkableFloorAngle())
			{
				auto DesiredRotation = GetDesiredRotation(LandNormal);
				SkateboardMesh->SetWorldRotation(FQuat::Slerp(SkateboardMesh->GetComponentQuat(), DesiredRotation, (SkateboardSlopeRotationAirSpeed / 0.017f) * deltaTime));
			}
		}
	}
	// Case 2/3: On Ground
	else
	{
		FCollisionObjectQueryParams ObjParams;
		ObjParams.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldStatic);

		// Making the trace distance really short makes it happen more often over inclines that only 1 or 0 hits are recorded.
		// This directly affects the rate of which we change to the falling movement mode. This definitely needs more tweaking.
		// Use the bDebugMovement bool in the editor to visualize this (and the projectile prediction above).
		float TraceDistance = 20.f;

		FHitResult FrontResult;
		FVector Start = LinetraceFront.GetLocation() + (FVector(0, 0, 1) * TraceDistance);
		FVector End = LinetraceFront.GetLocation() + (FVector(0, 0, 1) * -TraceDistance);

		GetWorld()->LineTraceSingleByObjectType(FrontResult, Start, End, ObjParams);

		if (bDebugMovement)
			DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 0.1f);

		FHitResult BackResult;
		Start = LinetraceBack.GetLocation() + (FVector(0, 0, 1) * TraceDistance);
		End = LinetraceBack.GetLocation() + (FVector(0, 0, 1) * -TraceDistance);
		GetWorld()->LineTraceSingleByObjectType(BackResult, Start, End, ObjParams);

		if (bDebugMovement)
			DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 0.1f);

		if (bDebugMovement && FrontResult.bBlockingHit)
			DrawDebugSphere(GetWorld(), FrontResult.ImpactPoint, 10.f, 10, FColor::Green, false, 0.1f);

		if (bDebugMovement && BackResult.bBlockingHit)
			DrawDebugSphere(GetWorld(), BackResult.ImpactPoint, 10.f, 10, FColor::Green, false, 0.1f);

		// Both hits:
		if (FrontResult.bBlockingHit && BackResult.bBlockingHit)
		{
			auto dot = FVector::DotProduct(FrontResult.ImpactNormal, BackResult.ImpactNormal);
			if (dot != 0)
			{
				auto SkateboardMesh = PlayerCharacter->GetSkateboardMesh();
				auto newNormal = (FrontResult.ImpactNormal + BackResult.ImpactNormal).GetSafeNormal();
				auto DesiredRotation = GetDesiredRotation(newNormal);
				SkateboardMesh->SetWorldRotation(FQuat::Slerp(SkateboardMesh->GetComponentQuat(), DesiredRotation, (SkateboardRotationSlopeGroundSpeed / 0.017f) * deltaTime * dot));
			}

			// There is a case here where we should fall, because if the player handbrakes over an edge it is still glued to the slope and it looks weird.
			// If some combination of normals changing here is true, we need to switch movement mode to falling.
		}
		// One hit:
		else if (FrontResult.bBlockingHit || BackResult.bBlockingHit)
		{
			auto SkateboardMesh = PlayerCharacter->GetSkateboardMesh();
			auto& result = FrontResult.bBlockingHit ? FrontResult : BackResult;
			auto DesiredRotation = GetDesiredRotation(result.ImpactNormal);
			SkateboardMesh->SetWorldRotation(FQuat::Slerp(SkateboardMesh->GetComponentQuat(), DesiredRotation, (SkateboardRotationSlopeGroundSpeed / 0.017f) * deltaTime));

			// Turn on falling just in case we are at an edge/incline and the velocity is great enough to get some air
			bShouldChangeToFalling = true;
		}
		// No hits: 
		else
		{
			// Turn on falling, because we have no idea where the ground is and we are definitely in the air.
			bShouldChangeToFalling = true;
		}
	}
	return bShouldChangeToFalling;
}

FQuat UPlayerCharacterMovementComponent::GetDesiredRotation(FVector DestinationNormal) const
{
	FVector Right = FVector::CrossProduct(DestinationNormal, PlayerCharacter->GetActorForwardVector());
	FVector Forward = FVector::CrossProduct(PlayerCharacter->GetActorRightVector(), DestinationNormal);

	FRotator Rot = UKismetMathLibrary::MakeRotFromXY(Forward, Right);

	return Rot.Quaternion();
}
