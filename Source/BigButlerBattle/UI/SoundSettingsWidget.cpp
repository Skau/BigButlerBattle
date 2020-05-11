// Fill out your copyright notice in the Description page of Project Settings.


#include "SoundSettingsWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "ButlerGameInstance.h"

USoundSettingsWidget::USoundSettingsWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{}

FString USoundSettingsWidget::ToPercent(const float Value) const
{
	int32 Val = static_cast<int32>(Value * 10.5f) * 10;
	if (Val == 70)
		Val = 69;
	return FString::FromInt(Val) + "%";
}

bool USoundSettingsWidget::Initialize()
{
	const bool bInit = Super::Initialize();

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

	if (const auto Instance = GetGameInstance())
	{
		TextBlock_MasterVolume->SetText(FText::FromString(ToPercent(Instance->GetMainSoundVolume())));

		TextBlock_BackgroundVolume->SetText(FText::FromString(ToPercent(Instance->GetBackgroundSoundVolume())));

		TextBlock_SFXVolume->SetText(FText::FromString(ToPercent(Instance->GetSoundEffectsSoundVolume())));
	}
	return bInit;
}

void USoundSettingsWidget::OnBackButtonPressed()
{
	Button_Back->OnClicked.Broadcast();
}

void USoundSettingsWidget::OnMasterDownPressed()
{
	if (const auto Instance = GetGameInstance())
	{
		TextBlock_MasterVolume->SetText(FText::FromString(ToPercent(Instance->UpdateMainSoundVolume(false))));
	}
	else
		UE_LOG(LogTemp, Error, TEXT("USoundSettingsWidget::OnMasterDownPressed: Could not find game instance!"))
}

void USoundSettingsWidget::OnMasterUpPressed()
{
	if (const auto Instance = GetGameInstance())
	{
		TextBlock_MasterVolume->SetText(FText::FromString(ToPercent(Instance->UpdateMainSoundVolume(true))));
	}
	else
		UE_LOG(LogTemp, Error, TEXT("USoundSettingsWidget::OnMasterUpPressed: Could not find game instance!"))
}

void USoundSettingsWidget::OnBackgroundDownPressed()
{
	if (const auto Instance = GetGameInstance())
	{
		TextBlock_BackgroundVolume->SetText(FText::FromString(ToPercent(Instance->UpdateBackgroundSoundVolume(false))));
	}
	else
		UE_LOG(LogTemp, Error, TEXT("USoundSettingsWidget::OnBackgroundDownPressed: Could not find game instance!"))
}

void USoundSettingsWidget::OnBackgroundUpPressed()
{
	if (const auto Instance = GetGameInstance())
	{
		TextBlock_BackgroundVolume->SetText(FText::FromString(ToPercent(Instance->UpdateBackgroundSoundVolume(true))));
	}
	else
		UE_LOG(LogTemp, Error, TEXT("USoundSettingsWidget::OnBackgroundUpPressed: Could not find game instance!"))
}

void USoundSettingsWidget::OnSFXDownPressed()
{
	if (const auto Instance = GetGameInstance())
	{
		TextBlock_SFXVolume->SetText(FText::FromString(ToPercent(Instance->UpdateSoundEffectsSoundVolume(false))));
	}
	else
		UE_LOG(LogTemp, Error, TEXT("USoundSettingsWidget::OnSFXDownPressed: Could not find game instance!"))
}

void USoundSettingsWidget::OnSFXUpPressed()
{
	if (const auto Instance = GetGameInstance())
	{
		TextBlock_SFXVolume->SetText(FText::FromString(ToPercent(Instance->UpdateSoundEffectsSoundVolume(true))));
	}
	else
		UE_LOG(LogTemp, Error, TEXT("USoundSettingsWidget::OnSFXUpPressed: Could not find game instance!"))
}

UButlerGameInstance* USoundSettingsWidget::GetGameInstance() const
{
	return Cast<UButlerGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
}
