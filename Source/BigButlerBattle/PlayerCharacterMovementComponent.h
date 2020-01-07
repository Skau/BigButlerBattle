// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PlayerCharacterMovementComponent.generated.h"

// Forward declarations
class APlayerCharacter;

UENUM(BlueprintType)
enum class ECustomMovementTypeEnum : uint8
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
	ECustomMovementTypeEnum CurrentCustomMovementMode = ECustomMovementTypeEnum::MOVE_Skateboard;

public:
	UPlayerCharacterMovementComponent();

protected:
	void BeginPlay() override;

	void TickComponent(float deltaTime, enum ELevelTick TickType, FActorComponentTickFunction* thisTickFunction) override;

	/** @note Movement update functions should only be called through StartNewPhysics()*/
	void PhysCustom(float deltaTime, int32 Iterations) override;

	void PhysSkateboard(float deltaTime, int32 Iterations);
};
