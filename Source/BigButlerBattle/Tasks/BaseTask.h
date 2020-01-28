// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Utils/DataTables.h"
#include "BaseTask.generated.h"


/**
 * 
 */
UCLASS(BlueprintType)
class BIGBUTLERBATTLE_API UBaseTask : public UObject
{
	GENERATED_BODY()

public:
    UBaseTask();

    virtual bool InitTaskData(uint8* Data);

    virtual bool IsEqual(const UBaseTask* Other) const { UE_LOG(LogTemp, Warning, TEXT("Base IsEqual Called")); return false; }

    UPROPERTY(EditAnywhere)
    FString Name;

    UPROPERTY(EditAnywhere)
    EObjectType Type;

    UPROPERTY(EditAnywhere)
    class UStaticMesh* Mesh;

    UPROPERTY(EditAnywhere)
    class UMaterialInterface* Material;
};




