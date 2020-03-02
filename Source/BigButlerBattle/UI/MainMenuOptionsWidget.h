// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BaseUserWidget.h"
#include "MainMenuOptionsWidget.generated.h"

class UWidgetSwitcher;
class UButton;
class UMainMenuWidget;
class USoundSettingsWidget;


/**
 * 
 */
UCLASS()
class BIGBUTLERBATTLE_API UMainMenuOptionsWidget : public UBaseUserWidget
{
	GENERATED_BODY()
	
public:
	UMainMenuOptionsWidget(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UWidgetSwitcher* Switcher;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* Button_Sound;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* Button_Back;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USoundSettingsWidget* SoundSettings;

	UPROPERTY(BlueprintReadOnly)
	UMainMenuWidget* MainMenuWidget;

protected:
	bool Initialize() override;

	UFUNCTION(BlueprintCallable)
	void OnBackButtonPressed() override;

	void OnPlayerControllerSet() override;

private:
	enum class EWidgetSwitcherIndex
	{
		Main,
		Sound
	};

	void SetCurrentWidgetSwitcherIndex(EWidgetSwitcherIndex NewIndex);

	UFUNCTION()
	void OnSoundPressed();

	UFUNCTION()
	void OnBackPressed();

	EWidgetSwitcherIndex CurrentIndex = EWidgetSwitcherIndex::Main;
};
