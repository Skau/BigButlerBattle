// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BigButlerBattleGameModeBase.generated.h"

class APlayerCharacter;
class APlayerCharacterController;
class UPauseWidget;
class UBaseTask;

DECLARE_DELEGATE_OneParam(TasksGeneratedSignature, const TArray<UBaseTask*>&);


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
	static float GetAngleBetween(FVector Vector1, FVector Vector2);
	static float GetAngleBetweenNormals(FVector Normal1, FVector Normal2);

	FORCEINLINE const TArray<UBaseTask*> GetGeneratedTasks() const { return Tasks; }

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

	TArray<UBaseTask*> Tasks;

	UFUNCTION()
	void OnPlayerPaused(APlayerCharacterController* Controller);
	UFUNCTION()
	void OnPlayerContinued(APlayerCharacterController* Controller);
	UFUNCTION()
	void OnPlayerQuit();

	UFUNCTION()
	void GenerateTasks();
	
};
