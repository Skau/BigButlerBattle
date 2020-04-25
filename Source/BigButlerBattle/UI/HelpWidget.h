// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BaseUserWidget.h"
#include "HelpWidget.generated.h"

class UMainMenuWidget;
class UWidgetSwitcher;
class UButton;
/**
 * 
 */
UCLASS()
class BIGBUTLERBATTLE_API UHelpWidget : public UBaseUserWidget
{
	GENERATED_BODY()

public:
	UMainMenuWidget* MainMenuWidget;

protected:
	bool Initialize() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UWidgetSwitcher* Switcher;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* Button_Continue;

	UFUNCTION(BlueprintCallable)
	void OnBackButtonPressed() override;
	
private:
	int CurrentIndex = 0;

	UFUNCTION()
	void OnContinueButtonPressed();

	void BackToMenu();
};
