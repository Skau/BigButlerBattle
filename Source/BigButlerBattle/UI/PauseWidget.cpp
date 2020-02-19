// Fill out your copyright notice in the Description page of Project Settings.


#include "PauseWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

bool UPauseWidget::Initialize()
{
	bool bInit = Super::Initialize();

	SetVisibility(ESlateVisibility::Hidden);

	Button_Continue->OnClicked.AddDynamic(this, &UPauseWidget::OnContinuePressed);
	Button_Quit->OnClicked.AddDynamic(this, &UPauseWidget::OnQuitPressed);

	Buttons.Add(Button_Continue);
	Buttons.Add(Button_Quit);

	DefaultWidgetToFocus = Button_Continue;

	return bInit;
}

void UPauseWidget::OnPlayerControllerSet()
{
	const auto ControllerID = UGameplayStatics::GetPlayerControllerID(OwningPlayerController);
	PlayerText->SetText(FText::FromString("By Player " + FString::FromInt(ControllerID + 1)));
}

void UPauseWidget::OnContinuePressed()
{
	const auto ControllerID = UGameplayStatics::GetPlayerControllerID(OwningPlayerController);
	ContinueGame.ExecuteIfBound(ControllerID);
}

void UPauseWidget::OnQuitPressed()
{
	QuitGame.ExecuteIfBound();
}
