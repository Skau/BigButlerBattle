// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BaseUserWidget.h"
#include "CameraSettingsWidget.generated.h"

class UButton;
class UTextBlock;
class UButlerGameInstance;

/**
 * 
 */
UCLASS()
class BIGBUTLERBATTLE_API UCameraSettingsWidget : public UBaseUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* Button_CameraToggleInvertYaw;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* Text_InvertYaw;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* Button_CameraToggleInvertPitch;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* Text_InvertPitch;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* Button_Back;
	
protected:
	bool Initialize() override;

	void OnPlayerControllerSet() override;

	UFUNCTION(BlueprintCallable)
	void OnBackButtonPressed() override;

private:
	UFUNCTION()
	void OnCameraToggleInvertYawPressed();

	UFUNCTION()
	void OnCameraToggleInvertPitchPressed();

private:
	UButlerGameInstance* GameInstance;
	UButlerGameInstance* GetGameInstance();

	int ID = -1;
};
