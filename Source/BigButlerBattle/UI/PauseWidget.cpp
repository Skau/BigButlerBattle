// Fill out your copyright notice in the Description page of Project Settings.


#include "PauseWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "SoundSettingsWidget.h"
#include "Kismet/GameplayStatics.h"

bool UPauseWidget::Initialize()
{
	bool bInit = Super::Initialize();

	SetVisibility(ESlateVisibility::Hidden);

	Button_Continue->OnClicked.AddDynamic(this, &UPauseWidget::OnContinuePressed);
	Buttons.Add(Button_Continue);

	Button_Options->OnClicked.AddDynamic(this, &UPauseWidget::OnOptionsPressed);
	Buttons.Add(Button_Options);

	Button_Quit->OnClicked.AddDynamic(this, &UPauseWidget::OnQuitPressed);
	Buttons.Add(Button_Quit);

	Button_Sound->OnClicked.AddDynamic(this, &UPauseWidget::OnSoundPressed);
	Buttons.Add(Button_Sound);

	Button_Camera->OnClicked.AddDynamic(this, &UPauseWidget::OnCameraPressed);
	Buttons.Add(Button_Camera);

	Button_Back->OnClicked.AddDynamic(this, &UPauseWidget::OnBackButtonPressed);
	Buttons.Add(Button_Back);

	DefaultWidgetToFocus = Button_Continue;

	return bInit;
}

void UPauseWidget::OnPlayerControllerSet()
{
	const auto ControllerID = UGameplayStatics::GetPlayerControllerID(OwningPlayerController);
	PlayerText->SetText(FText::FromString("By Player " + FString::FromInt(ControllerID + 1)));
}

void UPauseWidget::OnBackButtonPressed()
{
	switch (CurrentIndex)
	{
	case EWidgetSwitcherIndex::Main:
		OnContinuePressed();
		break;
	case EWidgetSwitcherIndex::Options:
		SetCurrentWidgetSwitcherIndex(EWidgetSwitcherIndex::Main);
		break;
	case EWidgetSwitcherIndex::Sound:
		SetCurrentWidgetSwitcherIndex(EWidgetSwitcherIndex::Options);
		break;
	case EWidgetSwitcherIndex::Camera:
		SetCurrentWidgetSwitcherIndex(EWidgetSwitcherIndex::Options);
		break;
	}
}

void UPauseWidget::SetCurrentWidgetSwitcherIndex(EWidgetSwitcherIndex NewIndex)
{
	CurrentIndex = NewIndex;
	Switcher->SetActiveWidgetIndex(static_cast<int>(CurrentIndex));

	switch (CurrentIndex)
	{
	case EWidgetSwitcherIndex::Main:
		FocusWidget(OwningPlayerController);
		break;
	case EWidgetSwitcherIndex::Options:
		FocusWidget(OwningPlayerController, Button_Sound);
		break;
	case EWidgetSwitcherIndex::Sound:
		if(!SoundSettings->Button_Back->OnClicked.IsBound())
			SoundSettings->Button_Back->OnClicked.AddDynamic(this, &UPauseWidget::OnBackButtonPressed);
		SoundSettings->FocusWidget(OwningPlayerController);
		break;
	case EWidgetSwitcherIndex::Camera:
		break;
	}
}

void UPauseWidget::OnContinuePressed()
{
	const auto ControllerID = UGameplayStatics::GetPlayerControllerID(OwningPlayerController);
	ContinueGame.ExecuteIfBound(ControllerID);
}

void UPauseWidget::OnOptionsPressed()
{
	SetCurrentWidgetSwitcherIndex(EWidgetSwitcherIndex::Options);
}

void UPauseWidget::OnSoundPressed()
{
	SetCurrentWidgetSwitcherIndex(EWidgetSwitcherIndex::Sound);
}

void UPauseWidget::OnCameraPressed()
{
	//SetCurrentWidgetSwitcherIndex(EWidgetSwitcherIndex::Camera);
}

void UPauseWidget::OnQuitPressed()
{
	QuitGame.ExecuteIfBound();
}
