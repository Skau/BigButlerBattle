// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BaseUserWidget.h"
#include "MainMenuWidget.generated.h"


class UWidgetSwitcher;
class UButton;
class UMainMenuPlayWidget;
class UMainMenuOptionsWidget;

/**
 * 
 */
UCLASS()
class BIGBUTLERBATTLE_API UMainMenuWidget : public UBaseUserWidget
{
	GENERATED_BODY()

public:
	UMainMenuWidget(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* Button_Play;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* Button_Options;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* Button_Quit;

	UMainMenuPlayWidget* PlayWidget;

	UMainMenuOptionsWidget* OptionsWidget;

protected:
	void NativeConstruct() override;

	bool Initialize() override;

private:
	UFUNCTION()
	void OnPlayPressed();

	UFUNCTION()
	void OnOptionsPressed();

	UFUNCTION()
	void OnQuitPressed();
};
