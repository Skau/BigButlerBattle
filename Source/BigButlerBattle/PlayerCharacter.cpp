// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "PlayerCharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "BigButlerBattleGameModeBase.h"
#include "Engine/SkeletalMeshSocket.h"
#include "DrawDebugHelpers.h"

APlayerCharacter::APlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: ACharacter(ObjectInitializer.SetDefaultSubobjectClass<UPlayerCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;

	SkateboardMesh = CreateDefaultSubobject<USkeletalMeshComponent>("Skateboard Mesh");
	SkateboardMesh->SetupAttachment(RootComponent);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>("Spring Arm");
	SpringArm->SetupAttachment(RootComponent);

	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

	bUseControllerRotationYaw = false;
}

void APlayerCharacter::EnableRagdoll()
{
	if (!bCanFall || bEnabledRagdoll)
		return;

	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetAllBodiesSimulatePhysics(true);
	//bEnabledRagdoll = true;
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (!SkateboardMesh || !IsValid(SkateboardMesh))
	{
		UE_LOG(LogTemp, Error, TEXT("SkateboardMesh is not valid!"));
		return;
	}

	LinetraceSocketFront = SkateboardMesh->GetSocketByName("LinetraceFront");
	LinetraceSocketBack = SkateboardMesh->GetSocketByName("LinetraceBack");

	Movement = Cast<UPlayerCharacterMovementComponent>(GetMovementComponent());
	check(Movement != nullptr);
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bEnabledRagdoll && bCurrentlyHoldingHandbrake && Movement->Velocity.Size() > HandbreakeVelocityThreshold && !Movement->IsFalling())
	{
		AddActorLocalRotation({ 0, RightAxis * DeltaTime * HandbrakeRotationFactor, 0 });
	}

	UpdateSkateboardRotation(DeltaTime);
}

void APlayerCharacter::UpdateSkateboardRotation(float DeltaTime)
{
	if (!LinetraceSocketFront || !IsValid(LinetraceSocketFront) || !LinetraceSocketBack ||!IsValid(LinetraceSocketBack))
	{
		UE_LOG(LogTemp, Error, TEXT("Linetrace sockets not good"));
		return;
	}

	FTransform LinetraceFront = LinetraceSocketFront->GetSocketTransform(SkateboardMesh);
	FTransform LinetraceBack = LinetraceSocketBack->GetSocketTransform(SkateboardMesh);

	// Case 1: In air
	if (GetMovementComponent()->IsFalling())
	{
		FVector Start = LinetraceFront.GetLocation() + (LinetraceBack.GetLocation() - LinetraceFront.GetLocation()) * 0.5f;

		FPredictProjectilePathParams Params;
		Params.ActorsToIgnore.Add(this);
		Params.LaunchVelocity = GetMovementComponent()->Velocity;
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
			FVector LandNormal = Result.HitResult.ImpactNormal;
			float Angle = ABigButlerBattleGameModeBase::GetAngleBetweenNormals(LandNormal, SkateboardMesh->GetUpVector());

			if (bDebugMovement)
				DrawDebugSphere(GetWorld(), Result.HitResult.ImpactPoint, 10.f, 10, FColor::Green, false, 0.1f);

			if(Angle < Movement->GetWalkableFloorAngle())
			{
				auto DesiredRotation = GetDesiredRotation(LandNormal);
				SkateboardMesh->SetWorldRotation(FQuat::Slerp(SkateboardMesh->GetComponentQuat(), DesiredRotation, (SkateboardRotationAirSpeed / 0.017f) * DeltaTime));
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
				auto newNormal = (FrontResult.ImpactNormal + BackResult.ImpactNormal).GetSafeNormal();
				auto DesiredRotation = GetDesiredRotation(newNormal);
				SkateboardMesh->SetWorldRotation(FQuat::Slerp(SkateboardMesh->GetComponentQuat(), DesiredRotation, (SkateboardRotationGroundSpeed / 0.017f) * DeltaTime * dot));
			}
			
			// There is a case here where we should fall, because if the player handbrakes over an edge it is still glued to the slope and it looks weird.
			// If some combination of normals changing here is true, we need to switch movement mode to falling.
		}
		// One hit:
		else if (FrontResult.bBlockingHit || BackResult.bBlockingHit)
		{	
			auto &result = FrontResult.bBlockingHit ? FrontResult : BackResult;
			auto DesiredRotation = GetDesiredRotation(result.ImpactNormal);
			SkateboardMesh->SetWorldRotation(FQuat::Slerp(SkateboardMesh->GetComponentQuat(), DesiredRotation, (SkateboardRotationGroundSpeed / 0.017f) * DeltaTime));

			// Turn on falling just in case we are at an edge/incline and the velocity is great enough to get some air
			Movement->SetMovementMode(EMovementMode::MOVE_Falling);
		}
		// No hits: 
		else
		{
			// Turn on falling, because we have no idea where the ground is and we are definitely in the air.
			Movement->SetMovementMode(EMovementMode::MOVE_Falling);
		}
	}
}


FQuat APlayerCharacter::GetDesiredRotation(FVector DestinationNormal) const
{
	FVector Right = FVector::CrossProduct(DestinationNormal, GetActorForwardVector());
	FVector Forward = FVector::CrossProduct(GetActorRightVector(), DestinationNormal); 

	FRotator Rot = UKismetMathLibrary::MakeRotFromXY(Forward, Right);

	return Rot.Quaternion();
}
