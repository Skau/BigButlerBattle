// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "Engine/EngineTypes.h"
#include "Utils/Spawnpoint.h"
#include "PlayerCharacter.generated.h"

// Forward declarations
class ABigButlerBattleGameModeBase;
class UPlayerCharacterMovementComponent;
class USkeletalMeshComponent;
class UCharacterAnimInstance;
class UPlayerCameraComponent;
class UPlayerSpringArmComponent;
class USkeletalMeshSocket;
class UBoxComponent;
class ATaskObject;
class UTask;
class UAudioComponent;
class ARailing;
class USphereComponent;
class UNiagaraComponent;
class UCurveFloat;

// Delegates
DECLARE_DELEGATE_TwoParams(FCharacterFellSignature, ERoomSpawn, FVector);
DECLARE_DELEGATE_OneParam(FTaskObjectPickedUpSignature, ATaskObject*);
DECLARE_DELEGATE_OneParam(FTaskObjectDroppedSignature, ATaskObject*);
DECLARE_DELEGATE_OneParam(FDeliverTasksSignature, TArray<ATaskObject*>&)
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

	ERoomSpawn CurrentRoom;

protected:
	void BeginPlay() override;

	void SetupPlayerInputComponent(class UInputComponent* Input) override;

	void Tick(float DeltaTime) override;





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
	void EnableRagdoll(const FVector& Impulse = FVector::ZeroVector, const FVector& HitLocation = FVector::ZeroVector);
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

	float highestInput = 0.f;

public:
	JumpEventSignature OnJumpEvent;

	void Jump() override;

	UPlayerCharacterMovementComponent* GetPlayerCharacterMovementComponent() const { return Movement; }

	FVector GetInputAxis() const;

