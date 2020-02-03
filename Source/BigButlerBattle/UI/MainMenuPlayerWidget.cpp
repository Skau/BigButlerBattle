// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuPlayerWidget.h"
#include "MainMenuPlayWidget.h"
#include "Components/TextBlock.h"
#include "Components/CheckBox.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Button.h"
#include "Player/PlayerCharacterController.h"
#include "Kismet/GameplayStatics.h"
#include "MainMenuGameModeBase.h"

UMainMenuPlayerWidget::UMainMenuPlayerWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{}

bool UMainMenuPlayerWidget::Initialize()
{
	bool bInit =  Super::Initialize();

	Button_Join->OnClicked.AddDynamic(this, &UMainMenuPlayerWidget::OnJoinClicked);
	Buttons.Add(Button_Join);

	Button_ToggleReady->OnClicked.AddDynamic(this, &UMainMenuPlayerWidget::OnToggledReady);
	Buttons.Add(Button_ToggleReady);

	DefaultWidgetToFocus = Button_Join;

	return bInit;
}

void UMainMenuPlayerWidget::OnPlayerCharacterControllerSet()
{
	UpdatePlayerName();
}

void UMainMenuPlayerWidget::OnBackButtonPressed()
{
	if (bIsReady)
	{
		OnToggledReady();
	}
	else if (bHasJoined)
	{
		if (bIsReady)
			OnToggledReady();

		bHasJoined = false;

		Switcher->SetActiveWidgetIndex(0);
		auto ID = UGameplayStatics::GetPlayerControllerID(OwningCharacterController);
		OnToggleJoinedGame.ExecuteIfBound(bHasJoined, ID);
		FocusWidget(OwningCharacterController, Button_Join);
	}
	else
	{
		auto GameMode = Cast<AMainMenuGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
		if (GameMode && !GameMode->HasAnyPlayerJoined())
			MainPlayWidget->BackToMainMenu();
	}
}

void UMainMenuPlayerWidget::OnJoinClicked()
{
	Switcher->SetActiveWidgetIndex(1);
	auto ID = UGameplayStatics::GetPlayerControllerID(OwningCharacterController);
	OnToggleJoinedGame.ExecuteIfBound(true, ID);
	FocusWidget(OwningCharacterController, Button_ToggleReady);
	bHasJoined = true;
}

void UMainMenuPlayerWidget::OnToggledReady()
{
	auto ID = UGameplayStatics::GetPlayerControllerID(OwningCharacterController);
	bIsReady = !bIsReady;
	UpdateToggledReady();
}

void UMainMenuPlayerWidget::UpdateToggledReady()
{
	auto ID = UGameplayStatics::GetPlayerControllerID(OwningCharacterController);
	OnToggleReadyGame.ExecuteIfBound(bIsReady, ID);
	ButtonReadyText->SetText(FText::FromString(bIsReady ? "Ready" : "Not ready"));
}

void UMainMenuPlayerWidget::UpdatePlayerName()
{
	auto ID = UGameplayStatics::GetPlayerControllerID(OwningCharacterController);
	PlayerNameText->SetText(FText::FromString("Player " + FString::FromInt(ID + 1)));
}