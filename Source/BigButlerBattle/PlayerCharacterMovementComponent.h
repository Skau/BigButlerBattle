// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PlayerCharacterMovementComponent.generated.h"

// Forward declarations
class APlayerCharacter;

UENUM(BlueprintType)
enum class ECustomMovementType : uint8
{
	MOVE_None			UMETA(DisplayName = "None"),
	MOVE_Skateboard		UMETA(DisplayName = "Skateboard")
};

/** Custom override of movement component
 * 
 */
UCLASS()
class BIGBUTLERBATTLE_API UPlayerCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Custom Movement")
	ECustomMovementType CurrentCustomMovementMode = ECustomMovementType::MOVE_Skateboard;

	bool bStandstill = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Custom Movement")
	float StandstillThreshold = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Custom Movement", meta = (DisplayName = "Ground Friction"))
	float SkateboardGroundFriction = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Custom Movement", meta = (DisplayName = "Braking Deceleration"))
	float SkateboardBrakingDeceleration = 100.f;

public:
	UPlayerCharacterMovementComponent();

	UFUNCTION(BlueprintPure)
	bool IsStandstill() const { return bStandstill; }

	bool IsMovingOnGround() const override;

protected:
	void BeginPlay() override;

	void TickComponent(float deltaTime, enum ELevelTick TickType, FActorComponentTickFunction* thisTickFunction) override;

	/** @note Movement update functions should only be called through StartNewPhysics()*/
	void PhysCustom(float deltaTime, int32 Iterations) override;

	void PhysSkateboard(float deltaTime, int32 Iterations);

	void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

private:
	FVector InputDir;

	void CalcSkateboardVelocity(float DeltaTime);

	FORCEINLINE float GetRotationInput() const { return InputDir.Y; }
	FORCEINLINE FVector GetForwardInput() const { return FVector{InputDir.X, 0, 0}; }
	FORCEINLINE FVector CalcAcceleration() const { return GetForwardInput() * GetMaxAcceleration(); }
};
