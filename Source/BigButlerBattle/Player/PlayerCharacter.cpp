// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "PlayerCharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PlayerSpringArmComponent.h"
#include "PlayerCameraComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "BigButlerBattleGameModeBase.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "Animation/CharacterAnimInstance.h"
#include "Utils/btd.h"
#include "Tasks/TaskObject.h"
#include "Tasks/Task.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "King/King.h"
#include "PlayerCharacterController.h"
#include "Components/AudioComponent.h"
#include "Utils/Railing.h"
#include "Components/SplineComponent.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "Engine/EngineTypes.h"
#include "Curves/CurveFloat.h"
#include "Kismet/GameplayStatics.h"

APlayerCharacter::APlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: ACharacter(ObjectInitializer.SetDefaultSubobjectClass<UPlayerCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;

	JumpMaxHoldTime = 0.3f;

	SkateboardMesh = CreateDefaultSubobject<USkeletalMeshComponent>("Skateboard Mesh");
	SkateboardMesh->SetupAttachment(RootComponent);
	SkateboardMesh->SetRelativeLocation(FVector{0.f, 0.f, -100.f});

	GetMesh()->SetRelativeLocation(FVector{0.f, 0.f, -110.f});

	GetCapsuleComponent()->SetCapsuleHalfHeight(100.f);
	GetCapsuleComponent()->SetNotifyRigidBodyCollision(true);

	SpringArm = CreateDefaultSubobject<UPlayerSpringArmComponent>("Spring Arm");
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->SetRelativeLocation(FVector(0, 80.f, 0.f));
	SpringArm->SetRelativeRotation(FRotator(5.f, 0, 0));
	SpringArm->TargetArmLength = 300.f;
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 10.f;
	SpringArm->bEnableCameraRotationLag = true;
	SpringArm->CameraRotationLagSpeed = 15.f;
	SpringArm->CameraLagMaxDistance = 70.f;
	SpringArm->ProbeSize = 18.f;

	Camera = CreateDefaultSubobject<UPlayerCameraComponent>("Camera");
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

	TaskObjectPickupCollision = CreateDefaultSubobject<UBoxComponent>("Object Pickup Collision");
	TaskObjectPickupCollision->SetupAttachment(RootComponent);

	// Default values

	TaskObjectPickupCollision->SetGenerateOverlapEvents(true);
	TaskObjectPickupCollision->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECollisionResponse::ECR_Overlap);

	TaskObjectPickupCollision->SetRelativeLocation(FVector{85.f, 0.f, 0.f});
	TaskObjectPickupCollision->SetBoxExtent(FVector{64.f, 190.f, 150.f});

	TaskObjectCameraCollision = CreateDefaultSubobject<UCapsuleComponent>("Capsule Object Collision");
	TaskObjectCameraCollision->SetupAttachment(Camera);

	// Default values

	TaskObjectCameraCollision->SetGenerateOverlapEvents(true);
	TaskObjectCameraCollision->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECollisionResponse::ECR_Overlap);

	TaskObjectCameraCollision->SetRelativeLocation(FVector{630.f, -80.f, 4.f});
	TaskObjectCameraCollision->SetRelativeRotation(FRotator(-90.f + SpringArm->GetRelativeRotation().Pitch, 0.f, 0.f));
	TaskObjectCameraCollision->InitCapsuleSize(324.f, 410.f);

	Tray = CreateDefaultSubobject<UStaticMeshComponent>("Tray");
	Tray->SetupAttachment(GetMesh(), "TraySocket");
	Tray->SetRelativeLocation(FVector(60.f, -50.f, 184.f));
	Tray->SetRelativeRotation(FRotator{0, -90.f, -10.f});
	Tray->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));


	check(Tray != nullptr);

	TraySlotNames.Reserve(4);
	TraySlotNames.Add("Slot_0");
	TraySlotNames.Add("Slot_1");
	TraySlotNames.Add("Slot_2");
	TraySlotNames.Add("Slot_3");

	MainSlotName = "Slot_Main";

	// The 5th spot is for the main item. This can never be reached by incrementing etc,
	// but makes things easier for us in code.
	Inventory.Init(nullptr, 5);



	bUseControllerRotationYaw = false;

	PlayersInRangeCollision = CreateDefaultSubobject<UBoxComponent>("Players In Range Collision");
	PlayersInRangeCollision->SetupAttachment(RootComponent);

	PlayersInRangeCollision->SetGenerateOverlapEvents(true);
	PlayersInRangeCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	PlayersInRangeCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	PlayersInRangeCollision->SetRelativeLocation(FVector{ 0, 0, 32.f });
	PlayersInRangeCollision->SetBoxExtent(FVector{ 128.f, 256.f, 128.f });


	// Sound
	ContinuousSound = CreateDefaultSubobject<UAudioComponent>("Audio Component");
	ContinuousSound->SetupAttachment(RootComponent);

	GrindingOverlapThreshold = CreateDefaultSubobject<USphereComponent>("Grinding Overlap Threshold");
	GrindingOverlapThreshold->SetupAttachment(RootComponent);
	// Set overlap threshold to ignore everything but the rail's
	GrindingOverlapThreshold->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	GrindingOverlapThreshold->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel3, ECollisionResponse::ECR_Overlap);
	GrindingOverlapThreshold->SetRelativeLocation(FVector{0.f, 0.f, -60.f});
	GrindingOverlapThreshold->SetSphereRadius(200.f);


	// Particles
	SkateboardParticles = CreateDefaultSubobject<UNiagaraComponent>("Skateboard particles");
	SkateboardParticles->SetupAttachment(SkateboardMesh);
	SkateboardParticles->SetRelativeScale3D(FVector{0.1f, 0.1f, 0.1f});
	SkateboardParticles->SetRelativeRotation(FRotator{0.f, 90.f, 0.f});
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (!SkateboardMesh || !IsValid(SkateboardMesh))
	{
		UE_LOG(LogTemp, Error, TEXT("SkateboardMesh is not valid!"));
		return;
	}

	Tray->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true), "TraySocket");

	AnimInstance = Cast<UCharacterAnimInstance>(GetMesh()->GetAnimInstance());

	LinetraceSocketFront = SkateboardMesh->GetSocketByName("LinetraceFront");
	LinetraceSocketBack = SkateboardMesh->GetSocketByName("LinetraceBack");

	DefaultCameraRotation.X = SpringArm->GetRelativeRotation().Yaw;
	DefaultCameraRotation.Y = SpringArm->GetRelativeRotation().Pitch;

	Movement = Cast<UPlayerCharacterMovementComponent>(GetMovementComponent());

	Movement->OnCustomMovementStart.AddLambda([&](uint8 MovementMode){
		if (MovementMode == static_cast<uint8>(ECustomMovementType::MOVE_Grinding))
			SetRailCollision(false);
	});

	Movement->OnCustomMovementEnd.AddLambda([&](uint8 MovementMode){
		if (MovementMode == static_cast<uint8>(ECustomMovementType::MOVE_Grinding))
			SetRailCollision(true);
	});

	Movement->OnMovementEnd.AddLambda([&](EMovementMode MovementMode) {
		if (MovementMode == EMovementMode::MOVE_Falling)
		{
			if (LandSound)
				UGameplayStatics::PlaySoundAtLocation(this, LandSound, GetActorLocation(), 1.f);
		}
	});

	GameMode = Cast<ABigButlerBattleGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));

	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &APlayerCharacter::OnCapsuleHit);

	TaskObjectPickupCollision->OnComponentBeginOverlap.AddDynamic(this, &APlayerCharacter::OnTaskObjectPickupCollisionBeginOverlap);
	TaskObjectPickupCollision->OnComponentEndOverlap.AddDynamic(this, &APlayerCharacter::OnTaskObjectPickupCollisionEndOverlap);

	TaskObjectCameraCollision->OnComponentBeginOverlap.AddDynamic(this, &APlayerCharacter::OnTaskObjectCameraCollisionBeginOverlap);
	TaskObjectCameraCollision->OnComponentEndOverlap.AddDynamic(this, &APlayerCharacter::OnTaskObjectCameraCollisionEndOverlap);

	DefaultSpringArmLength = SpringArm->TargetArmLength;

	PlayersInRangeCollision->OnComponentBeginOverlap.AddDynamic(this, &APlayerCharacter::OnPlayersInRangeCollisionBeginOverlap);
	PlayersInRangeCollision->OnComponentEndOverlap.AddDynamic(this, &APlayerCharacter::OnPlayersInRangeCollisionEndOverlap);

	GrindingOverlapThreshold->OnComponentBeginOverlap.AddDynamic(this, &APlayerCharacter::OnGrindingOverlapBegin);
	GrindingOverlapThreshold->OnComponentEndOverlap.AddDynamic(this, &APlayerCharacter::OnGrindingOverlapEnd);


	// Particles
	Movement->OnCustomMovementStart.AddLambda([&](uint8 movementMode){
		if (static_cast<ECustomMovementType>(movementMode) == ECustomMovementType::MOVE_Grinding)
		{
			SkateboardParticles->SetVariableBool(FName{"User.EmittersEnabled"}, true);
		}
	});

	Movement->OnCustomMovementEnd.AddLambda([&](uint8 movementMode) {
		if (static_cast<ECustomMovementType>(movementMode) == ECustomMovementType::MOVE_Grinding)
		{
			SkateboardParticles->SetVariableBool(FName{"User.EmittersEnabled"}, false);
		}
	});
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* Input)
{
	Super::SetupPlayerInputComponent(Input);

	// Action Mappings
	Input->BindAction("Jump", EInputEvent::IE_Pressed, this, &APlayerCharacter::Jump);
	Input->BindAction("Jump", EInputEvent::IE_Released, this, &ACharacter::StopJumping);
	Input->BindAction("DropObject", EInputEvent::IE_Pressed, this, &APlayerCharacter::ThrowStart);
	//Input->BindAction("DropObject", EInputEvent::IE_Repeat, this, &APlayerCharacter::OnHoldingThrow);
	//Input->BindAction("DropObject", EInputEvent::IE_Released, this, &APlayerCharacter::OnHoldThrowReleased);
	Input->BindAction("IncrementInventory", EInputEvent::IE_Pressed, this, &APlayerCharacter::IncrementCurrentItemIndex);
	Input->BindAction("Tackle", EInputEvent::IE_Pressed, this, &APlayerCharacter::TryTackle);

	// Axis Mappings
	Input->BindAxis("Forward", this, &APlayerCharacter::MoveForward);
	Input->BindAxis("Right", this, &APlayerCharacter::MoveRight);
	Input->BindAxis("LookUp", this, &APlayerCharacter::LookUp);
	Input->BindAxis("LookRight", this, &APlayerCharacter::LookRight);
	Input->BindAxis("Handbrake", this, &APlayerCharacter::UpdateHandbrake);
	Input->BindAxis("Brake", this, &APlayerCharacter::Brake);
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateCameraRotation(DeltaTime);
	UpdateSkateboardRotation(DeltaTime);

	// No point checking for closest objects when main item is held.
	if(!bHasMainItem)
		UpdateClosestTaskObject();

	// Update sound
	if (ContinuousSound && Movement)
	{
		ContinuousSound->SetFloatParameter(FName{"skateboardGain"}, Movement->GetRollingAudioVolumeMult());
		ContinuousSound->SetFloatParameter(FName{"grindingGain"}, Movement->GetGrindingAudioVolumeMult());
	}

	if (CanGrind())
	{
		if (auto rail = GetClosestRail())
		{
			StartGrinding(rail);
		}
	}

	// Particles
	const auto wheels = GetWheelLocations();
	for (int i{0}; i < 4; ++i)
	{
		FString var{"User.WheelPos"};
		var.AppendChar(btd::ConvertIntDigitToChar(i));

		SkateboardParticles->SetVariableVec3(*var, wheels[i]);
	}
	float velocityFactor = Movement->Velocity.Size2D() / SkidmarkVelocityThreshold;
	velocityFactor = FMath::Clamp(SkidmarkStrengthCurve ? SkidmarkStrengthCurve->GetFloatValue(velocityFactor) : velocityFactor, 0.f, 1.f);
	const float skidMarkStrength = velocityFactor * FMath::Abs(Movement->GetRotationInput()) * static_cast<float>(!(Movement->IsFalling()));
	SkateboardParticles->SetVariableFloat(FName{"User.SkidMarkStrength"}, skidMarkStrength);
}






