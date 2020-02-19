// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Utils/Spawnpoint.h"
#include "PlayerCharacterController.generated.h"

// Broadcasted when the player presses pause
DECLARE_DELEGATE_OneParam(FPauseGameSignature, int);

// Broadcasted when the player finishes all tasks at the king
DECLARE_DELEGATE_OneParam(FGameFinishedSignature, int);

class UBaseUserWidget;
class UPlayerWidget;
class APlayerCharacter;
class ABigButlerBattleGameModeBase;
class ATaskObject;
class UTask;
class APlayerStart;
enum class ETaskState;


/**
 * 
 */
UCLASS()
class BIGBUTLERBATTLE_API APlayerCharacterController : public APlayerController
{
	GENERATED_BODY()

public:
	APlayerCharacterController();

	FPauseGameSignature OnPausedGame;
	FGameFinishedSignature OnGameFinished;

	void SetPlayerTasks(const TArray<TPair<UTask*, ETaskState>>& Tasks);

	TArray<TPair<UTask*, ETaskState>>& GetPlayerTasks() { return PlayerTasks; }

	void SetPlayerTaskName(int Index, const FString& Name) const;
	void SetPlayerTaskState(int Index, ETaskState NewState);

	void CheckIfTasksAreDone(TArray<ATaskObject*>& Inventory);

	void RespawnCharacter(ASpawnpoint* Spawnpoint);

	bool bUseCustomSpringArmLength = false;

protected:
	void BeginPlay() override;

	void OnPossess(APawn* InPawn) override;

	void Tick(float DeltaTime) override;

	void SetupInputComponent() override;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPlayerWidget> PlayerWidgetType;

	UPlayerWidget* PlayerWidget = nullptr;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<APlayerCharacter> PlayerCharacterClass = nullptr;

	APlayerCharacter* PlayerCharacter = nullptr;

	ABigButlerBattleGameModeBase* ButlerGameMode = nullptr;

	UPROPERTY(EditDefaultsOnly)
	float RespawnTime = 3.f;

private:
	TArray<TPair<UTask*, ETaskState>> PlayerTasks;

	void PauseGamePressed();

	UFUNCTION()
	void OnPlayerPickedUpObject(ATaskObject* Object);

	UFUNCTION()
	void OnPlayerDroppedObject(ATaskObject* Object);

	void UpdatePlayerTasks();

	void OnTaskObjectDelivered(ATaskObject* Object);

	void OnCharacterFell(ERoomSpawn Room, FVector Position);
};
