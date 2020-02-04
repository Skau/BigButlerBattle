// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Utils/DataTables.h"
#include "Task.generated.h"


/**
 * 
 */
UCLASS(BlueprintType)
class BIGBUTLERBATTLE_API UTask : public UObject
{
	GENERATED_BODY()

public:
    UTask();

    bool InitTaskData(uint8* Data);

    bool IsEqual(const UTask* Other) const;

    UPROPERTY(EditAnywhere)
    FString Name;

    UPROPERTY(EditAnywhere)
    EObjectType Type;

    UPROPERTY(EditAnywhere)
    class UStaticMesh* Mesh;

    UPROPERTY(EditAnywhere)
    class UMaterialInterface* Material;
};




