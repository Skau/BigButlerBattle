// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraSettingsWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "ButlerGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Player/PlayerCharacterController.h"

bool UCameraSettingsWidget::Initialize()
{
	bool bInit = Super::Initialize();

	Button_CameraToggleInvertYaw->OnClicked.AddDynamic(this, &UCameraSettingsWidget::OnCameraToggleInvertYawPressed);
	Buttons.Add(Button_CameraToggleInvertYaw);

	Button_CameraToggleInvertPitch->OnClicked.AddDynamic(this, &UCameraSettingsWidget::OnCameraToggleInvertPitchPressed);
	Buttons.Add(Button_CameraToggleInvertPitch);

	Buttons.Add(Button_Back);

	DefaultWidgetToFocus = Button_Back;

	return bInit;
}

void UCameraSettingsWidget::OnPlayerControllerSet()
{
	ID = UGameplayStatics::GetPlayerControllerID(OwningPlayerController);

	if (auto GameInstance = GetGameInstance())
	{
		const auto Options = GetGameInstance()->PlayerOptions[ID];
		Text_InvertYaw->SetText(FText::FromString((Options.InvertCameraYaw) ? "Inverted" : "Regular"));
		Text_InvertPitch->SetText(FText::FromString((Options.InvertCameraPitch) ? "Inverted" : "Regular"));
	}
	else
		UE_LOG(LogTemp, Error, TEXT("UCameraSettingsWidget::OnPlayerControllerSet: Could not find game instance!"))
}

void UCameraSettingsWidget::OnBackButtonPressed()
{
	Button_Back->OnClicked.Broadcast();
}

void UCameraSettingsWidget::OnCameraToggleInvertPitchPressed()
{
	if (auto GameInstance = GetGameInstance())
	{
		auto& Options = GameInstance->PlayerOptions[ID];
		Options.InvertCameraPitch = !Options.InvertCameraPitch;
		Text_InvertPitch->SetText(FText::FromString((Options.InvertCameraPitch) ? "Inverted" : "Regular"));

		if (auto Controller = Cast<APlayerCharacterController>(OwningPlayerController))
		{
			Controller->UpdateCameraSettings();
		}
	}
	else
		UE_LOG(LogTemp, Error, TEXT("UCameraSettingsWidget::OnCameraToggleInvertPitchPressed: Could not find game instance!"))
}

void UCameraSettingsWidget::OnCameraToggleInvertYawPressed()
{
	if (auto GameInstance = GetGameInstance())
	{
		auto& Options = GameInstance->PlayerOptions[ID];
		Options.InvertCameraYaw = !Options.InvertCameraYaw;
		Text_InvertYaw->SetText(FText::FromString((Options.InvertCameraYaw) ? "Inverted" : "Regular"));

		if (auto Controller = Cast<APlayerCharacterController>(OwningPlayerController))
		{
			Controller->UpdateCameraSettings();
		}
	}
	else
		UE_LOG(LogTemp, Error, TEXT("UCameraSettingsWidget::OnCameraToggleInvertYawPressed: Could not find game instance!"))
}

UButlerGameInstance* UCameraSettingsWidget::GetGameInstance()
{
	return Cast<UButlerGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
}


