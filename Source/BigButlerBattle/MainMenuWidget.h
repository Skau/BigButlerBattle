// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UWidgetSwitcher;
class UMainMenuPlayerWidget;
class UHorizontalBox;
class UTextBlock;

/**
 * 
 */
UCLASS()
class BIGBUTLERBATTLE_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UMainMenuWidget(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UMainMenuPlayerWidget* PlayerWidget_0;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UMainMenuPlayerWidget* PlayerWidget_1;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UMainMenuPlayerWidget* PlayerWidget_2;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UMainMenuPlayerWidget* PlayerWidget_3;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UHorizontalBox* GameTimerBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* GameStartTime;

protected:
	void NativeConstruct() override;

	bool Initialize() override;
};
