// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BigButlerBattleGameModeBase.generated.h"

class APlayerCharacter;
class APlayerCharacterController;
class UPauseWidget;

/**
 *
 */
UCLASS()
class BIGBUTLERBATTLE_API ABigButlerBattleGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

protected:
	void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<APlayerCharacter> PlayerCharacterClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPauseWidget> PauseWidgetClass;

	UPauseWidget* PauseWidget;
private:
	UFUNCTION()
	void OnPlayerPaused(APlayerCharacterController* Controller);
	UFUNCTION()
	void OnPlayerContinued(APlayerCharacterController* Controller);
	UFUNCTION()
	void OnPlayerQuit();

};
