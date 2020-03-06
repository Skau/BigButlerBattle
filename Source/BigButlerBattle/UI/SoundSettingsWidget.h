// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BaseUserWidget.h"
#include "SoundSettingsWidget.generated.h"

class UButton;
class UTextBlock;
class UButlerGameInstance;

/**
 * 
 */
UCLASS()
class BIGBUTLERBATTLE_API USoundSettingsWidget : public UBaseUserWidget
{
	GENERATED_BODY()
	
public:
	USoundSettingsWidget(const FObjectInitializer& ObjectInitializer);

	/* Master */

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* Button_MasterDown;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* Button_MasterUp;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* TextBlock_MasterVolume;

	/* Background */

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* Button_BackgroundDown;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* Button_BackgroundUp;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* TextBlock_BackgroundVolume;

	/* SFX */

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* Button_SFXDown;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* Button_SFXUp;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* TextBlock_SFXVolume;

	/* Misc */

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* Button_Back;

protected:
	bool Initialize() override;

	UFUNCTION(BlueprintCallable)
	void OnBackButtonPressed() override;

	UFUNCTION()
	void OnMasterDownPressed();

	UFUNCTION()
	void OnMasterUpPressed();

	UFUNCTION()
	void OnBackgroundDownPressed();

	UFUNCTION()
	void OnBackgroundUpPressed();

	UFUNCTION()
	void OnSFXDownPressed();

	UFUNCTION()
	void OnSFXUpPressed();
	
private:
	UButlerGameInstance* GetGameInstance();
};
