// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Spawnpoint.h"
#include "RoomUpdater.generated.h"

class UBoxComponent;

UCLASS()
class BIGBUTLERBATTLE_API ARoomUpdater : public AActor
{
	GENERATED_BODY()
	
public:	
	ARoomUpdater();

	void BeginPlay() override;

protected:
	UPROPERTY(VisibleAnywhere)
	UBoxComponent* Collision;

	UPROPERTY(EditAnywhere)
	ERoomSpawn RoomConnection1;

	UPROPERTY(EditAnywhere)
	ERoomSpawn RoomConnection2;

private:
	UFUNCTION()
	void OnCollisionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
