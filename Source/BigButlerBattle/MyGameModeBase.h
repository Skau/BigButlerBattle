// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MyGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class BIGBUTLERBATTLE_API AMyGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	

public:
	void AddTimerHandle(FTimerHandle TimerHandle);

	void StartToLeaveMap() override;

	void BeginPlay() override;

private:
	TArray<FTimerHandle> TimerHandles;
};
