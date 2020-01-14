// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MenuPlayerController.generated.h"


DECLARE_DELEGATE_OneParam(ToggleJoinGameSignature, bool);
DECLARE_DELEGATE_OneParam(ToggleReadyGameSignature, bool);

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

	void SetPlayerWidgets(UWidgetSwitcher* WidgetSwitcherIn, UMainMenuPlayerWidget* PlayerWidgetIn);

protected:
	void SetupInputComponent() override;

private:
	UWidgetSwitcher* WidgetSwitcher;
	UMainMenuPlayerWidget* PlayerWidget;

	void Activate();
	void Deactivate();
};
