// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "King.generated.h"

class UBoxComponent;

UCLASS()
class BIGBUTLERBATTLE_API AKing : public AActor
{
	GENERATED_BODY()
	
public:	
	AKing();

	bool GetCanReceiveMainItem() const { return bCanReceiveMainItem; }
	void UpdateCanReceiveMainItem() { bCanReceiveMainItem = true; }

protected:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* BoxCollision;

private:
	bool bCanReceiveMainItem = false;
};