protected:
	void MoveForward(float Value);

	void MoveRight(float Value);

	void UpdateHandbrake(float Value);

	void Brake(float Value);
	





	/// ==================================== Camera =================================================

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (DisplayName = "Rotation Speed"))
	float CameraRotationSpeed = 2.8f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (DisplayName = "Rotation Dead Zone"))
	float CameraRotationDeadZone = 0.001f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (DisplayName = "Rotation Yaw Angle"))
	float CameraRotationYawAngle = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (DisplayName = "Invert Yaw"))
	bool CameraInvertYaw = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (DisplayName = "Invert Pitch"))
	bool CameraInvertPitch = false;

	/**
	 * Both the pitch rotation of the camera but as the camera also moves up/down from the character this also controls the height.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (DisplayName = "Rotation Pitch Height"))
	float CameraRotationPitchHeight = 1.2f;



	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	float CustomSpringArmLength = 400.f;

	UPROPERTY(VisibleAnywhere)
	UPlayerCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere)
	UPlayerSpringArmComponent *SpringArm;


	FVector2D DefaultCameraRotation = {};
	// The target rotation set by the player
	FVector2D DesiredCameraRotation = {};
	// The current actual rotation
	FVector2D CameraRotation = {};

	float DefaultSpringArmLength;

public:
	void SetCustomSpringArmLength();

	void SetCameraInvertPitch(const bool& Value) { CameraInvertPitch = Value; }

	void SetCameraInvertYaw(const bool& Value) { CameraInvertYaw = Value; }

protected:
	void UpdateCameraRotation(float DeltaTime);

	void LookUp(float Value);

	void LookRight(float Value);






/// ==================================== Skateboard rotation =================================================

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	USkeletalMeshComponent* SkateboardMesh;

	const USkeletalMeshSocket* LinetraceSocketFront = nullptr;

	const USkeletalMeshSocket* LinetraceSocketBack = nullptr;

	FSkateboardTraceResult LastTraceResult;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skateboard Rotation", meta = (DisplayName = "Ground Rotation Speed", ClampMin = "0", UIMin = "0", ClampMax = "1", UIMax = "1"))
	float SkateboardRotationGroundSpeed = 0.16f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skateboard Rotation", meta = (DisplayName = "AirRotation Speed", ClampMin = "0", UIMin = "0", ClampMax = "1", UIMax = "1"))
	float SkateboardRotationAirSpeed = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skateboard Rotation", meta = (DisplayName = "GrindingRotation Speed", ClampMin = "0", UIMin = "0", ClampMax = "1", UIMax = "1"))
	float SkateboardRotationGrindingSpeed = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skateboard Rotation")
	bool bDebugMovement = false;

public:
	USkeletalMeshComponent* GetSkateboardMesh() const { return SkateboardMesh; }

	FRotator GetSkateboardRotation() const;
	FVector GetSkateboardLocation() const;

	UFUNCTION(BlueprintPure)
	FSkateboardTraceResult GetSkateboardTraceResults() const { return LastTraceResult;  }

	UFUNCTION(BlueprintCallable)
	bool TraceSkateboard();

	bool IsSocketsValid() const;

protected:
	void UpdateSkateboardRotation(float DeltaTime);

	FQuat GetDesiredRotation(const FVector& DestinationNormal) const;
	FQuat GetDesiredGrindingRotation(const FVector& DestinationNormal) const;







/// ==================================== Tasks =================================================

public:
	FTaskObjectPickedUpSignature OnTaskObjectPickedUp;
	FTaskObjectDroppedSignature OnTaskObjectDropped;
	FDeliverTasksSignature OnDeliverTasks;

	int GetCurrentItemIndex() const { return CurrentItemIndex; }
	void IncrementCurrentItemIndex();

	bool bHasMainItem = false;

protected:
	UPROPERTY(VisibleAnywhere)
	UBoxComponent* TaskObjectPickupCollision;

	UPROPERTY(VisibleAnywhere)
	UCapsuleComponent* TaskObjectCameraCollision;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ATaskObject> TaskObjectBlueprintOverride;

	TArray<ATaskObject*> Inventory;
	TArray<FName> TraySlotNames;
	FName MainSlotName;

	TArray<ATaskObject*> TaskObjectsInCameraRange;
	TArray<ATaskObject*> TaskObjectsInPickupRange;

	UPROPERTY()
	ATaskObject* ClosestPickup;

	UPROPERTY(EditDefaultsOnly)
	float ThrowStrength = 5000.f;

	UPROPERTY(EditDefaultsOnly)
	float AimbotMaxDistance = 3000.f;

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

	void ResetItemIndex();

	void UpdateClosestTaskObject();

	/* Delegates to handle actual pickup of tasks */

	UFUNCTION()
	void OnTaskObjectPickupCollisionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnTaskObjectPickupCollisionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/* Delegates to handle the ones in range based on camera */

	UFUNCTION()
	void OnTaskObjectCameraCollisionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnTaskObjectCameraCollisionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);






	/// ==================================== Tackling =================================================

private:
	TArray<APlayerCharacter*> PlayersInRange;

	void TryTackle();

protected:
	UPROPERTY(VisibleAnywhere)
	UBoxComponent* PlayersInRangeCollision;

	UPROPERTY(EditDefaultsOnly)
	bool CanTackle = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tackling", meta = (DisplayName = "Angle Threshold"))
	float TackleAngleThreshold = 45.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tackling", meta = (DisplayName = "Strength"))
	float TackleStrength = 100.f;

	UFUNCTION()
	void OnPlayersInRangeCollisionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnPlayersInRangeCollisionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);





/// ========================================= Sounds =================================================
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Sound")
	UAudioComponent* ContinuousSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundBase *JumpSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundBase *LandSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundBase *PickupSound = nullptr;





/// ==================================== Grinding =================================================
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Grinding")
	USphereComponent* GrindingOverlapThreshold;

	TArray<ARailing*> RailsInRange;
	ARailing* CurrentGrindingRail{nullptr};

	ARailing* GetClosestRail();

public:
	void SetRailCollision(bool mode);
	bool CanGrind() const;
	void StartGrinding(ARailing* rail);

	UFUNCTION()
	void OnGrindingOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnGrindingOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);






/// =================================== Particles =================================================
protected:
	UPROPERTY(VisibleAnywhere, Category = "Particles")
	UNiagaraComponent *SkateboardParticles = nullptr;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
	float ParticleWheelOffset = 0.07f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
	float SkidmarkVelocityThreshold = 1500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
	UCurveFloat* SkidmarkStrengthCurve = nullptr;

	TArray<FVector> GetWheelLocations() const;
};
