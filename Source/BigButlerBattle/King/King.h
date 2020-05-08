// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "King.generated.h"

class UBoxComponent;
class APlayerCharacter;

UCLASS()
class BIGBUTLERBATTLE_API AKing : public AActor
{
	GENERATED_BODY()
	
public:	
	AKing();

	bool bCanReceiveMainItem = false;

	// Called by gamemode when timer runs out
	bool CheckIfAnyInRange();

	void AddClosePlayer(APlayerCharacter* Player);
	void RemoveClosePlayer(APlayerCharacter* Player);

protected:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* BoxCollision;

private:
	TArray<APlayerCharacter*> PlayersInRange;

};
