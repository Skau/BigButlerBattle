// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerScoreWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

bool UPlayerScoreWidget::Initialize()
{
	const bool bInit = Super::Initialize();

	return bInit;
}

void UPlayerScoreWidget::SetPlayerName(const FString& Name, UTexture2D* Icon) const
{
	PlayerName->SetText(FText::FromString(Name));
	PlayerIcon->SetBrushFromTexture(Icon);
	PlayerIcon->SetBrushSize({ 64.f, 64.f });
}

void UPlayerScoreWidget::UpdateScore(const int NewScore) const
{
	if (PlayerScore)
	{
		PlayerScore->SetText(FText::FromString(FString::FromInt(NewScore)));
	}
}
