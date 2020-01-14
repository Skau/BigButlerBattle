// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ButlerGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class BIGBUTLERBATTLE_API UButlerGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	int NumberOfPlayers = 1;

};
