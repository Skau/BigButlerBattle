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
#include "ButlerGameInstance.h"

UMainMenuPlayerWidget::UMainMenuPlayerWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{}

bool UMainMenuPlayerWidget::Initialize()
{
	bool bInit =  Super::Initialize();

	Button_Join->OnClicked.AddDynamic(this, &UMainMenuPlayerWidget::OnJoinPressed);
	Buttons.Add(Button_Join);

	Button_Ready->OnClicked.AddDynamic(this, &UMainMenuPlayerWidget::OnReadyPressed);
	Buttons.Add(Button_Ready);

	Button_CameraOptions->OnClicked.AddDynamic(this, &UMainMenuPlayerWidget::OnCameraOptionsPressed);
	Buttons.Add(Button_CameraOptions);

	Button_CameraToggleInvertYaw->OnClicked.AddDynamic(this, &UMainMenuPlayerWidget::OnCameraToggleInvertYawPressed);
	Buttons.Add(Button_CameraToggleInvertYaw);

	Button_CameraToggleInvertPitch->OnClicked.AddDynamic(this, &UMainMenuPlayerWidget::OnCameraToggleInvertPitchPressed);
	Buttons.Add(Button_CameraToggleInvertPitch);

	Button_Back->OnClicked.AddDynamic(this, &UMainMenuPlayerWidget::OnBackPressed);
	Buttons.Add(Button_Back);

	DefaultWidgetToFocus = Button_Join;

	return bInit;
}

void UMainMenuPlayerWidget::OnPlayerCharacterControllerSet()
{
	GameInstance = Cast<UButlerGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	check(GameInstance != nullptr);

	ID = UGameplayStatics::GetPlayerControllerID(OwningCharacterController);

	auto Options = GameInstance->PlayerOptions[ID];
	Text_InvertYaw->SetText(FText::FromString((Options.InvertCameraYaw) ? "Inverted" : "Regular"));
	Text_InvertPitch->SetText(FText::FromString((Options.InvertCameraPitch) ? "Inverted" : "Regular"));

	PlayerNameText->SetText(FText::FromString("Player " + FString::FromInt(ID + 1)));
}

void UMainMenuPlayerWidget::SetCurrentWidgetSwitcherIndex(EWidgetSwitcherIndex NewIndex)
{
	CurrentIndex = NewIndex;
	Switcher->SetActiveWidgetIndex((int)CurrentIndex);

	switch (CurrentIndex)
	{
	case EWidgetSwitcherIndex::Join:
		FocusWidget(OwningCharacterController, Button_Join);
		break;
	case EWidgetSwitcherIndex::Main:
		FocusWidget(OwningCharacterController, Button_Ready);
		break;
	case EWidgetSwitcherIndex::CameraOptions:
		FocusWidget(OwningCharacterController, Button_Back);
		break;
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

void UMainMenuPlayerWidget::UpdateJoinedStatus(bool bHasJoined)
{
	auto ID = UGameplayStatics::GetPlayerControllerID(OwningCharacterController);
	OnToggleJoinedGame.ExecuteIfBound(bHasJoined, ID);
}

void UMainMenuPlayerWidget::OnReadyPressed()
{
	if (CurrentIndex == EWidgetSwitcherIndex::Ready)
		return;

	UpdateReadyStatus(true);
	SetCurrentWidgetSwitcherIndex(EWidgetSwitcherIndex::Ready);
}

void UMainMenuPlayerWidget::UpdateReadyStatus(bool bIsReady)
{
	auto ID = UGameplayStatics::GetPlayerControllerID(OwningCharacterController);
	OnToggleReadyGame.ExecuteIfBound(bIsReady, ID);
}

void UMainMenuPlayerWidget::OnCameraOptionsPressed()
{
	SetCurrentWidgetSwitcherIndex(EWidgetSwitcherIndex::CameraOptions);
}

void UMainMenuPlayerWidget::OnCameraToggleInvertYawPressed()
{
	auto& Options = GameInstance->PlayerOptions[ID];
	Options.InvertCameraYaw = !Options.InvertCameraYaw;
	Text_InvertYaw->SetText(FText::FromString((Options.InvertCameraYaw) ? "Inverted" : "Regular"));
}

void UMainMenuPlayerWidget::OnCameraToggleInvertPitchPressed()
{
	auto& Options = GameInstance->PlayerOptions[ID];
	Options.InvertCameraPitch = !Options.InvertCameraPitch;
	Text_InvertPitch->SetText(FText::FromString((Options.InvertCameraPitch) ? "Inverted" : "Regular"));
}

void UMainMenuPlayerWidget::OnBackPressed()
{
	SetCurrentWidgetSwitcherIndex(EWidgetSwitcherIndex::Main);
}
