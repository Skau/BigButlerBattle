// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "King.generated.h"

class UBoxComponent;
class UBaseTask;

UCLASS()
class BIGBUTLERBATTLE_API AKing : public AActor
{
	GENERATED_BODY()
	
public:	
	AKing();

	bool CheckIfDone(UBaseTask* TaskIn);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* BoxCollision;

private:
	TArray<UBaseTask*> Tasks;

	UFUNCTION()
	void OnTasksFinishedGenerated(const TArray<UBaseTask*>& TasksIn);

	UFUNCTION()
	void OnBoxCollisionOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

};
