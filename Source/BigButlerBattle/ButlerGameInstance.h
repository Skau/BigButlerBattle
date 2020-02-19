// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ButlerGameInstance.generated.h"

struct FPlayerOptions
{
	bool InvertCameraYaw = false;
	bool InvertCameraPitch = false;
};

/**
 * 
 */
UCLASS()
class BIGBUTLERBATTLE_API UButlerGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
    void Init() override;

	int GetCurrentRandomSeed() const { return Seed; }

	TArray<FPlayerOptions> PlayerOptions;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Random Generator")
	bool bUseCustomSeed = false;

	UPROPERTY(EditDefaultsOnly, Category = "Random Generator")
	int Seed = 0;
};
