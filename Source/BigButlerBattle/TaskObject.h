// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "DataTables.h"
#include "TaskObject.generated.h"

class UBoxComponent;

UCLASS()
class BIGBUTLERBATTLE_API ATaskObject : public AActor
{
	GENERATED_BODY()
	
public:	
	ATaskObject();

	FString GetObjectName() { return ObjectName; }
	EObjectType GetObjectType() { return ObjectType; }
	FBaseTableData* GetObjectData() { return AssetData; }

protected:
	void BeginPlay() override;

	void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComponent = nullptr;

	UPROPERTY(EditAnywhere)
	EObjectType ObjectType = EObjectType::None;

	UPROPERTY()
	FString ObjectName = "";

	UPROPERTY(EditDefaultsOnly)
	UStaticMesh* DefaultMesh = nullptr;

	UPROPERTY(EditDefaultsOnly)
	UMaterialInterface* DefaultMaterial = nullptr;

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent);

private:
	bool SetDataFromTable(EObjectType Type);

	UDataTable* WineDataTable = nullptr;
	UDataTable* FoodDataTable = nullptr;

	FBaseTableData* AssetData = nullptr;

	void SetDefault();
};