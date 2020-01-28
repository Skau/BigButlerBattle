// Fill out your copyright notice in the Description page of Project Settings.


#include "WineTask.h"

bool UWineTask::InitTaskData(uint8* Data)
{
    auto WineData = (FWineTableData*)(Data);
    if (WineData)
    {
        Type = WineData->Type;
        Mesh = WineData->Mesh;
        Material = WineData->Material;
        Year = WineData->Year;
        return true;
    }

    return false;
}

bool UWineTask::IsEqual(const UBaseTask* Other) const
{
    auto OtherData = (UWineTask*)(Other);
    if (OtherData)
    {
        if (OtherData->Name != Name)
            return false;

        if (OtherData->Year != Year)
            return false;
    }
    else
    {
        return false;
    }

    return true;
}