// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerWidget.h"
#include "TaskWidget.h"

bool UPlayerWidget::Initialize()
{
	if (!Super::Initialize())
		return false;

	TaskWidgets.Add(TaskSlot0);
	TaskWidgets.Add(TaskSlot1);
	TaskWidgets.Add(TaskSlot2);
	TaskWidgets.Add(TaskSlot3);
	TaskWidgets.Add(TaskSlot4);
	TaskWidgets.Add(TaskSlot5);

	return true;
}

void UPlayerWidget::UpdateTaskSlotName(FString name, int index)
{
	if (index >= 0 && index < TaskWidgets.Num())
	{
		TaskWidgets[index]->SetTaskName(name);
	}
}