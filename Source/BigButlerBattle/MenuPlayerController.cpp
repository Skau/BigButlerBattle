// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuPlayerController.h"
#include "Components/MenuAnchor.h"
#include "MainMenuPlayerWidget.h"
#include "Components/CheckBox.h"
#include "Components/WidgetSwitcher.h"

AMenuPlayerController::AMenuPlayerController()
{
}


void AMenuPlayerController::SetPlayerWidgets(UWidgetSwitcher* WidgetSwitcherIn, UMainMenuPlayerWidget* PlayerWidgetIn)
{
	WidgetSwitcher = WidgetSwitcherIn;
	PlayerWidget = PlayerWidgetIn;
}

void AMenuPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	check(InputComponent != nullptr);

	InputComponent->BindAction("Activate", EInputEvent::IE_Pressed, this, &AMenuPlayerController::Activate);
	InputComponent->BindAction("Deactivate", EInputEvent::IE_Pressed, this, &AMenuPlayerController::Deactivate);
}

void AMenuPlayerController::Activate()
{
	if (WidgetSwitcher && PlayerWidget)
	{
		if (WidgetSwitcher->ActiveWidgetIndex == 0)
		{
			WidgetSwitcher->SetActiveWidgetIndex(1);
			OnToggleJoinedGame.ExecuteIfBound(true, ID);
		}
		else
		{
			if (PlayerWidget->GetPlayerReadyState() == ECheckBoxState::Checked)
			{
				PlayerWidget->SetPlayerReadyState(ECheckBoxState::Unchecked);
				OnToggleReadyGame.ExecuteIfBound(false, ID);
			}
			else
			{
				PlayerWidget->SetPlayerReadyState(ECheckBoxState::Checked);
				OnToggleReadyGame.ExecuteIfBound(true, ID);
			}
		}
	}
}

void AMenuPlayerController::Deactivate()
{
	if (WidgetSwitcher && PlayerWidget)
	{
		if (WidgetSwitcher->ActiveWidgetIndex == 1)
		{
			if (PlayerWidget->GetPlayerReadyState() == ECheckBoxState::Checked)
			{
				PlayerWidget->SetPlayerReadyState(ECheckBoxState::Unchecked);
				OnToggleReadyGame.ExecuteIfBound(false, ID);
			}
			else
			{
				WidgetSwitcher->SetActiveWidgetIndex(0);
				OnToggleJoinedGame.ExecuteIfBound(false, ID);
			}
		}
	}
}
