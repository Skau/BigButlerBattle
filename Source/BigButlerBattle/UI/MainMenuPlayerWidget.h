// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseUserWidget.h"
#include "MainMenuPlayerWidget.generated.h"

DECLARE_DELEGATE_TwoParams(FToggleJoinGameSignature, bool, int);
DECLARE_DELEGATE_TwoParams(FToggleReadyGameSignature, bool, int);

class UButton;
class UWidgetSwitcher;
class UTextBlock;
class UCheckBox;
class UMainMenuPlayWidget;
class UButlerGameInstance;

enum class EWidgetSwitcherIndex
{
	Join = 0,
	Main,
	CameraOptions, 
	Ready
};

/**
 * 
 */
UCLASS()
class BIGBUTLERBATTLE_API UMainMenuPlayerWidget : public UBaseUserWidget
{
	GENERATED_BODY()
	

public:
	UMainMenuPlayerWidget(const FObjectInitializer& ObjectInitializer);

	FToggleJoinGameSignature OnToggleJoinedGame;
	FToggleReadyGameSignature OnToggleReadyGame;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* Button_Join;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UWidgetSwitcher* Switcher;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* PlayerNameText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* Button_Ready;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* ButtonReadyText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* Button_CameraOptions;

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

	UMainMenuPlayWidget* MainPlayWidget;

protected:
	bool Initialize() override;

	void OnPlayerCharacterControllerSet() override;

	UFUNCTION(BlueprintCallable)
	void OnBackButtonPressed() override;

private:
	void SetCurrentWidgetSwitcherIndex(EWidgetSwitcherIndex NewIndex);

	UFUNCTION()
	void OnJoinPressed();

	void UpdateJoinedStatus(bool bHasJoined) const;

	UFUNCTION()
	void OnReadyPressed();

	void UpdateReadyStatus(bool bIsReady) const;

	UFUNCTION()
	void OnCameraOptionsPressed();

	UFUNCTION()
	void OnCameraToggleInvertYawPressed();

	UFUNCTION()
	void OnCameraToggleInvertPitchPressed();

	UFUNCTION()
	void OnBackPressed();

	EWidgetSwitcherIndex CurrentIndex = EWidgetSwitcherIndex::Join;

	int ID = -1;
	UButlerGameInstance* GameInstance;

};
