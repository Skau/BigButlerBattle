// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseTask.h"
#include "FoodTask.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class BIGBUTLERBATTLE_API UFoodTask : public UBaseTask
{
	GENERATED_BODY()
	
public:
	bool InitTaskData(uint8* Data) override;

	bool IsEqual(const UBaseTask* Other) const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int Temperature = 0;
};
