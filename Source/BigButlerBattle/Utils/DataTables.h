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

    UPROPERTY(EditAnywhere)
    int Temperature;
};


USTRUCT(BlueprintType)
struct FBezierPoint : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Position;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector InTangent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector OutTangent;
};


