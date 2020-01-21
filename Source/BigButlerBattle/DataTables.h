// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DataTables.generated.h"

UENUM(BlueprintType)
enum class EObjectType : uint8
{
    None,
    Wine,
    Food
};

USTRUCT(BlueprintType)
struct FBaseTableData : public FTableRowBase
{
    GENERATED_BODY()

    FBaseTableData()
    : Type(EObjectType::None)
    , Mesh(nullptr)
    , Material(nullptr)
    {}

    virtual bool IsEqual(FBaseTableData* Other) { return false; }

    UPROPERTY(EditAnywhere)
    EObjectType Type;

    UPROPERTY(EditAnywhere)
    class UStaticMesh* Mesh;

    UPROPERTY(EditAnywhere)
    class UMaterialInterface* Material;
};

USTRUCT(BlueprintType)
struct FWineTableData : public FBaseTableData
{
    GENERATED_BODY()

    FWineTableData()
    : Year(0)
    {}

    virtual bool IsEqual(FBaseTableData* Other) override
    {
        auto OtherData = (FWineTableData*)(Other);
        if (OtherData)
        {
            if (OtherData == this)
            {
                return true;
            }
        }

        return false;
    }

    UPROPERTY(EditAnywhere)
    int Year;
};


USTRUCT(BlueprintType)
struct FFoodTableData : public FBaseTableData
{
    GENERATED_BODY()

    FFoodTableData()
    : Temperature(0)
    {}

    virtual bool IsEqual(FBaseTableData* Other) override
    {
        auto OtherData = (FFoodTableData*)(Other);
        if (OtherData)
        {
            if (OtherData == this)
            {
                return true;
            }
        }

        return false;
    }

    UPROPERTY(EditAnywhere)
    int Temperature;
};


/**
 * 
 */
class BIGBUTLERBATTLE_API DataTables
{

};