void APlayerCharacter::EnableRagdoll(const FVector& Impulse, const FVector& HitLocation)
{
	if (!bCanFall || bEnabledRagdoll)
		return;


	// Capsule

	GetCapsuleComponent()->SetNotifyRigidBodyCollision(false);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Butler mesh

	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetAllBodiesSimulatePhysics(true);
	GetMesh()->WakeAllRigidBodies();

	if (!Impulse.IsZero())
	{
		GetMesh()->AddImpulseAtLocation(Impulse, HitLocation);
	}

	// Tray

	Tray->SetSimulatePhysics(true);
	Tray->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, true));

	// Only place where we iterate through all, just so main item too is affected.
	for (auto& Obj : Inventory)
	{
		if (Obj)
		{
			DetachObject(Obj, Obj->GetActorLocation(), Movement->Velocity);
		}
	}

	Camera->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, true));


	// Skateboard

	SkateboardMesh->SetAllBodiesSimulatePhysics(true);
	SkateboardMesh->WakeAllRigidBodies();

	bEnabledRagdoll = true;

	if (CrashSound)
		UGameplayStatics::PlaySoundAtLocation(this, CrashSound, GetActorLocation(), 1.f);

	OnCharacterFell.ExecuteIfBound(CurrentRoom, GetActorLocation());

	bDed = true;
	for (auto& obj : TaskObjectsInPickupRange)
		obj->SetSelected(false);
}

