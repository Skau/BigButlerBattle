// Fill out your copyright notice in the Description page of Project Settings.


#include "GameWidget.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"

bool UGameWidget::Initialize()
{
	bool bInit = Super::Initialize();

	if (Text_Info)
	{
		Text_Info->SetVisibility(ESlateVisibility::Hidden);
	}

	return bInit;
}

void UGameWidget::UpdateTimer(const FString& String)
{
	if (Text_Timer)
	{
		Text_Timer->SetText(FText::FromString(String));
	}
}

void UGameWidget::UpdateInfo(int ControllerID, bool bPickedUp)
{
	if (Text_Info)
	{
		FString Name = "Player " + FString::FromInt(ControllerID + 1);
		FString State = bPickedUp ? " picked up" : " dropped";
		FString Message = Name + State + " the main item!";
		Text_Info->SetText(FText::FromString(Message));
		Text_Info->SetVisibility(ESlateVisibility::Visible);

		FTimerDelegate TimerCallback;
		TimerCallback.BindLambda([&]
		{
			Text_Info->SetVisibility(ESlateVisibility::Hidden);
		});

		GetWorld()->GetTimerManager().SetTimer(HandleInfoMessage, TimerCallback, MessageDuration, false);
	}
}