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

bool APlayerCharacter::TraceSkateboard()
{
	if (!IsSocketsValid())
		return false;

	FTransform LinetraceFront = LinetraceSocketFront->GetSocketTransform(SkateboardMesh);
	FTransform LinetraceBack = LinetraceSocketBack->GetSocketTransform(SkateboardMesh);

	FCollisionObjectQueryParams ObjParams;
	ObjParams.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldStatic);

	// Making the trace distance really short makes it happen more often over inclines that only 1 or 0 hits are recorded.
	// This directly affects the rate of which we change to the falling movement mode. This definitely needs more tweaking.
	// Use the bDebugMovement bool in the editor to visualize this (and the projectile prediction above).
	float TraceDistance = 20.f;

	FVector Start = LinetraceFront.GetLocation() + (FVector(0, 0, 1) * TraceDistance);
	FVector End = LinetraceFront.GetLocation() + (FVector(0, 0, 1) * -TraceDistance);

	GetWorld()->LineTraceSingleByObjectType(LastTraceResult.Front, Start, End, ObjParams);

	if (bDebugMovement)
		DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 0.1f);

	Start = LinetraceBack.GetLocation() + (FVector(0, 0, 1) * TraceDistance);
	End = LinetraceBack.GetLocation() + (FVector(0, 0, 1) * -TraceDistance);
	GetWorld()->LineTraceSingleByObjectType(LastTraceResult.Back, Start, End, ObjParams);

	if (bDebugMovement)
		DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 0.1f);

	if (bDebugMovement && LastTraceResult.Front.bBlockingHit)
		DrawDebugSphere(GetWorld(), LastTraceResult.Front.ImpactPoint, 10.f, 10, FColor::Green, false, 0.1f);

	if (bDebugMovement && LastTraceResult.Back.bBlockingHit)
		DrawDebugSphere(GetWorld(), LastTraceResult.Back.ImpactPoint, 10.f, 10, FColor::Green, false, 0.1f);


	return true;
}

bool APlayerCharacter::IsSocketsValid() const
{
	if (!LinetraceSocketFront || !IsValid(LinetraceSocketFront) || !LinetraceSocketBack || !IsValid(LinetraceSocketBack))
	{
		UE_LOG(LogTemp, Error, TEXT("Linetrace sockets not good"));
		return false;
	}
	return true;
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

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

	// Action Mappings
	InputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &APlayerCharacter::Jump);
	InputComponent->BindAction("Handbrake", EInputEvent::IE_Pressed, this, &APlayerCharacter::Handbrake);
	InputComponent->BindAction("Handbrake", EInputEvent::IE_Released, this, &APlayerCharacter::LetGoHandBrake);

	// Axis Mappings
	InputComponent->BindAxis("Forward", this, &APlayerCharacter::MoveForward);
	InputComponent->BindAxis("Right", this, &APlayerCharacter::MoveRight);
	InputComponent->BindAxis("LookUp", this, &APlayerCharacter::LookUp);
	InputComponent->BindAxis("LookRight", this, &APlayerCharacter::LookRight);
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bEnabledRagdoll && bCurrentlyHoldingHandbrake && Movement->Velocity.Size() > HandbreakeVelocityThreshold && !Movement->IsFalling())
	{
		AddActorLocalRotation({ 0, RightAxis * DeltaTime * HandbrakeRotationFactor, 0 });
	}

	UpdateCameraRotation(DeltaTime);
	UpdateSkateboardRotation(DeltaTime);
}

void APlayerCharacter::MoveForward(float Value)
{
	if (HasEnabledRagdoll())
		return;

	if ((bAllowBrakingWhileHandbraking && Value < 0.0f) || (!bCurrentlyHoldingHandbrake && Value != 0))
	{
		AddMovementInput(FVector::ForwardVector * Value);
	}
}

void APlayerCharacter::MoveRight(float Value)
{
	if (HasEnabledRagdoll())
		return;

	const bool bCanMoveHorizontally = !bCurrentlyHoldingHandbrake || Movement->IsFalling();

	if (bCanMoveHorizontally)
	{
		AddMovementInput(FVector::RightVector * Value);
	}

	RightAxis = Value;
}

void APlayerCharacter::LookUp(float Value)
{
	if (Value != 0)
	{
		CameraPitch += Value * CameraRotationSpeed * GetWorld()->GetDeltaSeconds();
	}
	else
	{
		if (FMath::Abs(CameraPitch) <= .1f)
			CameraPitch -= FMath::Sign(CameraPitch) * CameraSnapbackSpeed * GetWorld()->GetDeltaSeconds();
		else
			CameraPitch = 0;
	}
}

