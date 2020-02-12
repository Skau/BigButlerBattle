// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "UObject/UObjectGlobals.h"
#include "TimerManager.h"
#include "Engine/EngineTypes.h"
#include "PlayerCharacter.generated.h"

// Forward declarations
class ABigButlerBattleGameModeBase;
class UPlayerCharacterMovementComponent;
class USkeletalMeshComponent;
class UCharacterAnimInstance;
class UPlayerCameraComponent;
class USpringArmComponent;
class USkeletalMeshSocket;
class UBoxComponent;
class ATaskObject;
class UTask;

// Delegates
DECLARE_DELEGATE(FCharacterFellSignature);
DECLARE_DELEGATE_OneParam(FTaskObjectPickedUpSignature, ATaskObject*);
DECLARE_DELEGATE_OneParam(FTaskObjectDroppedSignature, ATaskObject*);
DECLARE_MULTICAST_DELEGATE(JumpEventSignature);

// Structs
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


/** Main player class
 * ACharacter class used by all players.
 */
UCLASS()
class BIGBUTLERBATTLE_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly)
	ABigButlerBattleGameModeBase* GameMode;

public:
	APlayerCharacter(const FObjectInitializer &ObjectInitializer);

	UFUNCTION(BlueprintCallable)
	void AddForwardInput();

	FCharacterFellSignature OnCharacterFell;

protected:
	virtual void BeginPlay() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	virtual void Tick(float DeltaTime) override;





/// ==================================== Anim =================================================
private:
	UCharacterAnimInstance* AnimInstance = nullptr;




/// ==================================== Ragdoll =================================================

private:
	bool bEnabledRagdoll = false;

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ragdoll", meta = (DisplayName = "Can Fall Off"))
	bool bCanFall = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ragdoll", meta = (DisplayName = "Sideways Force Fall Off Threshold"))
	float SidewaysForceFallOffThreshold = 5000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ragdoll", meta = (DisplayName = "Crash Velocity Fall Off Threshold"))
	float CrashVelocityFallOffThreshold = 1000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ragdoll", meta = (DisplayName = "Crash Angle Threshold"))
	float CrashAngleThreshold = 45.f;


public:
	void EnableRagdoll(FVector Impulse = FVector::ZeroVector, FVector HitLocation = FVector::ZeroVector);
	bool HasEnabledRagdoll() const { return bEnabledRagdoll; }
	bool CanFall() const { return bCanFall; }
	float GetSidewaysForceFallOffThreshold() const { return SidewaysForceFallOffThreshold; }
	float GetCrashVelocityFallOffThreshold() const { return CrashVelocityFallOffThreshold; }
	float GetCrashAngleThreshold() const { return CrashAngleThreshold; }
	UFUNCTION()
	void OnCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);





	/// ==================================== Movement =================================================

protected:
	UPROPERTY(VisibleAnywhere)
	UPlayerCharacterMovementComponent *Movement = nullptr;

public:
	JumpEventSignature OnJumpEvent;

	void StartJump();

	UPlayerCharacterMovementComponent* GetPlayerCharacterMovementComponent() { return Movement; }

protected:
	void MoveForward(float Value);

	void MoveRight(float Value);

	void UpdateHandbrake(float Value);




	/// ==================================== Camera =================================================

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (DisplayName = "Rotation Speed"))
	float CameraRotationSpeed = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (DisplayName = "Rotation Dead Zone"))
	float CameraRotationDeadZone = 0.1f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (DisplayName = "Rotation Yaw Angle"))
	float CameraRotationYawAngle = 120.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (DisplayName = "Invert X"))
	bool CameraInvertX = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (DisplayName = "Invert Y"))
	bool CameraInvertY = false;

	/**
	 * Both the pitch rotation of the camera but as the camera also moves up/down from the character this also controls the height.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (DisplayName = "Rotation Pitch Height"))
	float CameraRotationPitchHeight = 1.2f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	float CustomSpringArmLength = 450.f;

	UPROPERTY(VisibleAnywhere)
	UPlayerCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere)
	USpringArmComponent *SpringArm;

	// The target rotation set by the player
	FVector2D DesiredCameraRotation = {};
	// The current actual rotation
	FVector2D CameraRotation = {};

	float DefaultSpringArmLength;

public:
	void SetCustomSpringArmLength();

protected:
	void UpdateCameraRotation(float DeltaTime);

	void LookUp(float Value);

	void LookRight(float Value);






/// ==================================== Skateboard rotation =================================================

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	USkeletalMeshComponent* SkateboardMesh;

	const USkeletalMeshSocket *LinetraceSocketFront = nullptr;

	const USkeletalMeshSocket *LinetraceSocketBack = nullptr;

	FSkateboardTraceResult LastTraceResult;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skateboard Rotation", meta = (DisplayName = "Ground Rotation Speed", ClampMin = "0", UIMin = "0", ClampMax = "1", UIMax = "1"))
	float SkateboardRotationGroundSpeed = 0.16f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skateboard Rotation", meta = (DisplayName = "AirRotation Speed", ClampMin = "0", UIMin = "0", ClampMax = "1", UIMax = "1"))
	float SkateboardRotationAirSpeed = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skateboard Rotation")
	bool bDebugMovement = false;

public:
	USkeletalMeshComponent* GetSkateboardMesh() { return SkateboardMesh; }

	FRotator GetSkateboardRotation() const;

	UFUNCTION(BlueprintPure)
	FSkateboardTraceResult GetSkateboardTraceResults() const { return LastTraceResult;  }

	UFUNCTION(BlueprintCallable)
	bool TraceSkateboard();

	bool IsSocketsValid() const;

protected:
	void UpdateSkateboardRotation(float DeltaTime);

	FQuat GetDesiredRotation(FVector DestinationNormal) const;







/// ==================================== Tasks =================================================

public:
	FTaskObjectPickedUpSignature OnTaskObjectPickedUp;
	FTaskObjectDroppedSignature OnTaskObjectDropped;

protected:
	UPROPERTY(VisibleAnywhere)
	UBoxComponent* ObjectPickupCollision;

	UPROPERTY(VisibleAnywhere)
	UCapsuleComponent* CapsuleObjectCollision;

	TArray<ATaskObject *> Inventory;
	TArray<ATaskObject *> PickupBlacklist;
	TArray<FName> TraySlotNames;

	TArray<ATaskObject*> TaskObjectsInRange;

	UPROPERTY()
	ATaskObject* ClosestPickup;

	UPROPERTY(EditDefaultsOnly)
	float ThrowStrength = 2000.f;

	bool bCurrentlyHoldingThrow = false;

	int CurrentItemIndex = 0;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Tray;

public:
	TArray<ATaskObject*>& GetInventory() { return Inventory; }

protected:
	void OnObjectPickedUp(ATaskObject* Object);

	void DropCurrentObject();

	void DetachObject(ATaskObject* Object, FVector SpawnLocation, FVector LaunchVelocity = FVector::ZeroVector);

	void OnHoldingThrow();
	void OnHoldThrowReleased();

	void IncrementCurrentItemIndex();
	void DecrementCurrentItemIndex();

	UFUNCTION()
	void OnObjectPickupCollisionOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnObjectPickupCollisionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnCapsuleObjectCollisionOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnCapsuleObjectCollisionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void UpdateClosestTaskObject();
};
