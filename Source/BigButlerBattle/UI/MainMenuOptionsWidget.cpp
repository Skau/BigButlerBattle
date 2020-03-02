// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuOptionsWidget.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Button.h"
#include "MainMenuWidget.h"
#include "SoundSettingsWidget.h"

UMainMenuOptionsWidget::UMainMenuOptionsWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{}

bool UMainMenuOptionsWidget::Initialize()
{
	bool bInit = Super::Initialize();

	Button_Sound->OnClicked.AddDynamic(this, &UMainMenuOptionsWidget::OnSoundPressed);
	Buttons.Add(Button_Sound);

	Button_Back->OnClicked.AddDynamic(this, &UMainMenuOptionsWidget::OnBackButtonPressed);
	Buttons.Add(Button_Back);

	DefaultWidgetToFocus = Button_Sound;

	return bInit;
}

void UMainMenuOptionsWidget::OnBackButtonPressed()
{
	switch (CurrentIndex)
	{
	case EWidgetSwitcherIndex::Main:
		SetVisibility(ESlateVisibility::Hidden);
		MainMenuWidget->SetVisibility(ESlateVisibility::Visible);
		MainMenuWidget->FocusWidget(OwningPlayerController);
		break;
	case EWidgetSwitcherIndex::Sound:
		SetCurrentWidgetSwitcherIndex(EWidgetSwitcherIndex::Main);
		break;
	}
}

void UMainMenuOptionsWidget::SetCurrentWidgetSwitcherIndex(EWidgetSwitcherIndex NewIndex)
{
	CurrentIndex = NewIndex;
	Switcher->SetActiveWidgetIndex(static_cast<int>(CurrentIndex));

	switch (CurrentIndex)
	{
	case EWidgetSwitcherIndex::Main:
		FocusWidget(OwningPlayerController, Button_Sound);
		break;
	case EWidgetSwitcherIndex::Sound:
		if (!SoundSettings->Button_Back->OnClicked.IsBound())
			SoundSettings->Button_Back->OnClicked.AddDynamic(this, &UMainMenuOptionsWidget::OnBackButtonPressed);
		SoundSettings->FocusWidget(OwningPlayerController);
		break;
	}
}

void UMainMenuOptionsWidget::OnSoundPressed()
{
	SetCurrentWidgetSwitcherIndex(EWidgetSwitcherIndex::Sound);
}