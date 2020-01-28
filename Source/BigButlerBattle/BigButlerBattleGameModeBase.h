// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BigButlerBattleGameModeBase.generated.h"

class APlayerCharacter;
class APlayerCharacterController;
class UPauseWidget;
class UBaseTask;

UENUM()
enum class ETaskState
{
	NotPresent,
	Present,
	Finished
};

USTRUCT(BlueprintType)
struct FIntRange
{
	GENERATED_BODY()

	FIntRange()
	: Min(0)
	, Max(0)
	{}

	UPROPERTY(EditAnywhere)
	int Min;

	UPROPERTY(EditAnywhere)
	int Max;
};

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

	UPROPERTY(EditDefaultsOnly, Category = "Task Generator")
	int TotalTasks = 6;

	UPROPERTY(EditDefaultsOnly, Category = "Task Generator")
	FIntRange FoodRange;

	UPROPERTY(EditDefaultsOnly, Category = "Task Generator")
	FIntRange WineRange;

	UPROPERTY(EditDefaultsOnly, Category = "Task Generator")
	FIntRange CutleryRange;

private:
	TArray<APlayerCharacterController*> Controllers;

	UFUNCTION()
	void OnGameFinished(int ControllerID);
	UFUNCTION()
	void OnPlayerPaused(int ControllerID);
	UFUNCTION()
	void OnPlayerContinued(int ControllerID);
	UFUNCTION()
	void OnPlayerQuit();

	UFUNCTION()
	void GenerateTasks();

	void GeneratePlayerTasks(TArray<UBaseTask*> Tasks);

};
