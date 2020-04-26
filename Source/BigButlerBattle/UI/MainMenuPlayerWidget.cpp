// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuPlayerWidget.h"
#include "MainMenuPlayWidget.h"
#include "CameraSettingsWidget.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "MainMenuGameModeBase.h"
#include "ButlerGameInstance.h"
#include "Player/PlayerCharacter.h"

UMainMenuPlayerWidget::UMainMenuPlayerWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{}

bool UMainMenuPlayerWidget::Initialize()
{
	const bool bInit =  Super::Initialize();

	Button_Join->OnClicked.AddDynamic(this, &UMainMenuPlayerWidget::OnJoinPressed);
	Buttons.Add(Button_Join);

	Button_Ready->OnClicked.AddDynamic(this, &UMainMenuPlayerWidget::OnReadyPressed);
	Buttons.Add(Button_Ready);

	Button_CameraOptions->OnClicked.AddDynamic(this, &UMainMenuPlayerWidget::OnCameraOptionsPressed);
	Buttons.Add(Button_CameraOptions);

	DefaultWidgetToFocus = Button_Join;

	return bInit;
}

void UMainMenuPlayerWidget::OnPlayerControllerSet()
{
	ID = UGameplayStatics::GetPlayerControllerID(OwningPlayerController);
	PlayerNameText->SetText(FText::FromString("Player " + FString::FromInt(ID + 1)));
}

void UMainMenuPlayerWidget::SetCurrentWidgetSwitcherIndex(EWidgetSwitcherIndex NewIndex)
{
	CurrentIndex = NewIndex;
	Switcher->SetActiveWidgetIndex(static_cast<int>(CurrentIndex));

	switch (CurrentIndex)
	{
	case EWidgetSwitcherIndex::Join:
		FocusWidget(OwningPlayerController, Button_Join);
		HideCharacter();
		break;
	case EWidgetSwitcherIndex::Main:
		FocusWidget(OwningPlayerController, Button_Ready);
		ShowCharacter();
		break;
	case EWidgetSwitcherIndex::CameraOptions:
		if(!CameraSettings->Button_Back->OnClicked.IsBound())
			CameraSettings->Button_Back->OnClicked.AddDynamic(this, &UMainMenuPlayerWidget::OnBackButtonPressed);
		CameraSettings->FocusWidget(OwningPlayerController);
		break;
	case EWidgetSwitcherIndex::Ready: break;
	default: ;
	}
}

void UMainMenuPlayerWidget::OnBackButtonPressed()
{
	switch (CurrentIndex)
	{
	case EWidgetSwitcherIndex::Join:
	{
		auto GameMode = Cast<AMainMenuGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
		if (GameMode && !GameMode->HasAnyPlayerJoined())
			MainPlayWidget->BackToMainMenu();
	}
		break;
	case EWidgetSwitcherIndex::Ready:
		UpdateReadyStatus(false);
		SetCurrentWidgetSwitcherIndex(EWidgetSwitcherIndex::Main);
		break;
	case EWidgetSwitcherIndex::Main:
		UpdateJoinedStatus(false);
		SetCurrentWidgetSwitcherIndex(EWidgetSwitcherIndex::Join);
		break;
	case EWidgetSwitcherIndex::CameraOptions:
		SetCurrentWidgetSwitcherIndex(EWidgetSwitcherIndex::Main);
		break;
	}
}

void UMainMenuPlayerWidget::OnJoinPressed()
{
	UpdateJoinedStatus(true);
	SetCurrentWidgetSwitcherIndex(EWidgetSwitcherIndex::Main);
}

void UMainMenuPlayerWidget::UpdateJoinedStatus(bool bHasJoined) const
{
	const auto ControllerID = UGameplayStatics::GetPlayerControllerID(OwningPlayerController);
	OnToggleJoinedGame.ExecuteIfBound(bHasJoined, ControllerID);
}

void UMainMenuPlayerWidget::OnReadyPressed()
{
	if (CurrentIndex == EWidgetSwitcherIndex::Ready)
		return;

	UpdateReadyStatus(true);
	SetCurrentWidgetSwitcherIndex(EWidgetSwitcherIndex::Ready);
}

void UMainMenuPlayerWidget::UpdateReadyStatus(const bool bIsReady) const
{
	const auto ControllerID = UGameplayStatics::GetPlayerControllerID(OwningPlayerController);
	OnToggleReadyGame.ExecuteIfBound(bIsReady, ControllerID);
}

void UMainMenuPlayerWidget::OnCameraOptionsPressed()
{
	SetCurrentWidgetSwitcherIndex(EWidgetSwitcherIndex::CameraOptions);
}

void UMainMenuPlayerWidget::SpawnCharacter(FTransform Transform)
{
	CharacterInstance = GetWorld()->SpawnActor<APlayerCharacter>(CharacterClass, Transform);
	HideCharacter();
}

void UMainMenuPlayerWidget::ShowCharacter()
{
	if (CharacterInstance)
	{
		CharacterInstance->SetActorHiddenInGame(false);
	}
}

void UMainMenuPlayerWidget::HideCharacter()
{
	if (CharacterInstance)
	{
		CharacterInstance->SetActorHiddenInGame(true);
	}
}
