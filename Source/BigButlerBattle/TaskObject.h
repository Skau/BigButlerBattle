// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DataTables.h"
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

protected:
	void BeginPlay() override;

	void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComponent = nullptr;

	UPROPERTY(EditAnywhere)
	EObjectType TaskType = EObjectType::None;

	UPROPERTY(EditDefaultsOnly)
	UStaticMesh* DefaultMesh = nullptr;

	UPROPERTY(EditDefaultsOnly)
	UMaterialInterface* DefaultMaterial = nullptr;

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent);

private:
	bool SetDataFromTable();
	bool SetDataFromAssetData();

	UDataTable* WineDataTable = nullptr;
	UDataTable* FoodDataTable = nullptr;

	UPROPERTY(EditInstanceOnly)
	UBaseTask* TaskData = nullptr;

	void SetDefault();
};