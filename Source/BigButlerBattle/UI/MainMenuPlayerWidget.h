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

	UFUNCTION()
	void OnReadyPressed();

	void UpdateJoinedStatus(bool bHasJoined);
	void UpdateReadyStatus(bool bIsReady);

	void OnCameraOptionsPressed();

	EWidgetSwitcherIndex CurrentIndex = EWidgetSwitcherIndex::Join;
};
