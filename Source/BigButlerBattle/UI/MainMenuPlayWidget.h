// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BaseUserWidget.h"
#include "MainMenuPlayWidget.generated.h"

class UMainMenuWidget;
class UMainMenuPlayerWidget;
class UHorizontalBox;
class UTextBlock;

/**
 * 
 */
UCLASS()
class BIGBUTLERBATTLE_API UMainMenuPlayWidget : public UBaseUserWidget
{
	GENERATED_BODY()
	
public:
	UMainMenuPlayWidget(const FObjectInitializer& ObjectInitializer);

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

	UPROPERTY(BlueprintReadOnly)
	UMainMenuWidget* MainMenuWidget;

	UFUNCTION(BlueprintCallable)
	void BackToMainMenu();

protected:
	void NativeConstruct() override;

	bool Initialize() override;

};
