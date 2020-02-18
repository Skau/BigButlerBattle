// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "Spawnpoint.generated.h"

UENUM(BlueprintType)
enum class ERoomSpawn : uint8
{
	Room_MainHall		UMETA(DisplayName = "Main Hall"),
	Room_Kitchen		UMETA(DisplayName = "Kitchen"),
	Room_SquareHall		UMETA(DisplayName = "SquareHall"),
	Room_StairRoom		UMETA(DisplayName = "StairRoom"),
	Room_Bar			UMETA(DisplayName = "Bar")
};

/**
 * 
 */
UCLASS()
class BIGBUTLERBATTLE_API ASpawnpoint : public APlayerStart
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	bool bIsStartSpawn = false;

	UPROPERTY(EditAnywhere)
	ERoomSpawn RoomSpawn = ERoomSpawn::Room_MainHall;
};
