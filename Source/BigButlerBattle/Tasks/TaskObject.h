// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Utils/DataTables.h"
#include "TaskObject.generated.h"

class ATaskObject;
class UBoxComponent;
class UDataTable;
class UTask;
class APlayerCharacter;

DECLARE_DELEGATE_OneParam(FTaskObjectDeliveredSignature, ATaskObject*);

UCLASS()
class BIGBUTLERBATTLE_API ATaskObject : public AActor
{
	GENERATED_BODY()
	
public:	
	ATaskObject();

	UTask* GetTaskData() { return TaskData; }
	void SetTaskData(UTask* Task) { TaskData = Task; }

	void OnPickedUp();

	void Enable(bool NewVisiblity, bool NewCollision, bool NewPhysics) const;

	void Launch(const FVector& LaunchVelocity);

	FTaskObjectDeliveredSignature OnTaskObjectDelivered;

	void SetSelected(bool Value);

	bool bCanHit = false;

	APlayerCharacter* Instigator = nullptr;

	UPROPERTY(EditAnywhere)
	bool bIsMainItem = false;

protected:
	void BeginPlay() override;

	void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComponent = nullptr;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent);
#endif

private:
	bool SetDataFromTable();
	bool SetDataFromAssetData();

	UPROPERTY()
	UDataTable* DrinksDataTable = nullptr;
	
	UPROPERTY()
	UDataTable* FoodDataTable = nullptr;

	UPROPERTY()
	UDataTable* CutleryDataTable = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Task")
	UStaticMesh* DefaultMesh = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Task")
	UMaterialInterface* DefaultMaterial = nullptr;

	UMaterialInstanceDynamic* DynamicMaterial = nullptr;

	UPROPERTY(EditAnywhere, Category = "Task")
	EObjectType TaskType = EObjectType::None;

	UPROPERTY(EditAnywhere, Category = "Task")
	bool bRespawn = false;

	UPROPERTY(EditAnywhere, Category = "Task")
	float RespawnTime = 15.f;

	UPROPERTY(EditAnywhere, Category = "Task")
	float CountAsPlayerTaskThreshold = 10.f;

	UPROPERTY(EditInstanceOnly, Category = "Task")
	UTask* TaskData = nullptr;

	UFUNCTION()
	void OnCollisionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void SetDefault();

	float TimeSinceThrown = 0.0f;
	bool bRecordingTimeSinceThrown = false;

private:
	void UpdateDataTables();
};