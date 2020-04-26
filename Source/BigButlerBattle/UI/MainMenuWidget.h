// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BaseUserWidget.h"
#include "MainMenuWidget.generated.h"


class UWidgetSwitcher;
class UButton;
class UMainMenuPlayWidget;
class UHelpWidget;
class UMainMenuOptionsWidget;
class AMainMenuGameModeBase;
class ACameraDirector;

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
	UButton* Button_Help;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* Button_Options;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* Button_Quit;

	UMainMenuPlayWidget* PlayWidget;

	UHelpWidget* HelpWidget;

	UMainMenuOptionsWidget* OptionsWidget;

	ACameraDirector* CameraDirector;

	TArray<FTransform> ButlerTransforms;

protected:
	void NativeConstruct() override;

	bool Initialize() override;

private:
	UFUNCTION()
	void OnPlayPressed();

	UFUNCTION()
	void OnHelpPressed();

	UFUNCTION()
	void OnOptionsPressed();

	UFUNCTION()
	void OnQuitPressed();
};
