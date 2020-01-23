// Fill out your copyright notice in the Description page of Project Settings.


#include "FoodTask.h"

bool UFoodTask::InitTaskData(uint8* Data)
{
    auto FoodData = (FFoodTableData*)(Data);
    if (FoodData)
    {
        Type = FoodData->Type;
        Mesh = FoodData->Mesh;
        Material = FoodData->Material;
        Temperature = FoodData->Temperature;
        return true;
    }

    return false;
}

bool UFoodTask::IsEqual(const UBaseTask* Other) const
{
    UE_LOG(LogTemp, Warning, TEXT("Food IsEqual Called"));

    auto OtherData = (UFoodTask*)(Other);
    if (OtherData)
    {
        if (OtherData == this)
        {
            return true;
        }
    }

    return false;
}