void APlayerCharacter::OnCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (bDed)
		return;

	if (auto Other = Cast<APlayerCharacter>(OtherActor))
	{
		if (Movement->Velocity.Size() > CrashVelocityFallOffThreshold)
		{
			EnableRagdoll();
			Other->EnableRagdoll(NormalImpulse * Movement->Velocity, Hit.ImpactPoint);
		}
	}
}





/// ==================================== Movement =================================================
void APlayerCharacter::Jump()
{
	Super::Jump();

	if (JumpSound)
		UGameplayStatics::PlaySoundAtLocation(this, JumpSound, GetActorLocation(), 1.f);
}

FVector APlayerCharacter::GetInputAxis() const
{
	return FVector{GetInputAxisValue("Forward"), GetInputAxisValue("Right"), 0.f};
}

void APlayerCharacter::MoveForward(float Value)
{
	if (HasEnabledRagdoll() || !Movement || Movement->IsFalling() || Movement->IsGrinding())
		return;

	bool bBraking = false;
	bool bMovingBackwards = false;
	auto acceleration = Movement->GetInputAcceleration(bBraking, bMovingBackwards, Value);

	if (!bBraking && Value != 0 && AnimInstance && !AnimInstance->IsAnyMontagePlaying())
	{
		// Normalize with time
		const float deltaTime = GetWorld()->GetDeltaSeconds();
		acceleration = Movement->GetInputAccelerationTimeNormalized(acceleration, bBraking, deltaTime);
		if (Movement->CanForwardAccelerate(acceleration, deltaTime, bMovingBackwards))
		{
			highestInput = 0.f;
			AnimInstance->ForwardKick();
		}
	}

	if (Value > highestInput)
		highestInput = Value;
}

