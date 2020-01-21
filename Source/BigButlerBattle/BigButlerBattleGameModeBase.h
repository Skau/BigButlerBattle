// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DataTables.h"
#include "BigButlerBattleGameModeBase.generated.h"

DECLARE_DELEGATE_OneParam(TasksGeneratedSignature, const TArray<FTask>&);

class APlayerCharacter;
class APlayerCharacterController;
class UPauseWidget;

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

USTRUCT(BlueprintType)
struct FTask
{
	GENERATED_BODY()

	FTask()
	: ObjectName("")
	, Data()
	{}

	FTask(FString Name, FBaseTableData TableData)
	: ObjectName(Name)
	, Data(TableData)
	{}

	bool IsEqual(FTask Other)
	{
		return ObjectName == Other.ObjectName && Data.IsEqual(&Other.Data);
	}

	UPROPERTY(BlueprintReadWrite)
	FString ObjectName;

	UPROPERTY(BlueprintReadWrite)
	FBaseTableData Data;
};

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

	FORCEINLINE const TArray<FTask> GetGeneratedTasks() const { return Tasks; }

	TasksGeneratedSignature OnTasksGenerated;

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

	TArray<FTask> Tasks;

	UFUNCTION()
	void OnPlayerPaused(APlayerCharacterController* Controller);
	UFUNCTION()
	void OnPlayerContinued(APlayerCharacterController* Controller);
	UFUNCTION()
	void OnPlayerQuit();

	UFUNCTION()
	void GenerateTasks();
	
};
