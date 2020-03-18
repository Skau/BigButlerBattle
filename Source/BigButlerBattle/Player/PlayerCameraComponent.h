// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "PlayerCameraComponent.generated.h"


class APlayerCharacter;

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

	UPROPERTY(EditDefaultsOnly)
	float MinFOV = 115.f;

	UPROPERTY(EditDefaultsOnly)
	float MaxFOV = 130.f;

	/**
	 * Whether to only use velocity in the x and y axis when figuring out current FOV
	 * as opposed to also taking into account the z axis.
	 */
	UPROPERTY(EditDefaultsOnly, meta = (DisplayName = "Constrain FOV Change To Velocity In XY Directions"))
	bool bConstrainFOVChangeToVelocityInXYDirections = true;

private:
	APlayerCharacter* Player;
};
