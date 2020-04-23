// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerScoreWidget.h"
#include "Components/TextBlock.h"

bool UPlayerScoreWidget::Initialize()
{
	bool bInit = Super::Initialize();

	return bInit;
}

void UPlayerScoreWidget::SetPlayerName(const FString& Name)
{
	if (PlayerName)
	{
		PlayerName->SetText(FText::FromString(Name));
	}
}

void UPlayerScoreWidget::UpdateScore(int NewScore)
{
	if (PlayerScore)
	{
		PlayerScore->SetText(FText::FromString(FString::FromInt(NewScore)));
	}
}