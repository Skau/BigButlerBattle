// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerWidget.generated.h"

class UTaskWidget;

/**
 * 
 */
UCLASS()
class BIGBUTLERBATTLE_API UPlayerWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void UpdateTaskSlotName(FString name, int index);

protected:
	bool Initialize() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTaskWidget* TaskSlot0;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTaskWidget* TaskSlot1;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTaskWidget* TaskSlot2;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTaskWidget* TaskSlot3;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTaskWidget* TaskSlot4;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTaskWidget* TaskSlot5;

private:
	TArray<UTaskWidget*> TaskWidgets;
};