void APlayerCharacter::LookRight(float Value)
{
	if (Value != 0)
	{
		CameraYaw += Value * CameraRotationSpeed * GetWorld()->GetDeltaSeconds();
	}
	else
	{
		if (FMath::Abs(CameraYaw) <= .1f)
			CameraYaw -= FMath::Sign(CameraYaw) * CameraSnapbackSpeed * GetWorld()->GetDeltaSeconds();
		else
			CameraYaw = 0;
	}
}

void APlayerCharacter::Handbrake()
{
	bCurrentlyHoldingHandbrake = true;
}

void APlayerCharacter::LetGoHandBrake()
{
	bCurrentlyHoldingHandbrake = false;
}

void APlayerCharacter::UpdateCameraRotation(float DeltaTime)
{
	CameraPitch = FMath::Clamp(CameraPitch, -MaxCameraRotationOffset, MaxCameraRotationOffset);
	CameraYaw = FMath::Clamp(CameraYaw, -MaxCameraRotationOffset, MaxCameraRotationOffset);
	
	FVector point = UKismetMathLibrary::CreateVectorFromYawPitch(CameraYaw - 180.f, CameraPitch);
	FVector Direction = FVector(0, 0, 0) - point;
	FRotator NewLocalRot = UKismetMathLibrary::MakeRotFromXZ(Direction, FVector(0, 0, 1));

	SpringArm->SetRelativeRotation(NewLocalRot);
}

void APlayerCharacter::UpdateSkateboardRotation(float DeltaTime)
{
	if (!IsSocketsValid())
		return;

	// Case 1: In air
	if (GetMovementComponent()->IsFalling())
	{
		FTransform LinetraceFront = LinetraceSocketFront->GetSocketTransform(SkateboardMesh);
		FTransform LinetraceBack = LinetraceSocketBack->GetSocketTransform(SkateboardMesh);

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
		// Perform tracing
		if (!TraceSkateboard())
			return;

		auto traceResults = GetSkateboardTraceResults();

		// Both hits:
		if (traceResults.Front.bBlockingHit && traceResults.Back.bBlockingHit)
		{
			auto dot = FVector::DotProduct(traceResults.Front.ImpactNormal, traceResults.Back.ImpactNormal);
			if (dot != 0)
			{
				auto newNormal = (traceResults.Front.ImpactNormal + traceResults.Back.ImpactNormal).GetSafeNormal();
				auto DesiredRotation = GetDesiredRotation(newNormal);
				SkateboardMesh->SetWorldRotation(FQuat::Slerp(SkateboardMesh->GetComponentQuat(), DesiredRotation, (SkateboardRotationGroundSpeed / 0.017f) * DeltaTime * dot));
			}
			
			// There is a case here where we should fall, because if the player handbrakes over an edge it is still glued to the slope and it looks weird.
			// If some combination of normals changing here is true, we need to switch movement mode to falling.
		}
		// One hit:
		else if (traceResults.Front.bBlockingHit || traceResults.Back.bBlockingHit)
		{	
			auto &result = traceResults.Front.bBlockingHit ? traceResults.Front : traceResults.Back;
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

TPair<FVector, FVector> APlayerCharacter::GetSkateboardFeetLocations() const
{
	return TPair<FVector, FVector>{SkateboardMesh->GetSocketLocation("FootLeft"), SkateboardMesh->GetSocketLocation("FootRight")};
}

FTransform APlayerCharacter::GetCharacterBoneTransform(FName BoneName) const
{
	auto boneIndex = GetMesh()->GetBoneIndex(BoneName);
	return boneIndex > -1 ? GetMesh()->GetBoneTransform(boneIndex) : FTransform{};
}

FTransform APlayerCharacter::GetCharacterBoneTransform(FName BoneName, const FTransform& localToWorld) const
{
	auto boneIndex = GetMesh()->GetBoneIndex(BoneName);
	return boneIndex > -1 ? GetMesh()->GetBoneTransform(boneIndex, localToWorld) : FTransform{};
}

FTransform APlayerCharacter::GetCharacterRefPoseBoneTransform(FName BoneName) const
{
	auto boneIndex = GetMesh()->GetBoneIndex(BoneName);
	return boneIndex > -1 ? GetMesh()->SkeletalMesh->RefSkeleton.GetRefBonePose()[boneIndex] : FTransform{};
}

FTransform APlayerCharacter::GetCharacterRefPoseBoneTransform(FName BoneName, const FTransform& localToWorld) const
{
	auto boneIndex = GetMesh()->GetBoneIndex(BoneName);
	return boneIndex > -1 ? GetMesh()->SkeletalMesh->RefSkeleton.GetRefBonePose()[boneIndex] * localToWorld : FTransform{};
}