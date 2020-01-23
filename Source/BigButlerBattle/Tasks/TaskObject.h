// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Utils/DataTables.h"
#include "TaskObject.generated.h"

class UBoxComponent;
class UDataTable;
class UBaseTask;

UCLASS()
class BIGBUTLERBATTLE_API ATaskObject : public AActor
{
	GENERATED_BODY()
	
public:	
	ATaskObject();

	UBaseTask* GetTaskData() { return TaskData; }

	void OnPickedUp();

protected:
	void BeginPlay() override;

	void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComponent = nullptr;

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent);

private:
	bool SetDataFromTable();
	bool SetDataFromAssetData();

	UDataTable* WineDataTable = nullptr;
	UDataTable* FoodDataTable = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Task")
	UStaticMesh* DefaultMesh = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Task")
	UMaterialInterface* DefaultMaterial = nullptr;

	UPROPERTY(EditAnywhere, Category = "Task")
	EObjectType TaskType = EObjectType::None;

	UPROPERTY(EditInstanceOnly, Category = "Task")
	bool bRespawn = false;

	UPROPERTY(EditInstanceOnly, Category = "Task")
	float RespawnTime = 15.f;

	UPROPERTY(EditInstanceOnly, Category = "Task")
	UBaseTask* TaskData = nullptr;

	void SetDefault();
};