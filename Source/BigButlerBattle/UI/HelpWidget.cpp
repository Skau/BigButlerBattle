// Fill out your copyright notice in the Description page of Project Settings.


#include "HelpWidget.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Button.h"
#include "MainMenuWidget.h"

bool UHelpWidget::Initialize()
{
	bool bInit = Super::Initialize();

	Button_Continue->OnClicked.AddDynamic(this, &UHelpWidget::OnContinueButtonPressed);

	DefaultWidgetToFocus = Button_Continue;

	return bInit;
}

void UHelpWidget::OnBackButtonPressed()
{
	if (Switcher)
	{
		if (--CurrentIndex == -1)
		{
			BackToMenu();
		}
		Switcher->SetActiveWidgetIndex(CurrentIndex);
	}
}

void UHelpWidget::OnContinueButtonPressed()
{
	if (Switcher)
	{
		if (++CurrentIndex == 2)
		{
			BackToMenu();
		}
		Switcher->SetActiveWidgetIndex(CurrentIndex);
	}
}

void UHelpWidget::BackToMenu()
{
	CurrentIndex = 0;
	SetVisibility(ESlateVisibility::Hidden);
	if (MainMenuWidget)
	{
		MainMenuWidget->SetVisibility(ESlateVisibility::Visible);
		MainMenuWidget->FocusWidget(OwningPlayerController);
	}
}