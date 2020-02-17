// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseUserWidget.h"
#include "MainMenuPlayerWidget.generated.h"

DECLARE_DELEGATE_TwoParams(ToggleJoinGameSignature, bool, int);
DECLARE_DELEGATE_TwoParams(ToggleReadyGameSignature, bool, int);

class UButton;
class UWidgetSwitcher;
class UTextBlock;
class UCheckBox;
class UMainMenuPlayWidget;

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

	ToggleJoinGameSignature OnToggleJoinedGame;
	ToggleReadyGameSignature OnToggleReadyGame;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* Button_Join;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UWidgetSwitcher* Switcher;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* PlayerNameText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* Button_ToggleReady;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* ButtonReadyText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* Button_CameraOptions;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* Button_CameraToggleInvert;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* Text_Invert;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* Button_Back;

	UMainMenuPlayWidget* MainPlayWidget;

protected:
	bool Initialize() override;

	virtual void OnPlayerCharacterControllerSet() override;

	UFUNCTION(BlueprintCallable)
	void OnBackButtonPressed() override;

private:
	void SetCurrentWidgetSwitcherIndex(EWidgetSwitcherIndex NewIndex);

	UFUNCTION()
	void OnJoinPressed();

	void UpdateJoinedStatus(bool bHasJoined);

	UFUNCTION()
	void OnReadyPressed();

	void UpdateReadyStatus(bool bIsReady);

	UFUNCTION()
	void OnCameraOptionsPressed();

	UFUNCTION()
	void OnCameraToggleInvertPressed();

	bool CameraInvert = false;

	UFUNCTION()
	void OnBackPressed();

	EWidgetSwitcherIndex CurrentIndex = EWidgetSwitcherIndex::Join;
};
