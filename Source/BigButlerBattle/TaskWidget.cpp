// Fill out your copyright notice in the Description page of Project Settings.


#include "TaskWidget.h"
#include "Components/TextBlock.h"

void UTaskWidget::SetTaskName(FString string)
{
	TaskName->SetText(FText::FromString(string));
}
