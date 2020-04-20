// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "PlayerCameraComponent.generated.h"


class APlayerCharacter;
class UCurveFloat;

/**
 * 
 */
UCLASS()
class BIGBUTLERBATTLE_API UPlayerCameraComponent : public UCameraComponent
{
	GENERATED_BODY()

public:
	UPlayerCameraComponent();

protected:
	void BeginPlay() override;
	
	void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditDefaultsOnly)
	float FieldOfViewSpeedChange = 17.f;

	/**
	 * Minimum FOV. FOV will never go below this.
	 */
	UPROPERTY(EditDefaultsOnly)
	float MinFOV = 90.f;

	/**
	 * Maximum FOV under max input speed (the speed you can manually kick youself up to)
	 */
	UPROPERTY(EditDefaultsOnly)
	float MaxPlayerInputFOV = 110.f;

	/**
	 * Maximim FOV. FOV will never surpass this.
	 */
	UPROPERTY(EditDefaultsOnly)
	float MaxFOV = 160.f;

	UPROPERTY(EditDefaultsOnly)
	UCurveFloat* FOVCurve = nullptr;

	/**
	 * Whether to only use velocity in the x and y axis when figuring out current FOV
	 * as opposed to also taking into account the z axis.
	 */
	UPROPERTY(EditDefaultsOnly, meta = (DisplayName = "Constrain FOV Change To Velocity In XY Directions"))
	bool bConstrainFOVChangeToVelocityInXYDirections = true;

	UPROPERTY(EditDefaultsOnly)
	bool bScaleChromaticAberrationByVelocityCurve = false;

	UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "bScaleChromaticAberrationByVelocityCurve"))
	UCurveFloat* ChromaticAberrationVelocityCurve = nullptr;

private:
	APlayerCharacter* Player;
};
