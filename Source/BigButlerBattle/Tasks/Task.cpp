// Fill out your copyright notice in the Description page of Project Settings.


#include "Task.h"

UTask::UTask()
	: Type(EObjectType::None)
	, Mesh(nullptr)
	, Material(nullptr)
{

}

bool UTask::InitTaskData(uint8* Data)
{
	const auto TaskData = reinterpret_cast<FTaskTableData*>(Data);
    if (TaskData)
    {
        Type = TaskData->Type;
        Mesh = TaskData->Mesh;
        Material = TaskData->Material;
        return true;
    }
    return false;
}

bool UTask::IsEqual(const UTask* Other) const
{
    return (Other && Other->Name == Name);
}
