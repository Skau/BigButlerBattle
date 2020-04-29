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
class UGameWidget;
class AKing;
class ATaskObject;
enum class EMainItemState : uint8;

/**
 *
 */
UCLASS()
class BIGBUTLERBATTLE_API ABigButlerBattleGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABigButlerBattleGameModeBase();

	ASpawnpoint* GetRandomSpawnpoint(const ERoomSpawn Room, const FVector& Position);
	FVector GetRandomSpawnPos(const FVector& Position) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RespawnRadius = 1000.f;

	void StartToLeaveMap() override;

	void SetNewMainItem(float delay = 0.5f);

	TArray<APlayerCharacterController*> GetControllers() const { return Controllers; }

protected:
	void BeginPlay() override;

	void Tick(float DeltaTime) override;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPauseWidget> PauseWidgetClass;

	UPauseWidget* PauseWidget;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameFinishedWidget> GameFinishedWidgetClass;

	UGameFinishedWidget* GameFinishedWidget;



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (DisplayName = "PickupMainItem"))
	USoundBase *PickupMainItemSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (DisplayName = "DeliverMainItem"))
	USoundBase *DeliverMainItemSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (DisplayName = "DropMainItem"))
	USoundBase *DropMainItemSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (DisplayName = "NewMainItem"))
	USoundBase *NewMainItemSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (DisplayName = "Win"))
	USoundBase *WinSound = nullptr;



private:
	TArray<APlayerCharacterController*> Controllers;

	UFUNCTION()
	void OnGameFinished(int ControllerID) const;
	UFUNCTION()
	void OnPlayerPaused(int ControllerID) const;
	UFUNCTION()
	void OnPlayerContinued(int ControllerID) const;
	UFUNCTION()
	void OnPlayerQuit() const;

	void SetupSpawnpoints();

	TMap<ERoomSpawn, TArray<ASpawnpoint*>> Spawnpoints;


protected:
	UPROPERTY(EditDefaultsOnly)
	float TotalSecondsToHold = 10.f;

	UPROPERTY(EditDefaultsOnly)
	int TotalPointsToWin = 5;

	// Actual seconds left to hold
	float SecondsLeftToHold = TotalSecondsToHold;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameWidget> GameWidgetClass;

	UGameWidget* GameWidget = nullptr;

	bool bItemCurrentlyBeingHeld = false;
	bool bTimeDone = false;

private:
	void OnMainItemStateChanged(int ControllerID, EMainItemState NewState);

	AKing* King = nullptr;
};
