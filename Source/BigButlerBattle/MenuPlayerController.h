// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MenuPlayerController.generated.h"


DECLARE_DELEGATE_TwoParams(ToggleJoinGameSignature, bool, int);
DECLARE_DELEGATE_TwoParams(ToggleReadyGameSignature, bool, int);

class UWidgetSwitcher;
class UMainMenuPlayerWidget;

/**
 * 
 */
UCLASS()
class BIGBUTLERBATTLE_API AMenuPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMenuPlayerController();

	ToggleJoinGameSignature OnToggleJoinedGame;
	ToggleReadyGameSignature OnToggleReadyGame;

	int ID = -1;

	void SetPlayerWidgets(UWidgetSwitcher* WidgetSwitcherIn, UMainMenuPlayerWidget* PlayerWidgetIn);

protected:
	void SetupInputComponent() override;

private:
	UWidgetSwitcher* WidgetSwitcher;
	UMainMenuPlayerWidget* PlayerWidget;

	void Activate();
	void Deactivate();
};
