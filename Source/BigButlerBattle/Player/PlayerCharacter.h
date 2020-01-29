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
class UCameraComponent;
class USpringArmComponent;
class USkeletalMeshSocket;
class UBoxComponent;
class ATaskObject;
class UTask;

// Delegates
DECLARE_DELEGATE(FTaskObjectPickedUpSignature);
DECLARE_DELEGATE(FTaskObjectDroppedSignature);
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

protected:
	virtual void BeginPlay() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	virtual void Tick(float DeltaTime) override;







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
	TPair<FVector, FVector> GetSkateboardFeetLocations() const;
	FTransform GetCharacterBoneTransform(FName BoneName) const;
	FTransform GetCharacterBoneTransform(FName BoneName, const FTransform &localToWorld) const;
	FTransform GetCharacterRefPoseBoneTransform(FName BoneName) const;
	FTransform GetCharacterRefPoseBoneTransform(FName BoneNamem, const FTransform &localToWorld) const;






	/// ==================================== Movement =================================================

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (DisplayName = "Skateboard Ground Rotation Speed", ClampMin = "0", UIMin = "0", ClampMax = "1", UIMax = "1"))
	float SkateboardRotationGroundSpeed = 0.16f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (DisplayName = "Skateboard AirRotation Speed", ClampMin = "0", UIMin = "0", ClampMax = "1", UIMax = "1"))
	float SkateboardRotationAirSpeed = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bDebugMovement = false;

	UPROPERTY(VisibleAnywhere)
	UPlayerCharacterMovementComponent *Movement = nullptr;

public:
	JumpEventSignature OnJumpEvent;

	void Jump() override;

protected:
	void MoveForward(float Value);

	void MoveRight(float Value);

	void HandbrakeEnable();

	void HandbrakeDisable();






	/// ==================================== Camera =================================================

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (DisplayName = "Rotation Speed"))
	float CameraRotationSpeed = 400.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (DisplayName = "Snapback Speed"))
	float CameraSnapbackSpeed = 200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (DisplayName = "Max Rotation Offset", ShortTooltip = "In angles"))
	float MaxCameraRotationOffset = 89.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	float CustomSpringArmLength = 450.f;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent *Camera;

	UPROPERTY(VisibleAnywhere)
	USpringArmComponent *SpringArm;

	float CameraYaw = 0.f;
	float CameraPitch = 0.f;

public:
	void SetCustomSpringArmLength();

protected:
	void LookUp(float Value);

	void LookRight(float Value);






/// ==================================== Skateboard rotation =================================================

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	USkeletalMeshComponent* SkateboardMesh;

	const USkeletalMeshSocket *LinetraceSocketFront = nullptr;

	const USkeletalMeshSocket *LinetraceSocketBack = nullptr;

	FSkateboardTraceResult LastTraceResult;

public:
	UFUNCTION(BlueprintPure)
	FSkateboardTraceResult GetSkateboardTraceResults() const { return LastTraceResult;  }

	UFUNCTION(BlueprintCallable)
	bool TraceSkateboard();

	bool IsSocketsValid() const;

protected:
	void UpdateCameraRotation(float DeltaTime);

	void UpdateSkateboardRotation(float DeltaTime);

	FQuat GetDesiredRotation(FVector DestinationNormal) const;







/// ==================================== Tasks =================================================

public:
	FTaskObjectPickedUpSignature OnTaskObjectPickedUp;
	FTaskObjectDroppedSignature OnTaskObjectDropped;

protected:
	UPROPERTY(VisibleAnywhere)
	UBoxComponent* ObjectPickupCollision;
	TArray<ATaskObject *> Inventory;
	TArray<ATaskObject *> PickupBlacklist;
	TArray<FName> TraySlotNames;

	int CurrentItemIndex = 0;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Tray;

public:
	TArray<ATaskObject*>& GetInventory() { return Inventory; }

protected:
	void OnObjectPickedUp(ATaskObject* Object);

	void DropCurrentObject();

	void IncrementCurrentItemIndex();
	void DecrementCurrentItemIndex();

	UFUNCTION()
	void OnObjectPickupCollisionOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnObjectPickupCollisionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

};
