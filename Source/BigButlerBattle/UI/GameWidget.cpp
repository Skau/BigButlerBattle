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

void UGameWidget::OnPlayerInteractMainItem(int ControllerID, bool bPickedUp)
{
	UpdateMessage("Player " + FString::FromInt(ControllerID + 1) + (bPickedUp ? " picked up" : " dropped") + " the main item!");
}

void UGameWidget::OnMainItemSet()
{
	UpdateMessage("The King demands a new item!");
}

void UGameWidget::UpdateMessage(const FString& Message, const float Duration)
{
	if (Message.IsEmpty() || !Text_Info)
		return;

	Text_Info->SetText(FText::FromString(Message));
	Text_Info->SetVisibility(ESlateVisibility::Visible);

	FTimerDelegate TimerCallback;
	TimerCallback.BindLambda([&]
	{
		Text_Info->SetVisibility(ESlateVisibility::Hidden);
	});

	GetWorld()->GetTimerManager().SetTimer(HandleInfoMessage, TimerCallback, Duration, false);
}