// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpringArmComponent.h"
#include "PlayerSpringArmComponent.generated.h"

/**
 * 
 */
UCLASS()
class BIGBUTLERBATTLE_API UPlayerSpringArmComponent : public USpringArmComponent
{
	GENERATED_BODY()

protected:
	/**
	 * Whether to constrain camera lag to xy plane (only movement not rotation)
	 */ 
	UPROPERTY(EditDefaultsOnly)
	bool bConstrainLagToXYPlane = true;

	// Begin USpringArmComponent interface
	/** Updates the desired arm location, calling BlendLocations to do the actual blending if a trace is done */
	virtual void UpdateDesiredArmLocation(bool bDoTrace, bool bDoLocationLag, bool bDoRotationLag, float DeltaTime) override;
	// End USpringArmComponent interface
};
