// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFinishedWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"

bool UGameFinishedWidget::Initialize()
{
	const bool bInit = Super::Initialize();

	SetVisibility(ESlateVisibility::Hidden);

	Button_Quit->OnClicked.AddDynamic(this, &UGameFinishedWidget::OnQuitPressed);

	Buttons.Add(Button_Quit);

	DefaultWidgetToFocus = Button_Quit;

	return bInit;
}

void UGameFinishedWidget::SetWonText(int ControllerID, const FString& Text) const
{
	WonText->SetText(FText::FromString(Text));
	PlayerIcon->SetBrushFromTexture(PlayerIcons[ControllerID]);
	PlayerIcon->SetBrushSize({ 64.f, 64.f });
}

void UGameFinishedWidget::OnQuitPressed()
{
	UE_LOG(LogTemp, Warning, TEXT("GameFinishedWidget: Quit pressed"));
	QuitGame.ExecuteIfBound();
}