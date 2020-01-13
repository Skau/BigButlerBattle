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
		Params.MaxSimTime = 2.f;
		Params.StartLocation = Start;
		Params.TraceChannel = ECollisionChannel::ECC_WorldStatic;
		Params.bTraceWithChannel = true;
		Params.bTraceWithCollision = true;
		
		FPredictProjectilePathResult Result;
		if (UGameplayStatics::PredictProjectilePath(GetWorld(), Params, Result))
		{
			FVector LandNormal = Result.HitResult.ImpactNormal;
			float Angle = ABigButlerBattleGameModeBase::GetAngleBetweenNormals(LandNormal, SkateboardMesh->GetUpVector());

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

		float TraceDistance = 200.f;
		FHitResult FrontResult;
		FVector Start = LinetraceFront.GetLocation() + (FVector(0, 0, 1) * TraceDistance);
		FVector End = LinetraceFront.GetLocation() + (FVector(0, 0, 1) * -TraceDistance);
		GetWorld()->LineTraceSingleByObjectType(FrontResult, Start, End, ObjParams);

		FHitResult BackResult;
		Start = LinetraceBack.GetLocation() + (FVector(0, 0, 1) * TraceDistance);
		End = LinetraceBack.GetLocation() + (FVector(0, 0, 1) * -TraceDistance);
		GetWorld()->LineTraceSingleByObjectType(BackResult, Start, End, ObjParams);

		/// Fint normals:
		// Both hits:
		if (FrontResult.bBlockingHit && BackResult.bBlockingHit)
		{
			auto tangentDot = FVector::DotProduct(FVector::CrossProduct(FrontResult.Normal, BackResult.Normal), GetActorRightVector());

			// 1. Get new normals
			// 2. Do dot product
			// 3. The bigger the angle the faster the interpolation

			auto dot = FVector::DotProduct(FrontResult.Normal, BackResult.Normal);
			if (dot != 0)
			{
				auto newNormal = (FrontResult.Normal + BackResult.Normal).GetSafeNormal();
				auto DesiredRotation = GetDesiredRotation(newNormal);
				SkateboardMesh->SetWorldRotation(FQuat::Slerp(SkateboardMesh->GetComponentQuat(), DesiredRotation, (SkateboardRotationGroundSpeed / 0.017f) * DeltaTime * dot));
			}
		}
		// One hit:
		else if (FrontResult.bBlockingHit || BackResult.bBlockingHit)
		{
			auto &result = FrontResult.bBlockingHit ? FrontResult : BackResult;

			auto DesiredRotation = GetDesiredRotation(result.Normal);
			SkateboardMesh->SetWorldRotation(FQuat::Slerp(SkateboardMesh->GetComponentQuat(), DesiredRotation, (SkateboardRotationGroundSpeed / 0.017f) * DeltaTime));
		}
		// No hits:
		else
		{
			// No hits. Default normal.
			SkateboardMesh->SetWorldRotation(FQuat::Slerp(SkateboardMesh->GetComponentQuat(), FQuat::Identity, (SkateboardRotationGroundSpeed / 0.017f) * DeltaTime));
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
