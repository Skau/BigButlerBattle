// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "UObject/UObjectGlobals.h"
#include "PlayerCharacter.generated.h"

class UPlayerCharacterMovementComponent;
class USkeletalMeshComponent;

UCLASS()
class BIGBUTLERBATTLE_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(VisibleAnywhere)
	UPlayerCharacterMovementComponent* Movement;

	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* SkateboardMesh;
};