void APlayerCharacter::AddForwardInput()
{
	AddMovementInput(FVector::ForwardVector * highestInput);
}

void APlayerCharacter::MoveRight(float Value)
{
	if (HasEnabledRagdoll())
		return;

	AddMovementInput(FVector::RightVector * Value);
}

void APlayerCharacter::UpdateHandbrake(float Value)
{
	AddMovementInput(FVector::UpVector * Value);
}

void APlayerCharacter::Brake(float Value)
{
	Value = FMath::Max(Value, 0.f);
	AddMovementInput(-FVector::ForwardVector * Value);
}





/// ==================================== Camera =================================================
void APlayerCharacter::SetCustomSpringArmLength()
{
	DefaultSpringArmLength = SpringArm->TargetArmLength = CustomSpringArmLength;
}

void APlayerCharacter::LookUp(float Value)
{
	DesiredCameraRotation.Y = Value != 0 ? Value * CameraRotationPitchHeight : 0.f;
	if (CameraInvertPitch)
		DesiredCameraRotation.Y = -DesiredCameraRotation.Y;
}

void APlayerCharacter::LookRight(float Value)
{
	DesiredCameraRotation.X = Value != 0 ? Value * CameraRotationYawAngle : 0.f;
	if (CameraInvertYaw)
		DesiredCameraRotation.X = -DesiredCameraRotation.X;
}

void APlayerCharacter::UpdateCameraRotation(const float DeltaTime)
{
	// Clamp rotation
	DesiredCameraRotation.X = FMath::Clamp(DesiredCameraRotation.X, -CameraRotationYawAngle, CameraRotationYawAngle);
	DesiredCameraRotation.Y = FMath::Clamp(DesiredCameraRotation.Y, -CameraRotationPitchHeight, CameraRotationPitchHeight);

	// Find camera lerp factor
	const float lerpFactor = FMath::Clamp(CameraRotationSpeed * DeltaTime, 0.f, 1.f);


	// Find new rotation
	const bool bXNearZero = FMath::Abs(DesiredCameraRotation.X - CameraRotation.X) / CameraRotationYawAngle < CameraRotationDeadZone;
	const bool bYNearZero = FMath::Abs(DesiredCameraRotation.Y - CameraRotation.Y) / CameraRotationPitchHeight < CameraRotationDeadZone;

	CameraRotation.X = bXNearZero ? DesiredCameraRotation.X : FMath::Lerp(CameraRotation.X, DesiredCameraRotation.X, lerpFactor);
	CameraRotation.Y = bYNearZero ? DesiredCameraRotation.Y : FMath::Lerp(CameraRotation.Y, DesiredCameraRotation.Y, lerpFactor);


	// Set rotation of camera
	const auto Point = UKismetMathLibrary::CreateVectorFromYawPitch(CameraRotation.X - 180.f, 0.f) + FVector{0.f, 0.f, CameraRotation.Y};
	const auto Direction = FVector(0, 0, 0) - Point;
	const auto NewLocalRot = UKismetMathLibrary::MakeRotFromXZ(Direction, FVector(0, 0, 1));

	SpringArm->SetRelativeRotation(FRotator(DefaultCameraRotation.Y, DefaultCameraRotation.X, 0) - NewLocalRot);
	SpringArm->TargetArmLength = DefaultSpringArmLength * Direction.Size();
}





FRotator APlayerCharacter::GetSkateboardRotation() const
{
	return SkateboardMesh->GetRelativeRotation();
}

FVector APlayerCharacter::GetSkateboardLocation() const
{
	return SkateboardMesh->GetRelativeLocation();
}

