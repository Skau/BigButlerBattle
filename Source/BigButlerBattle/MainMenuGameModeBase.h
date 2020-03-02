// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MainMenuGameModeBase.generated.h"

class UMainMenuWidget;
class UMainMenuPlayWidget;
class UMainMenuOptionsWidget;
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

	UFUNCTION(BlueprintPure)
	TArray<APlayerController*> GetControllers() const { return Controllers; }

	UFUNCTION(BlueprintPure)
	bool HasAnyPlayerJoined() const { return NumJoinedPlayers != 0; }

protected:
	void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UMainMenuWidget> MainMenuWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UMainMenuPlayWidget> MainMenuPlayWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UMainMenuOptionsWidget> MainMenuOptionsWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	float TimeUntilGameStart = 6.f;

	UPROPERTY(EditDefaultsOnly)
	int MinimumPlayersToStartGame = 1;

	UPROPERTY(BlueprintReadOnly)
	UMainMenuWidget* MainMenuWidgetInstance;

	UPROPERTY(BlueprintReadOnly)
	UMainMenuPlayWidget* MainMenuPlayWidgetInstance;

	UPROPERTY(BlueprintReadOnly)
	UMainMenuOptionsWidget* MainMenuOptionsWidgetInstance;

	UPROPERTY(EditDefaultsOnly)
	FName LevelToPlay = "Main";

private:
	TArray<APlayerController*> Controllers;

	void OnPlayerToggledJoinedGame(bool Value, int ID);
	void OnPlayerToggledReady(bool Value, int ID);

	void GameStartCountdown();

	int NumJoinedPlayers = 0;
	int NumReadiedPlayers = 0;

	float ElapsedCountdownTime = 0.0f;

	FTimerHandle HandleStartGame;

	UButlerGameInstance* Instance;

	TArray<int> PlayerNotJoinedIDs = {0, 1, 2, 3};
};
