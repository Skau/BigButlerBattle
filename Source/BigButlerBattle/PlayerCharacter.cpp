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

APlayerCharacter::APlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: ACharacter(ObjectInitializer.SetDefaultSubobjectClass<UPlayerCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;

	SkateboardMesh = CreateDefaultSubobject<USkeletalMeshComponent>("Skateboard Mesh");
	SkateboardMesh->SetupAttachment(RootComponent);

	LinetraceFront = CreateDefaultSubobject<USceneComponent>("Linetrace Front");
	LinetraceFront->SetupAttachment(SkateboardMesh);

	LinetraceBack = CreateDefaultSubobject<USceneComponent>("Linetrace Back");
	LinetraceBack->SetupAttachment(SkateboardMesh);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>("Spring Arm");
	SpringArm->SetupAttachment(RootComponent);

	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

	TempSkateboardMesh = CreateDefaultSubobject<UStaticMeshComponent>("Temp Skateboard Mesh");
	TempSkateboardMesh->SetupAttachment(RootComponent);


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
	// Case 1: In air
	if (GetMovementComponent()->IsFalling())
	{
		FVector Start = LinetraceFront->GetComponentLocation() + (LinetraceBack->GetComponentLocation() - LinetraceFront->GetComponentLocation()) * 0.5f;

		FPredictProjectilePathParams Params;
		Params.ActorsToIgnore.Add(this);
		Params.LaunchVelocity = GetMovementComponent()->Velocity;
		Params.MaxSimTime = 1.f;
		Params.StartLocation = Start;
		Params.TraceChannel = ECollisionChannel::ECC_WorldStatic;
		Params.bTraceWithChannel = true;
		Params.bTraceWithCollision = true;
		Params.DrawDebugTime = 0.5f;
		Params.DrawDebugType = EDrawDebugTrace::ForOneFrame;
		
		FPredictProjectilePathResult Result;
		if (UGameplayStatics::PredictProjectilePath(GetWorld(), Params, Result))
		{
			FVector LandNormal = Result.HitResult.ImpactNormal;
			float Angle = ABigButlerBattleGameModeBase::GetAngleBetweenNormals(LandNormal, TempSkateboardMesh->GetUpVector());

			if(Angle < Movement->GetWalkableFloorAngle())
			{
				auto DesiredRotation = GetDesiredRotation(LandNormal);
				TempSkateboardMesh->SetWorldRotation(FQuat::Slerp(TempSkateboardMesh->GetComponentQuat(), DesiredRotation, (SkateboardRotationSpeed / 0.017f) * DeltaTime));
			}
		}
	}
	// Case 2/3: On Ground
	else
	{
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);

		FHitResult FrontResult;
		FVector Start = LinetraceFront->GetComponentLocation();
		FVector End = Start + FVector(0, 0, -100.f);
		GetWorld()->LineTraceSingleByChannel(FrontResult, Start, End, ECollisionChannel::ECC_Camera);

		FHitResult BackResult;
		Start = LinetraceBack->GetComponentLocation();
		End = Start + FVector(0, 0, -100.f);
		GetWorld()->LineTraceSingleByChannel(BackResult, Start, End, ECollisionChannel::ECC_Camera);
	}
}


FQuat APlayerCharacter::GetDesiredRotation(FVector DestinationNormal) const
{
	FVector Right = FVector::CrossProduct(DestinationNormal, GetActorForwardVector());
	FVector Forward = FVector::CrossProduct(GetActorRightVector(), DestinationNormal); 

	FRotator Rot = UKismetMathLibrary::MakeRotFromXY(Forward, Right);

	return Rot.Quaternion();
}
