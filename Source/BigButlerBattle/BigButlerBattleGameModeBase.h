// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BigButlerBattleGameModeBase.generated.h"

class APlayerCharacter;

/**
 * 
 */
UCLASS()
class BIGBUTLERBATTLE_API ABigButlerBattleGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	static float GetAngleBetween(FVector Vector1, FVector Vector2);
	static float GetAngleBetweenNormals(FVector Normal1, FVector Normal2);

protected:
	void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<APlayerCharacter> PlayerCharacterClass;
	
};
