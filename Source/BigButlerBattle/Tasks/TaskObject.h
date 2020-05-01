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
class UNiagaraComponent;

UCLASS()
class BIGBUTLERBATTLE_API ATaskObject : public AActor
{
	GENERATED_BODY()
	
public:	
	ATaskObject();

	UTask* GetTaskData() { return TaskData; }
	void SetTaskData(UTask* Task) { TaskData = Task; }

	void OnPickedUp();

	void Enable(bool NewVisiblity, bool NewCollision, bool NewPhysics);

	void Launch(const FVector& LaunchVelocity);

	void SetSelected(bool Value);

	bool bCanHit = false;

	void SetAsMainItem();

	bool GetIsMainItem() { return bIsMainItem; }

	void Reset();

	bool GetIsRespawning() const { return bIsRespawning; }

	void SetParticlesEnable(bool bEnable);

	bool bOnTray = false;

protected:
	void BeginPlay() override;

	void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComponent = nullptr;

	UPROPERTY(VisibleAnywhere)
	UNiagaraComponent* Particles = nullptr;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent);
#endif

private:
	bool SetDataFromTable();
	bool SetDataFromAssetData();

	UPROPERTY(EditAnywhere)
	bool bIsMainItem = false;

	UPROPERTY(EditDefaultsOnly)
	uint8 MainItemStencilValue = 100;

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

	bool bIsRespawning = false;

	UPROPERTY(EditAnywhere, Category = "Task")
	float RespawnTime = 5.f;

	UPROPERTY(EditAnywhere, Category = "Task")
	float TimeUntilResetThreshold = 20.f;

	UPROPERTY(EditInstanceOnly, Category = "Task")
	UTask* TaskData = nullptr;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void OnWake(UPrimitiveComponent* WakingComponent, FName BoneName);

	UFUNCTION()
	void OnSleep(UPrimitiveComponent* SleepingComponent, FName BoneName);

	void SetDefault();

	float TimeSinceDropped = 0.0f;
	bool bRecordingTimeSinceDropped = false;
	float ObjectIdleTimer = 0.f;
	uint8 CleanupTime = 10;
	uint8 ParticleDisableTime = 4;
	bool bEnabled = true;

private:
	void UpdateDataTables();
};