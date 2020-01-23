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
/**
 * 
 */
UCLASS()
class BIGBUTLERBATTLE_API UMainMenuPlayerWidget : public UBaseUserWidget
{
	GENERATED_BODY()
	

public:
	UMainMenuPlayerWidget(const FObjectInitializer& ObjectInitializer);

	FORCEINLINE bool getHasJoined() { return bHasJoined; }

	ToggleJoinGameSignature OnToggleJoinedGame;
	ToggleReadyGameSignature OnToggleReadyGame;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* Button_Join;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UWidgetSwitcher* Switcher;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* PlayerNameText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCheckBox* CheckBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* Button_Leave;

protected:
	bool Initialize() override;

	virtual void OnPlayerCharacterControllerSet() override;

private:
	void UpdatePlayerName();

	UFUNCTION()
	void OnJoinClicked();

	UFUNCTION()
	void OnCheckStateChanged(bool NewCheckState);

	UFUNCTION()
	void OnLeaveClicked();

	bool bHasJoined = false;
};
