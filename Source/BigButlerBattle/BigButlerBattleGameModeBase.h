// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Utils/DataTables.h"
#include "Utils/Spawnpoint.h"
#include "BigButlerBattleGameModeBase.generated.h"

class APlayerCharacterController;
class UPauseWidget;
class UTask;
class UGameFinishedWidget;
class ASpawnpoint;

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

public:
	ASpawnpoint* GetRandomSpawnpoint(ERoomSpawn Room, FVector Position);

protected:
	void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPauseWidget> PauseWidgetClass;

	UPauseWidget* PauseWidget;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameFinishedWidget> GameFinishedWidgetClass;

	UGameFinishedWidget* GameFinishedWidget;

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

	int RemainingTasksToCreate = 0;

	void SetupSpawnpoints();

	/*
	 Starts the process of generating tasks
	*/
	UFUNCTION()
	void BeginTaskGeneration();

	/* 
	 Helper function that returns all tasks found in world.
	 Return value is a TMap where key is the type of task and the value is an array containing the tasks
	*/
	TMap<EObjectType, TArray<UTask*>> GetWorldTaskData();

	/*
	 Helper function that returns all tasks that needs to be created
	*/
	TArray<UTask*> GenerateTasks(const TArray<EObjectType>& Types, TMap<EObjectType, FIntRange>& Ranges, const FRandomStream& Stream, TMap<EObjectType, TArray<UTask*>>& WorldTaskData, bool bShouldGenerateMinTasks);

	/*
	 Helper function that returns a list of tasks based on the random stream
	*/
	TArray<UTask*> ProcessWorldTasks(TArray<UTask*>& TaskData, const FRandomStream& Stream, int Min, int Max);

	/*
	 Called when the task generation is done
	*/
	void EndTaskGeneration(TArray<UTask*> Tasks);

	/*
	 Sets up the individual player tasks and gives them to the respective controller
	 Called from EndTaskGeneration()
	*/
	void GeneratePlayerTasks(TArray<UTask*> Tasks);


	double TaskGenerationStartTime = 0;

	void GenerateExtraTask();

	TMap<ERoomSpawn, TArray<ASpawnpoint*>> Spawnpoints;

};