bool APlayerCharacter::TraceSkateboard()
{
	if (!IsSocketsValid())
		return false;

	const auto LinetraceFront = LinetraceSocketFront->GetSocketTransform(SkateboardMesh);
	const auto LinetraceBack = LinetraceSocketBack->GetSocketTransform(SkateboardMesh);

	FCollisionObjectQueryParams ObjParams;
	ObjParams.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldStatic);

	// Making the trace distance really short makes it happen more often over inclines that only 1 or 0 hits are recorded.
	// This directly affects the rate of which we change to the falling movement mode. This definitely needs more tweaking.
	// Use the bDebugMovement bool in the editor to visualize this (and the projectile prediction above).
	const float TraceDistance = 20.f;

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
			float Angle = btd::GetAngleBetweenNormals(LandNormal, SkateboardMesh->GetUpVector());

			if (bDebugMovement)
				DrawDebugSphere(GetWorld(), Result.HitResult.ImpactPoint, 10.f, 10, FColor::Green, false, 0.1f);

			if(Angle < Movement->GetWalkableFloorAngle())
			{
				auto DesiredRotation = GetDesiredRotation(LandNormal);
				SkateboardMesh->SetWorldRotation(FQuat::Slerp(SkateboardMesh->GetComponentQuat(), DesiredRotation, (SkateboardRotationAirSpeed / 0.017f) * DeltaTime));
			}
		}
	}
	// Case 2/4: Grinding
	else if (Movement->IsGrinding())
	{
		const auto railNormal = Movement->GetRailNormal();
		if (railNormal.IsNearlyZero())
			return;

		auto desiredRotation = GetDesiredRotation(railNormal);
		float alpha = (SkateboardRotationGrindingSpeed / 0.017f) * DeltaTime;
		if (AnimInstance)
		{
			AnimInstance->LeftLegJointRotation = FQuat::Slerp(AnimInstance->LeftLegJointRotation, FQuat{GetActorUpVector(), -PI / 2}, alpha);
			AnimInstance->RightLegJointRotation = FQuat::Slerp(AnimInstance->RightLegJointRotation, FQuat{GetActorUpVector(), -PI / 2}, alpha);
		}
		SkateboardMesh->SetWorldRotation(FQuat::Slerp(SkateboardMesh->GetComponentQuat(), desiredRotation, alpha));
	}
	// Case 3/4: On Ground
	else
	{
		// Perform tracing
		if (!TraceSkateboard())
			return;

		auto TraceResults = GetSkateboardTraceResults();

		// Both hits:
		if (TraceResults.Front.bBlockingHit && TraceResults.Back.bBlockingHit)
		{
			auto dot = FVector::DotProduct(TraceResults.Front.ImpactNormal, TraceResults.Back.ImpactNormal);
			if (dot != 0)
			{
				auto NewNormal = (TraceResults.Front.ImpactNormal + TraceResults.Back.ImpactNormal).GetSafeNormal();
				auto DesiredRotation = GetDesiredRotation(NewNormal);
				SkateboardMesh->SetWorldRotation(FQuat::Slerp(SkateboardMesh->GetComponentQuat(), DesiredRotation, (SkateboardRotationGroundSpeed / 0.017f) * DeltaTime * dot));
			}

			// There is a case here where we should fall, because if the player handbrakes over an edge it is still glued to the slope and it looks weird.
			// If some combination of normals changing here is true, we need to switch movement mode to falling.
		}
		// One hit:
		else if (TraceResults.Front.bBlockingHit || TraceResults.Back.bBlockingHit)
		{
			auto &Result = TraceResults.Front.bBlockingHit ? TraceResults.Front : TraceResults.Back;
			auto DesiredRotation = GetDesiredRotation(Result.ImpactNormal);
			SkateboardMesh->SetWorldRotation(FQuat::Slerp(SkateboardMesh->GetComponentQuat(), DesiredRotation, (SkateboardRotationGroundSpeed / 0.017f) * DeltaTime));

			// Turn on falling just in case we are at an edge/incline and the velocity is great enough to get some air
			// Movement->SetMovementMode(EMovementMode::MOVE_Falling);
		}
		// No hits:
		else
		{
			// Turn on falling, because we have no idea where the ground is and we are definitely in the air.
			// Movement->SetMovementMode(EMovementMode::MOVE_Falling);
		}
	}

	// Reset joint locations
	if ((GetMovementComponent()->IsFalling() || !Movement->IsGrinding()) && AnimInstance)
	{
		const float alpha = (SkateboardRotationGroundSpeed / 0.017f) * DeltaTime;
		if (!AnimInstance->LeftLegJointRotation.IsIdentity())
		{
			const auto lerp = FQuat::Slerp(AnimInstance->LeftLegJointRotation, FQuat::Identity, alpha);
			AnimInstance->LeftLegJointRotation = lerp.IsIdentity(0.0001f) ? FQuat::Identity : lerp;
		}

		if (!AnimInstance->RightLegJointRotation.IsIdentity())
		{
			const auto lerp = FQuat::Slerp(AnimInstance->RightLegJointRotation, FQuat::Identity, alpha);
			AnimInstance->RightLegJointRotation = lerp.IsIdentity(0.0001f) ? FQuat::Identity : lerp;
		}
	}
}


FQuat APlayerCharacter::GetDesiredRotation(const FVector& DestinationNormal) const
{
	const FVector Right = FVector::CrossProduct(DestinationNormal, GetActorForwardVector());
	const FVector Forward = FVector::CrossProduct(GetActorRightVector(), DestinationNormal);

	const FRotator Rot = UKismetMathLibrary::MakeRotFromXY(Forward, Right);

	return Rot.Quaternion();
}

FQuat APlayerCharacter::GetDesiredGrindingRotation(const FVector &DestinationNormal) const
{
	const FVector Right = FVector::CrossProduct(GetActorRightVector(), DestinationNormal);
	const FVector Forward = FVector::CrossProduct(GetActorForwardVector(), DestinationNormal);

	const FRotator Rot = UKismetMathLibrary::MakeRotFromXY(Forward, Right);

	return Rot.Quaternion();
}





