// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseTask.h"

UBaseTask::UBaseTask()
	: Type(EObjectType::None)
	, Mesh(nullptr)
	, Material(nullptr)
{

}

bool UBaseTask::InitTaskData(uint8* Data)
{
	return false;
}
