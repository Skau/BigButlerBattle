// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OutOfBounds.generated.h"

class UBoxComponent;

UCLASS()
class BIGBUTLERBATTLE_API AOutOfBounds : public AActor
{
	GENERATED_BODY()
	
public:	
	AOutOfBounds();

	UPROPERTY(EditAnywhere)
	bool bCanAffectPlayers = true;

protected:
	void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* Collision;

private:
	UFUNCTION()
	void OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

};
