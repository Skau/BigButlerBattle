// Fill out your copyright notice in the Description page of Project Settings.


#include "SoundSettingsWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "ButlerGameInstance.h"

USoundSettingsWidget::USoundSettingsWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{}

bool USoundSettingsWidget::Initialize()
{
	bool bInit = Super::Initialize();

	Button_MasterDown->OnClicked.AddDynamic(this, &USoundSettingsWidget::OnMasterDownPressed);
	Buttons.Add(Button_MasterDown);

	Button_MasterUp->OnClicked.AddDynamic(this, &USoundSettingsWidget::OnMasterUpPressed);
	Buttons.Add(Button_MasterUp);

	Button_BackgroundDown->OnClicked.AddDynamic(this, &USoundSettingsWidget::OnBackgroundDownPressed);
	Buttons.Add(Button_BackgroundDown);

	Button_BackgroundUp->OnClicked.AddDynamic(this, &USoundSettingsWidget::OnBackgroundUpPressed);
	Buttons.Add(Button_BackgroundUp);

	Button_SFXDown->OnClicked.AddDynamic(this, &USoundSettingsWidget::OnSFXDownPressed);
	Buttons.Add(Button_SFXDown);

	Button_SFXUp->OnClicked.AddDynamic(this, &USoundSettingsWidget::OnSFXUpPressed);
	Buttons.Add(Button_SFXUp);

	Buttons.Add(Button_Back);

	DefaultWidgetToFocus = Button_Back;

	if (auto Instance = GetGameInstance())
	{
		auto NewVolume = FString::FromInt(Instance->GetMainSoundVolume() * 100.f) + "%";
		TextBlock_MasterVolume->SetText(FText::FromString(NewVolume));

		NewVolume = FString::FromInt(Instance->GetBackgroundSoundVolume() * 100.f) + "%";
		TextBlock_BackgroundVolume->SetText(FText::FromString(NewVolume));

		NewVolume = FString::FromInt(Instance->GetSoundEffectsSoundVolume() * 100.f) + "%";
		TextBlock_SFXVolume->SetText(FText::FromString(NewVolume));
	}
	else
		UE_LOG(LogTemp, Error, TEXT("USoundSettingsWidget::Initialize: Could not find game instance!"))

	return bInit;
}

void USoundSettingsWidget::OnBackButtonPressed()
{
	Button_Back->OnClicked.Broadcast();
}

void USoundSettingsWidget::OnMasterDownPressed()
{
	if (auto Instance = GetGameInstance())
	{
		auto NewVolume = FString::FromInt(Instance->UpdateMainSoundVolume(false) * 100.f) + "%";
		TextBlock_MasterVolume->SetText(FText::FromString(NewVolume));
	}
	else
		UE_LOG(LogTemp, Error, TEXT("USoundSettingsWidget::OnMasterDownPressed: Could not find game instance!"))
}

void USoundSettingsWidget::OnMasterUpPressed()
{
	if (auto Instance = GetGameInstance())
	{
		auto NewVolume = FString::FromInt(Instance->UpdateMainSoundVolume(true) * 100.f) + "%";
		TextBlock_MasterVolume->SetText(FText::FromString(NewVolume));
	}
	else
		UE_LOG(LogTemp, Error, TEXT("USoundSettingsWidget::OnMasterUpPressed: Could not find game instance!"))
}

void USoundSettingsWidget::OnBackgroundDownPressed()
{
	if (auto Instance = GetGameInstance())
	{
		auto NewVolume = FString::FromInt(Instance->UpdateBackgroundSoundVolume(false) * 100.f) + "%";
		TextBlock_BackgroundVolume->SetText(FText::FromString(NewVolume));
	}
	else
		UE_LOG(LogTemp, Error, TEXT("USoundSettingsWidget::OnBackgroundDownPressed: Could not find game instance!"))
}

void USoundSettingsWidget::OnBackgroundUpPressed()
{
	if (auto Instance = GetGameInstance())
	{
		auto NewVolume = FString::FromInt(Instance->UpdateBackgroundSoundVolume(true) * 100.f) + "%";
		TextBlock_BackgroundVolume->SetText(FText::FromString(NewVolume));
	}
	else
		UE_LOG(LogTemp, Error, TEXT("USoundSettingsWidget::OnBackgroundUpPressed: Could not find game instance!"))
}

void USoundSettingsWidget::OnSFXDownPressed()
{
	if (auto Instance = GetGameInstance())
	{
		auto NewVolume = FString::FromInt(Instance->UpdateSoundEffectsSoundVolume(false) * 100.f) + "%";
		TextBlock_SFXVolume->SetText(FText::FromString(NewVolume));
	}
	else
		UE_LOG(LogTemp, Error, TEXT("USoundSettingsWidget::OnSFXDownPressed: Could not find game instance!"))
}

void USoundSettingsWidget::OnSFXUpPressed()
{
	if (auto Instance = GetGameInstance())
	{
		auto NewVolume = FString::FromInt(Instance->UpdateSoundEffectsSoundVolume(true) * 100.f) + "%";
		TextBlock_SFXVolume->SetText(FText::FromString(NewVolume));
	}
	else
		UE_LOG(LogTemp, Error, TEXT("USoundSettingsWidget::OnSFXUpPressed: Could not find game instance!"))
}

UButlerGameInstance* USoundSettingsWidget::GetGameInstance()
{
	return Cast<UButlerGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
}