void APlayerCharacter::OnObjectPickedUp(ATaskObject* Object)
{
	if (bHasMainItem)
		return;

	// Find out if we should use main slot or not
	FName SlotToUse = "";
	int Index = -1; // Index of the inventory to use, 4 for main item 0-3 for regular items.
	if (Object->GetIsMainItem())
	{
		SlotToUse = MainSlotName;
		Index = 4;
	}
	else
	{
		for (int i = 0; i < 4; ++i)
		{
			if (Inventory[i] == nullptr)
			{
				SlotToUse = TraySlotNames[i];
				Index = i;
				break;
			}
		}
	}

	if (SlotToUse != "")
	{
		// Disable picked up object
		Object->OnPickedUp();

		// Spawn new object
		auto Spawned = GetWorld()->SpawnActorDeferred<ATaskObject>(TaskObjectBlueprintOverride ? TaskObjectBlueprintOverride : ATaskObject::StaticClass(), FTransform::Identity);
		Spawned->SetTaskData(Object->GetTaskData());
		Spawned->Enable(true, false, false);
		UGameplayStatics::FinishSpawningActor(Spawned, FTransform::Identity);
		if (Object->GetIsMainItem())
			Spawned->SetAsMainItem();

		Spawned->bOnTray = true;
		Spawned->SetParticlesEnable(false);

		// Attach new object
		Inventory[Index] = Spawned;
		Spawned->AttachToComponent(
			Tray,
			FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true),
			SlotToUse);

		// Scale it down
		Spawned->SetActorScale3D(Spawned->GetActorScale3D() * 0.3f);

		if (PickupSound)
			UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation(), 1.f);

		OnTaskObjectPickedUp.ExecuteIfBound(Spawned);
	}
}

void APlayerCharacter::ThrowStart()
{
	bool bFound = false;
	for (const auto& obj : Inventory)
		if (obj)
		{
			bFound = true;
			break;
		}
	if (!bFound)
		return;

	if (AnimInstance)
	{
		if (AnimInstance->IsDoingAction())
			return;

		AnimInstance->ThrowAnim();
	}
	else
		Throw();
}

void APlayerCharacter::Throw()
{
	auto Obj = Inventory[CurrentItemIndex];
	if (!Obj)
	{
		IncrementCurrentItemIndex();
		Obj = Inventory[CurrentItemIndex];
		if (!Obj)
			return;
	}

	FVector Dir = Camera->GetForwardVector();

	FHitResult HitResult;

	const float Radius = 300.f;
	const FVector Start = GetActorLocation() + (Dir * (Radius + 10.f));
	const FVector End = Start + (Dir * AimbotMaxDistance);

	TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;
	TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	// Calculate projectile velocity (constant + projected velocity on direction to throw)
	float ProjectileVelocity = ThrowStrength;
	if (FMath::IsNearlyZero(Movement->Velocity.SizeSquared()))
	{
		ProjectileVelocity += UKismetMathLibrary::ProjectVectorOnToVector(Movement->Velocity, Dir).Size();
	}

	// Check if aimbot found any player
	if (GetWorld()->SweepSingleByObjectType(HitResult, Start, End, FQuat::Identity, FCollisionObjectQueryParams(TraceObjectTypes), FCollisionShape::MakeSphere(Radius), Params))
	{
		//UE_LOG(LogTemp, Warning, TEXT("Aimbot: Found %s"), *HitResult.GetActor()->GetName());

		// Location of other
		const auto TargetLocation = HitResult.GetActor()->GetActorLocation();

		// Only activate aimbot if target is within 180 degrees of us (should always be true tbh)
		const auto Angle = btd::GetAngleBetweenNormals(Dir, (TargetLocation - Start).GetSafeNormal());
		if (Angle < 180.f)
		{
			auto TargetVelocity = HitResult.GetActor()->GetVelocity();

			// So slow moving, that we will just go with the initial location
			if (TargetVelocity.SizeSquared() < 10.f)
			{
				Dir = (TargetLocation - GetActorLocation()).GetSafeNormal();
			}
			else
			{
				// Time to hit target based on our initial projectile velocity
				const auto Time = (GetActorLocation() - TargetLocation).Size() / ProjectileVelocity;

				// The predicted location of the target based on their current location, velocity and time
				const auto PredictedLocation = TargetLocation + TargetVelocity * Time;

				// Visualize predicted location
				//DrawDebugSphere(GetWorld(), predictedPos, 10.f, 4, FColor::Red, false, 2.f, 0, 10.f);

				// Get new direction
				Dir = (PredictedLocation - GetActorLocation()).GetSafeNormal();
			}
		}
	}

	const auto FinalVelocity = Dir * ProjectileVelocity;

	const auto SpawnPos = GetActorLocation() + (Dir * 200.f);

	DetachObject(Obj, SpawnPos, FinalVelocity);
	IncrementCurrentItemIndex();
}

