// Fill out your copyright notice in the Description page of Project Settings.


#include "GameWidget.h"
#include "Components/TextBlock.h"

bool UGameWidget::Initialize()
{
	bool bInit = Super::Initialize();



	return bInit;
}

void UGameWidget::UpdateTimer(const FString& String)
{
	if (Text_Timer)
	{
		Text_Timer->SetText(FText::FromString(String));
	}
}