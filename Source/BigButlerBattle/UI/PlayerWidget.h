// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BaseUserWidget.h"
#include "PlayerWidget.generated.h"

enum class ETaskState;
class UTaskWidget;

/**
 * 
 */
UCLASS()
class BIGBUTLERBATTLE_API UPlayerWidget : public UBaseUserWidget
{
	GENERATED_BODY()
	
public:
	void UpdateTaskSlotName(int index, FString Name);
	void UpdateTaskState(int index, ETaskState State);

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

	UPROPERTY(EditDefaultsOnly)
	FSlateColor NotPresent;

	UPROPERTY(EditDefaultsOnly)
	FSlateColor Present;

	UPROPERTY(EditDefaultsOnly)
	FSlateColor Finished;

private:
	TArray<UTaskWidget*> TaskWidgets;
};
