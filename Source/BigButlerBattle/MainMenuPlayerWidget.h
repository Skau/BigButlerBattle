// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuPlayerWidget.generated.h"

class UTextBlock;
class UCheckBox;
/**
 * 
 */
UCLASS()
class BIGBUTLERBATTLE_API UMainMenuPlayerWidget : public UUserWidget
{
	GENERATED_BODY()
	

public:
	UMainMenuPlayerWidget(const FObjectInitializer& ObjectInitializer);

	void SetPlayerName(FText Text);
	void SetPlayerReadyState(ECheckBoxState State);
	ECheckBoxState GetPlayerReadyState();

	FORCEINLINE bool getHasJoined() { return bHasJoined; }

protected:
	void NativeConstruct() override;

	bool Initialize() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* PlayerNameText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCheckBox* CheckBox;

private:
	bool bHasJoined = false;
};
