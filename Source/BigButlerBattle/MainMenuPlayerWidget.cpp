// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuPlayerWidget.h"
#include "Components/TextBlock.h"
#include "Components/CheckBox.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Button.h"
#include "PlayerCharacterController.h"
#include "Kismet/GameplayStatics.h"

UMainMenuPlayerWidget::UMainMenuPlayerWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{}

bool UMainMenuPlayerWidget::Initialize()
{
	bool bInitialized =  Super::Initialize();

	CheckBox->OnCheckStateChanged.AddDynamic(this, &UMainMenuPlayerWidget::OnCheckStateChanged);

	Button_Join->OnClicked.AddDynamic(this, &UMainMenuPlayerWidget::OnJoinClicked);
	Buttons.Add(Button_Join);

	Button_Leave->OnClicked.AddDynamic(this, &UMainMenuPlayerWidget::OnLeaveClicked);
	Buttons.Add(Button_Leave);

	DefaultWidgetToFocus = Button_Join;

	return bInitialized;
}

void UMainMenuPlayerWidget::OnPlayerCharacterControllerSet()
{
	UpdatePlayerName();
}

void UMainMenuPlayerWidget::OnJoinClicked()
{
	Switcher->SetActiveWidgetIndex(1);
	auto ID = UGameplayStatics::GetPlayerControllerID(OwningCharacterController);
	OnToggleJoinedGame.ExecuteIfBound(true, ID);
	FocusWidget(OwningCharacterController, CheckBox);
}

void UMainMenuPlayerWidget::OnCheckStateChanged(bool NewCheckState)
{
	auto ID = UGameplayStatics::GetPlayerControllerID(OwningCharacterController);
	if (NewCheckState)
	{
		CheckBox->SetCheckedState(ECheckBoxState::Checked);
		OnToggleReadyGame.ExecuteIfBound(true, ID);
	}
	else
	{
		CheckBox->SetCheckedState(ECheckBoxState::Unchecked);
		OnToggleReadyGame.ExecuteIfBound(false, ID);
	}
}

void UMainMenuPlayerWidget::OnLeaveClicked()
{
	if (CheckBox->GetCheckedState() == ECheckBoxState::Checked)
	{
		OnCheckStateChanged(false);
	}

	auto ID = UGameplayStatics::GetPlayerControllerID(OwningCharacterController);

	Switcher->SetActiveWidgetIndex(0);
	OnToggleJoinedGame.ExecuteIfBound(false, ID);
	FocusWidget(OwningCharacterController, Button_Join);
}

void UMainMenuPlayerWidget::UpdatePlayerName()
{
	auto ID = UGameplayStatics::GetPlayerControllerID(OwningCharacterController);
	PlayerNameText->SetText(FText::FromString("Player " + FString::FromInt(ID + 1)));
}