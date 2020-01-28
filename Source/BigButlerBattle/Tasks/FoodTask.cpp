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
    auto OtherData = (UFoodTask*)(Other);
    if (OtherData)
    {
        if (OtherData->Name != Name)
            return false;

        if (OtherData->Temperature != Temperature)
            return false;
    }
    else
    {
        return false;
    }

    return true;
}
