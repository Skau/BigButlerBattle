// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BaseUserWidget.h"
#include "PlayerWidget.generated.h"

enum class ETaskState;
class UHorizontalBox;
class UTaskWidget;
class UTask;

/**
 * 
 */
UCLASS()
class BIGBUTLERBATTLE_API UPlayerWidget : public UBaseUserWidget
{
	GENERATED_BODY()
	
public:
	/* Generates a UI Widget for every task */
	void InitializeTaskWidgets(const TArray<TPair<UTask*, ETaskState>>& Tasks);

	void UpdateTaskSlotName(const int Index, const FString& Name);
	void UpdateTaskState(int Index, ETaskState State);

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UHorizontalBox* TaskBox;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UTaskWidget> TaskWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	FSlateColor NotPresent;

	UPROPERTY(EditDefaultsOnly)
	FSlateColor Present;

	UPROPERTY(EditDefaultsOnly)
	FSlateColor Finished;

private:
	TArray<UTaskWidget*> TaskWidgets;
};
