// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BaseUserWidget.h"
#include "GameWidget.generated.h"

class UTextBlock;

DECLARE_DELEGATE(FOnTimerFinishedSignature);

/**
 * 
 */
UCLASS()
class BIGBUTLERBATTLE_API UGameWidget : public UBaseUserWidget
{
	GENERATED_BODY()

public:
	bool Initialize() override;

	void UpdateTimer(const FString& String);

	void OnPlayerInteractMainItem(int ControllerID, bool bPickedUp);

	void OnMainItemSet();

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* Text_Timer;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* Text_Info;

private:
	void UpdateMessage(const FString& Message, const float Duration = 3.f);
	
	FTimerHandle HandleInfoMessage;
};
