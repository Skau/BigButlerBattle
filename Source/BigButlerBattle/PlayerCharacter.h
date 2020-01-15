// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "UObject/UObjectGlobals.h"
#include "TimerManager.h"
#include "Engine/EngineTypes.h"
#include "PlayerCharacter.generated.h"

class UPlayerCharacterMovementComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class USpringArmComponent;
class USkeletalMeshSocket;

USTRUCT(BlueprintType)
struct FSkateboardTraceResult
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FHitResult Front;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FHitResult Back;

	FSkateboardTraceResult()
	{}
};

UCLASS()
class BIGBUTLERBATTLE_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter(const FObjectInitializer& ObjectInitializer);

	void EnableRagdoll();

	bool HasEnabledRagdoll() { return bEnabledRagdoll; }
	bool CanFall() { return bCanFall; }
	float GetSidewaysForceFallOffThreshold() { return SidewaysForceFallOffThreshold; }

	UFUNCTION(BlueprintPure)
	FSkateboardTraceResult GetSkateboardTraceResults() const { return LastTraceResult;  }

	UFUNCTION(BlueprintCallable)
	bool TraceSkateboard();

	bool IsSocketsValid() const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (DisplayName = "Handbrake Rotation"))
	float HandbrakeRotationFactor = 300.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (DisplayName = "Handbreake Velocity Threshold"))
	float HandbreakeVelocityThreshold = 300.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (DisplayName = "Can Fall Off"))
	bool bCanFall = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (DisplayName = "Sideways Force Fall Off Threshold"))
	float SidewaysForceFallOffThreshold = 4000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (DisplayName = "Skateboard Ground Rotation Speed", ClampMin = "0", UIMin = "0", ClampMax = "1", UIMax = "1"))
	float SkateboardRotationGroundSpeed = 0.16f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (DisplayName = "Skateboard AirRotation Speed", ClampMin = "0", UIMin = "0", ClampMax = "1", UIMax = "1"))
	float SkateboardRotationAirSpeed = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bDebugMovement = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (DisplayName = "Camera Rotation Speed"))
	float CameraRotationSpeed = 200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (DisplayName = "Max CameraRotationOffset", ShortTooltip = "In angles"))
	float MaxCameraRotationOffset = 90.f;


	virtual void BeginPlay() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	virtual void Tick(float DeltaTime) override;


	bool bAllowBrakingWhileHandbraking = false;

	void MoveForward(float Value);

	void MoveRight(float Value);

	void LookUp(float Value);

	void LookRight(float Value);

	void Handbrake();

	void LetGoHandBrake();

	UPROPERTY(VisibleAnywhere)
	UPlayerCharacterMovementComponent* Movement;

	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* SkateboardMesh;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* SpringArm;

	const USkeletalMeshSocket* LinetraceSocketFront = nullptr;

	const USkeletalMeshSocket* LinetraceSocketBack = nullptr;

	FTimerHandle HandbrakeHandle;
	FTimerDelegate HandbrakeTimerCallback;
	bool bCurrentlyHandbraking = false;

	bool bCurrentlyHoldingHandbrake = false;
	float RightAxis = 0.0f;

	bool bEnabledRagdoll = false;

	FSkateboardTraceResult LastTraceResult;

	void UpdateCameraRotation(float DeltaTime);

	void UpdateSkateboardRotation(float DeltaTime);

	FQuat GetDesiredRotation(FVector DestinationNormal) const;
};
