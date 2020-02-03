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

	UMainMenuPlayWidget* MainPlayWidget;

protected:
	bool Initialize() override;

	virtual void OnPlayerCharacterControllerSet() override;

	UFUNCTION(BlueprintCallable)
	void OnBackButtonPressed() override;

private:
	void UpdatePlayerName();

	UFUNCTION()
	void OnJoinClicked();

	UFUNCTION()
	void OnToggledReady();

	void UpdateToggledReady();

	bool bIsReady = false;
	bool bHasJoined = false;
};
