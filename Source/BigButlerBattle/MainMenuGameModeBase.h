// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MainMenuGameModeBase.generated.h"

class UMainMenuWidget;
class AMenuPlayerController;
class UButlerGameInstance;

/**
 * 
 */
UCLASS()
class BIGBUTLERBATTLE_API AMainMenuGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMainMenuGameModeBase();

	FORCEINLINE TArray<AMenuPlayerController*> GetControllers() { return Controllers; }

protected:
	void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UMainMenuWidget> MainMenuWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	float TimeUntilGameStart = 6.f;

	UPROPERTY(EditDefaultsOnly)
	int MinimumPlayersToStartGame = 1;

	UPROPERTY(BlueprintReadOnly)
	UMainMenuWidget* MainMenuWidgetInstance;

	UPROPERTY(EditDefaultsOnly)
	FName LevelToPlay = "Main";

private:
	TArray<AMenuPlayerController*> Controllers;

	void OnPlayerToggledJoinedGame(bool Value, int ID);
	void OnPlayerToggledReady(bool Value, int ID);

	void GameStartCountdown();

	int NumJoinedPlayers = 0;
	int NumReadiedPlayers = 0;

	float ElapsedCountdownTime = 0.0f;

	FTimerHandle HandleStartGame;

	UButlerGameInstance* Instance;
};
