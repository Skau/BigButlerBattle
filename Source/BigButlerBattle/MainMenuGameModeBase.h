// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyGameModeBase.h"
#include "MainMenuGameModeBase.generated.h"

class UMainMenuWidget;
class UMainMenuPlayWidget;
class UHelpWidget;
class UMainMenuOptionsWidget;

/**
 * 
 */
UCLASS()
class BIGBUTLERBATTLE_API AMainMenuGameModeBase : public AMyGameModeBase
{
	GENERATED_BODY()

public:
	bool HasAnyPlayerJoined() const { return NumJoinedPlayers != 0; }

protected:
	void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UMainMenuWidget> MainMenuWidgetClass = nullptr;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UMainMenuPlayWidget> MainMenuPlayWidgetClass = nullptr;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UHelpWidget> HelpWidgetClass = nullptr;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UMainMenuOptionsWidget> MainMenuOptionsWidgetClass = nullptr;

	UPROPERTY(EditDefaultsOnly)
	float TimeUntilGameStart = 6.f;

	UPROPERTY(EditDefaultsOnly)
	int MinimumPlayersToStartGame = 1;

	UPROPERTY(EditDefaultsOnly)
	FName LevelToPlay = "Game";

private:
	void OnPlayerToggledJoinedGame(bool Value, int ID);
	void OnPlayerToggledReady(bool Value, int ID);

	void StartCountdown();
	void Countdown();
	void EndCountdown();

	UPROPERTY()
	TArray<APlayerController*> Controllers;

	TArray<int> PlayerNotJoinedIDs = { 0, 1, 2, 3 };

	UMainMenuWidget* MainMenuWidgetInstance = nullptr;
	UMainMenuPlayWidget* MainMenuPlayWidgetInstance = nullptr;
	UHelpWidget* HelpWidgetInstance = nullptr;
	UMainMenuOptionsWidget* MainMenuOptionsWidgetInstance = nullptr;

	FTimerHandle HandleStartGame;
	float ElapsedCountdownTime = 0.0f;

	int NumJoinedPlayers = 0;
	int NumReadiedPlayers = 0;
};
