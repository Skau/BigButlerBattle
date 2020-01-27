// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerWidget.h"
#include "TaskWidget.h"
#include "Components/TextBlock.h"
#include "BigButlerBattleGameModeBase.h"

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

void UPlayerWidget::UpdateTaskSlotName(int index, FString name)
{
	if (index >= 0 && index < TaskWidgets.Num())
	{
		TaskWidgets[index]->TaskBlock->SetText(FText::FromString(name));
	}
}

void UPlayerWidget::UpdateTaskState(int index, ETaskState State)
{
	if (index >= 0 && index < TaskWidgets.Num())
	{
		switch (State)
		{
		case ETaskState::NotPresent:
		{
			TaskWidgets[index]->TaskBlock->SetColorAndOpacity(NotPresent);
		}
		break;
		case ETaskState::Present:
		{
			TaskWidgets[index]->TaskBlock->SetColorAndOpacity(Present);
		}
		break;
		case ETaskState::Finished:
		{
			TaskWidgets[index]->TaskBlock->SetColorAndOpacity(Finished);
		}
		break;
		default:
			break;
		}
	}
}
