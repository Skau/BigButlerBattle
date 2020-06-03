// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Utils/Spawnpoint.h"
#include "PlayerCharacterController.generated.h"


UENUM()
enum class EMainItemState : uint8
{
	PickedUp,
	Dropped,
	Delivered,
	Destroyed
};

// Broadcasted when the player presses pause
DECLARE_DELEGATE_OneParam(FPauseGameSignature, int);

// Broadcasted when the player delivers the main item
DECLARE_DELEGATE_OneParam(FDeliveredItem, int);

// Broadcasted when main item is dropped or picked up, passing along controller ID.
DECLARE_DELEGATE_TwoParams(FMainItemStateChangedSignature, int, EMainItemState);


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
	FPauseGameSignature OnPausedGame;
	FDeliveredItem OnDeliveredItem;

	FMainItemStateChangedSignature OnMainItemStateChange;

	void RespawnCharacter(ASpawnpoint* Spawnpoint);
	void RespawnCharacter(const FTransform& Spawntrans);

	bool bUseCustomSpringArmLength = false;

	void UpdateCameraSettings();

	int GetScore() const { return Score; }

	UPlayerWidget* GetPlayerWidget() const { return PlayerWidget; }

protected:
	void BeginPlay() override;

	void OnPossess(APawn* InPawn) override;

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
	int Score = 0;

	void PauseGamePressed();

	UFUNCTION()
	void OnPlayerPickedUpObject(ATaskObject* Object);

	UFUNCTION()
	void OnPlayerDroppedObject(ATaskObject* Object);

	UFUNCTION()
	void CheckIfTasksAreDone(TArray<ATaskObject*>& Inventory);

	void OnCharacterFell(ERoomSpawn Room, FVector Position);

	void ShowKeybinds();

	void HideKeybinds();
};