void APlayerCharacter::DetachObject(ATaskObject* Object, FVector SpawnLocation, FVector LaunchVelocity)
{
	if (Object)
	{
		// Disable old object
		Object->Enable(false, false, false);

		// Deferred spawn new
		auto Spawned = GetWorld()->SpawnActorDeferred<ATaskObject>(TaskObjectBlueprintOverride ? TaskObjectBlueprintOverride : ATaskObject::StaticClass(), FTransform::Identity);

		Spawned->SetTaskData(Object->GetTaskData());

		if (Object->GetIsMainItem())
			Spawned->SetAsMainItem();

		// Spawn transform
		auto transform = Object->GetTransform();

		// Scale (scale it back up)
		transform.SetScale3D(transform.GetScale3D() / 0.3f);

		transform.SetLocation(SpawnLocation);

		if (LaunchVelocity != FVector::ZeroVector)
		{
			Spawned->bCanHit = true;
		}

		// Finish
		Spawned = Cast<ATaskObject>(UGameplayStatics::FinishSpawningActor(Spawned, transform));
		DroppedObjects.Add(Spawned);
		btd::Delay(this, 0.5f, [this]()
		{
			if (DroppedObjects.Num())
			{
				if (DroppedObjects.IsValidIndex(0))
				{
					auto Object = DroppedObjects[0];
					DroppedObjects.Remove(Object);
				}
			}
		});
		Spawned->Launch(LaunchVelocity);
		Spawned->SetParticlesEnable(true);

		OnTaskObjectDropped.ExecuteIfBound(Spawned);

		// Destroy old object
		Object->Destroy();
		Inventory[CurrentItemIndex] = nullptr;
	}
}


void APlayerCharacter::OnHoldingThrow()
{
	if(!bCurrentlyHoldingThrow)
		bCurrentlyHoldingThrow = true;


	// Direction to throw object
	auto Dir = (GetActorLocation() - Camera->GetComponentLocation()).GetSafeNormal();

	auto SpawnPos = GetActorLocation() + (Dir * 200.f);

	auto VelProject = UKismetMathLibrary::ProjectVectorOnToVector(Movement->Velocity, Dir);

	FPredictProjectilePathParams Params;
	Params.ActorsToIgnore.Add(this);
	Params.LaunchVelocity = VelProject + (Dir * ThrowStrength);
	Params.MaxSimTime = 1.f;
	Params.StartLocation = SpawnPos;
	Params.TraceChannel = ECollisionChannel::ECC_WorldStatic;
	Params.bTraceWithChannel = true;
	Params.bTraceWithCollision = true;
	//if (bDebugMovement)
	//{
		Params.DrawDebugTime = 0.1f;
		Params.DrawDebugType = EDrawDebugTrace::ForDuration;
	//}


	FPredictProjectilePathResult Result;
	UGameplayStatics::PredictProjectilePath(GetWorld(), Params, Result);
}

void APlayerCharacter::OnHoldThrowReleased()
{
	Throw();

	if(bCurrentlyHoldingThrow)
		bCurrentlyHoldingThrow = false;
}

void APlayerCharacter::IncrementCurrentItemIndex()
{
	float DeltaYaw = 0.0f;
	int i = CurrentItemIndex;
	bool Found = false;
	do
	{
		DeltaYaw += 90.f;
		i = (i + 1) % 4; // Only to 4, because the 5th one is the main item
		if (Inventory[i] != nullptr)
		{
			Tray->AddLocalRotation(FRotator(0, DeltaYaw, 0));
			CurrentItemIndex = i;
			Found = true;
			break;
		}
	} while (i != CurrentItemIndex);

	if (!Found)
		ResetItemIndex();
}

void APlayerCharacter::ResetItemIndex()
{
	CurrentItemIndex = 0;
	Tray->SetRelativeRotation(FRotator(0, 0, 0));
}

void APlayerCharacter::OnTaskObjectPickupCollisionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bDed)
		return;

	if (OtherActor->IsA(ATaskObject::StaticClass()) && !bHasMainItem)
	{
		auto TaskObject = Cast<ATaskObject>(OtherActor);
		if(TaskObject == ClosestPickup)
		{
			OnObjectPickedUp(TaskObject);
		}
		else
		{
			TaskObjectsInPickupRange.Add(TaskObject);
		}
	}
	else if(auto King = Cast<AKing>(OtherActor))
	{
		if (King->bCanReceiveMainItem)
		{
			OnDeliverTasks.ExecuteIfBound(Inventory);
		}
	}
}

void APlayerCharacter::OnTaskObjectPickupCollisionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (bDed)
		return;

	if (auto TaskObject = Cast<ATaskObject>(OtherActor))
	{
		TaskObjectsInPickupRange.RemoveSingle(TaskObject);
		TaskObject->SetSelected(false);
	}
}

void APlayerCharacter::OnTaskObjectCameraCollisionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bDed)
		return;

	if (auto Object = Cast<ATaskObject>(OtherActor))
	{
		if (!bHasMainItem && DroppedObjects.Find(Object) == INDEX_NONE)
		{
			TaskObjectsInCameraRange.Add(Object);
		}
	}
}

void APlayerCharacter::OnTaskObjectCameraCollisionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (bDed)
		return;

	if (auto Object = Cast<ATaskObject>(OtherActor))
	{
		if (Object == ClosestPickup)
		{
			if(!Object->GetIsMainItem())
				Object->SetSelected(false);

			ClosestPickup = nullptr;
		}
		TaskObjectsInCameraRange.RemoveSingle(Object);
	}
}

