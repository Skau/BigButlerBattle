// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "BBBSettings.generated.h"


/**
 * 
 */
UCLASS(Config = Game)
class BIGBUTLERBATTLE_API UBBBSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UBBBSettings(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, config, Category="Procedural Generation")
	bool bUseCustomSeed;

	UPROPERTY(EditAnywhere, config, Category = "Procedural Generation")
	float CustomSeed;
};
