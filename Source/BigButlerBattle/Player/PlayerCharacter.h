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
class UCameraComponent;
class USpringArmComponent;
class USkeletalMeshSocket;
class UBoxComponent;
class ATaskObject;
class UTask;

// Delegates
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

struct FFeetTransform
{
	FTransform Left;
	FTransform Right;

	FFeetTransform(const FTransform& left, const FTransform& right)
	 : Left{left}, Right{right}
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

protected:
	virtual void BeginPlay() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	virtual void Tick(float DeltaTime) override;





/// ==================================== Anim =================================================
private:
	UCharacterAnimInstance* AnimInstance = nullptr;




/// ==================================== Ragdoll =================================================

protected:
	bool bEnabledRagdoll = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ragdoll", meta = (DisplayName = "Can Fall Off"))
	bool bCanFall = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ragdoll", meta = (DisplayName = "Sideways Force Fall Off Threshold"))
	float SidewaysForceFallOffThreshold = 4000.f;

public:
	void EnableRagdoll();
	bool HasEnabledRagdoll() const { return bEnabledRagdoll; }
	bool CanFall() const { return bCanFall; }
	float GetSidewaysForceFallOffThreshold() const { return SidewaysForceFallOffThreshold; }





/// ==================================== IK =================================================

public:
	/**
	 * Returns the locations of the skateboard feet sockets in world space.
	 */
	FFeetTransform GetSkateboardFeetTransform() const;
	/**
	 * Returns the locations of the skateboard feet sockets in component space.
	 */
	FFeetTransform GetComponentSkateboardFeetTransform() const;

	FFeetTransform GetSkateboardFeetTransformInButlerSpace() const;
	
	FTransform GetCharacterBoneTransform(FName BoneName) const;
	FTransform GetCharacterBoneTransform(FName BoneName, const FTransform &localToWorld) const;
	FTransform GetCharacterRefPoseBoneTransform(FName BoneName) const;
	FTransform GetCharacterRefPoseBoneTransform(FName BoneNamem, const FTransform &localToWorld) const;

	/**
	 * Does a recursive search upwards to find the world space transform of the reference bone.
	 */
	FTransform GetCharacterRefPoseBoneTransformRec(FName BoneName) const;
	FTransform GetCharacterRefPoseBoneTransformRec(int32 BoneIndex) const;

	FTransform LocalSkateboardToButler(const FTransform& trans) const;
	FTransform LocalButlerToSkateboard(const FTransform& trans) const;







	/// ==================================== Movement =================================================

protected:
	UPROPERTY(VisibleAnywhere)
	UPlayerCharacterMovementComponent *Movement = nullptr;

public:
	JumpEventSignature OnJumpEvent;

	void StartJump();

protected:
	void MoveForward(float Value);

	void MoveRight(float Value);

	void HandbrakeEnable();

	void HandbrakeDisable();






	/// ==================================== Camera =================================================

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (DisplayName = "Rotation Speed"))
	float CameraRotationSpeed = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (DisplayName = "Rotation Dead Zone"))
	float CameraRotationDeadZone = 0.1f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (DisplayName = "Rotation Yaw Angle"))
	float CameraRotationYawAngle = 120.f;

	/**
	 * Both the pitch rotation of the camera but as the camera also moves up/down from the character this also controls the height.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (DisplayName = "Rotation Pitch Height"))
	float CameraRotationPitchHeight = 1.2f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	float CustomSpringArmLength = 450.f;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent *Camera;

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
