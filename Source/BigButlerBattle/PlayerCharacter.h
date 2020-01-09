// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "UObject/UObjectGlobals.h"
#include "TimerManager.h"
#include "PlayerCharacter.generated.h"

class UPlayerCharacterMovementComponent;
class USkeletalMeshComponent;

UCLASS()
class BIGBUTLERBATTLE_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter(const FObjectInitializer& ObjectInitializer);

	void ToggleHoldingHandbrake(bool Value) { bCurrentlyHoldingHandbrake = Value; }
	void SetRightAxisValue(float Value) { RightAxis = Value; }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (DisplayName = "Handbrake Rotation"))
	float HandbrakeRotationFactor = 300.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (DisplayName = "Handbreake Velocity Threshold"))
	float HandbreakeVelocityThreshold = 70.f;

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(VisibleAnywhere)
	UPlayerCharacterMovementComponent* Movement;

	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* SkateboardMesh;

	FTimerHandle HandbrakeHandle;
	FTimerDelegate HandbrakeTimerCallback;
	bool bCurrentlyHandbraking = false;

	bool bCurrentlyHoldingHandbrake = false;
	float RightAxis = 0.0f;
};