void APlayerCharacter::TryTackle()
{
	if (AnimInstance)
	{
		// Cancel tackle if already doing something (like throwing)
		if (AnimInstance->IsDoingAction())
			return;
		
		AnimInstance->TackleAnim();
	}

	if (!PlayersInRange.Num())
		return;

	float ClosestDistance = MAX_FLT;
	APlayerCharacter* ClosestPlayer = nullptr;
	FVector Direction = FVector::ZeroVector;

	for (int i = 0; i < PlayersInRange.Num(); ++i)
	{
		auto OtherPlayer = PlayersInRange[i];
		auto OtherPlayerLocation = OtherPlayer->GetActorLocation();

		// Check angle
		Direction = (OtherPlayerLocation - GetActorLocation()).GetSafeNormal();
		auto angle = FMath::RadiansToDegrees(btd::FastAcos(FMath::Abs(FVector::DotProduct(GetActorForwardVector(), Direction))));
		if (angle < TackleAngleThreshold)
			continue;

		// Check distance
		auto Distance = FVector::Distance(GetActorLocation(), OtherPlayerLocation);
		if (Distance < ClosestDistance)
			ClosestPlayer = OtherPlayer;
	}

	if (ClosestPlayer)
	{
		ClosestPlayer->EnableRagdoll(Direction * TackleStrength, ClosestPlayer->GetActorLocation());
		PlayersInRange.RemoveSingle(ClosestPlayer);
	}
}

void APlayerCharacter::OnPlayersInRangeCollisionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (auto Other = Cast<APlayerCharacter>(OtherActor))
	{
		if(Other != this)
			PlayersInRange.AddUnique(Other);
	}
}

void APlayerCharacter::OnPlayersInRangeCollisionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (auto Other = Cast<APlayerCharacter>(OtherActor))
	{
		PlayersInRange.RemoveSingle(Other);
	}
}

void APlayerCharacter::UpdateClosestTaskObject()
{
	if (!TaskObjectsInCameraRange.Num())
		return;

	float Closest = MAX_FLT;
	if (ClosestPickup)
	{
		ClosestPickup->SetSelected(false);
		ClosestPickup = nullptr;
	}

	for (int i = 0; i < TaskObjectsInCameraRange.Num(); ++i)
	{
		auto Distance = FVector::Distance(TaskObjectsInCameraRange[i]->GetActorLocation(), GetActorLocation());
		if (Distance < Closest)
		{
			Closest = Distance;
			ClosestPickup = TaskObjectsInCameraRange[i];
		}
	}

	if (ClosestPickup)
		ClosestPickup->SetSelected(true);

	if (TaskObjectsInPickupRange.Find(ClosestPickup) != INDEX_NONE)
		OnObjectPickedUp(ClosestPickup);
}





ARailing* APlayerCharacter::GetClosestRail()
{
	ARailing* rail{nullptr};
	float currentRange{MAX_FLT};

	for (auto& item : RailsInRange)
	{
		if (item)
		{
			auto range = (item->GetActorLocation() - GetActorLocation()).Size();
			if (range < currentRange)
			{
				currentRange = range;
				rail = item;
			}
		}
	}

	return rail;
}

void APlayerCharacter::SetRailCollision(bool mode)
{
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel3, mode ? ECollisionResponse::ECR_Block : ECollisionResponse::ECR_Ignore);
}

bool APlayerCharacter::CanGrind() const
{
	return Movement->IsFalling() && CurrentGrindingRail == nullptr /* && !Movement->CurrentSpline.HasValue() */;
}

void APlayerCharacter::StartGrinding(ARailing* rail)
{
	// Start grinding
	CurrentGrindingRail = rail;
	Movement->SetSpline(rail->SplineComp, rail->bLoopedRail);
	Movement->SetMovementMode(EMovementMode::MOVE_Custom, static_cast<uint8>(ECustomMovementType::MOVE_Grinding));
}

void APlayerCharacter::OnGrindingOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	auto rail = Cast<ARailing>(OtherActor);
	if (rail)
	{
		RailsInRange.Add(rail);

		if (CanGrind())
			StartGrinding(rail);
	}
}

void APlayerCharacter::OnGrindingOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	auto rail = Cast<ARailing>(OtherActor);
	if (rail)
	{
		RailsInRange.RemoveSingle(rail);
		if (CurrentGrindingRail == rail)
			CurrentGrindingRail = nullptr;
	}
}



TArray<FVector> APlayerCharacter::GetWheelLocations() const
{
	constexpr int32 wheelCount = 4;
	TArray<FVector> locations;
	locations.Reserve(wheelCount);
	locations.AddZeroed(wheelCount);
	
	if (!IsValid(SkateboardMesh))
		return locations;

	auto compTrans = SkateboardMesh->GetComponentSpaceTransforms();

	const FName bones[]{"WheelsBack", "WheelsFront"};
	FTransform boneTrans = FTransform::Identity;
	for (int i{0}; i < wheelCount; ++i)
	{
		auto boneIndex = SkateboardMesh->GetBoneIndex(bones[i / 2]);
		if (boneIndex == INDEX_NONE)
		{
			++i; // Add one to skip both wheels on bone
			continue;
		}

		// Only update each other bone (for efficiency)
		if (!(i % 2))
			boneTrans = SkateboardMesh->GetBoneTransform(boneIndex);

		const FVector dir{0.f, (i % 2 ? 1.f : -1.f) * ParticleWheelOffset, 0.f};
		locations[i] = boneTrans.TransformPosition(dir);
	}

	return locations;
}

