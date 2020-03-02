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

	auto Instance = GetGameInstance();
	if (Instance)
	{
		auto NewVolume = FString::FromInt(Instance->GetMainSoundVolume() * 100.f);
		NewVolume.Append("%");
		TextBlock_MasterVolume->SetText(FText::FromString(NewVolume));

		NewVolume = FString::FromInt(Instance->GetBackgroundSoundVolume() * 100.f);
		NewVolume.Append("%");
		TextBlock_BackgroundVolume->SetText(FText::FromString(NewVolume));

		NewVolume = FString::FromInt(Instance->GetSoundEffectsSoundVolume() * 100.f);
		NewVolume.Append("%");
		TextBlock_SFXVolume->SetText(FText::FromString(NewVolume));
	}

	return bInit;
}

void USoundSettingsWidget::OnBackButtonPressed()
{
	Button_Back->OnClicked.Broadcast();
}

void USoundSettingsWidget::OnMasterDownPressed()
{
	auto Instance = GetGameInstance();
	auto NewVolume = FString::FromInt(Instance->UpdateMainSoundVolume(false) * 100.f);
	NewVolume.Append("%");
	TextBlock_MasterVolume->SetText(FText::FromString(NewVolume));
}

void USoundSettingsWidget::OnMasterUpPressed()
{
	auto Instance = GetGameInstance();
	auto NewVolume = FString::FromInt(Instance->UpdateMainSoundVolume(true) * 100.f);
	NewVolume.Append("%");
	TextBlock_MasterVolume->SetText(FText::FromString(NewVolume));
}

void USoundSettingsWidget::OnBackgroundDownPressed()
{
	auto Instance = GetGameInstance();
	auto NewVolume = FString::FromInt(Instance->UpdateBackgroundSoundVolume(false) * 100.f);
	NewVolume.Append("%");
	TextBlock_BackgroundVolume->SetText(FText::FromString(NewVolume));
}

void USoundSettingsWidget::OnBackgroundUpPressed()
{
	auto Instance = GetGameInstance();
	auto NewVolume = FString::FromInt(Instance->UpdateBackgroundSoundVolume(true) * 100.f);
	NewVolume.Append("%");
	TextBlock_BackgroundVolume->SetText(FText::FromString(NewVolume));
}

void USoundSettingsWidget::OnSFXDownPressed()
{
	auto Instance = GetGameInstance();
	auto NewVolume = FString::FromInt(Instance->UpdateSoundEffectsSoundVolume(false) * 100.f);
	NewVolume.Append("%");
	TextBlock_SFXVolume->SetText(FText::FromString(NewVolume));
}

void USoundSettingsWidget::OnSFXUpPressed()
{
	auto Instance = GetGameInstance();
	auto NewVolume = FString::FromInt(Instance->UpdateSoundEffectsSoundVolume(true) * 100.f);
	NewVolume.Append("%");
	TextBlock_SFXVolume->SetText(FText::FromString(NewVolume));
}

UButlerGameInstance* USoundSettingsWidget::GetGameInstance()
{
	if (!GameInstance)
		GameInstance = Cast<UButlerGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	return GameInstance;
}