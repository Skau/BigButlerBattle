// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TaskWidget.generated.h"

enum class ETaskState;
class UTextBlock;

/**
 * 
 */
UCLASS()
class BIGBUTLERBATTLE_API UTaskWidget : public UUserWidget
{
	GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UTextBlock* TaskBlock;
};
