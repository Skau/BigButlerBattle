// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.


#include "BigButlerBattleGameModeBase.h"
#include "ButlerGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "PlayerCharacter.h"

float ABigButlerBattleGameModeBase::GetAngleBetween(FVector Vector1, FVector Vector2)
{
	return FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Vector1, Vector2) / (Vector1.Size() * Vector2.Size())));
}

float ABigButlerBattleGameModeBase::GetAngleBetweenNormals(FVector Normal1, FVector Normal2)
{
	return FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Normal1, Normal2)));
}

void ABigButlerBattleGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	auto Instance = Cast<UButlerGameInstance>(GetGameInstance());
	if (Instance)
	{
		// Spawn and possess
		auto IDs = Instance->PlayerIDs;
		for (int i = 0; i < IDs.Num(); ++i)
		{
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			auto Character = GetWorld()->SpawnActor<APlayerCharacter>(PlayerCharacterClass, Params);
			UGameplayStatics::GetPlayerControllerFromID(GetWorld(), IDs[i])->Possess(Character);
		}

		// Remove unecessary controllers
		for (int i = 0; i < 4; ++i)
		{
			if (auto controller = UGameplayStatics::GetPlayerControllerFromID(GetWorld(), i))
			{
				auto pawn = controller->GetPawn();
				if (!pawn)
				{
					UGameplayStatics::RemovePlayer(controller, false);
				}
			}
		}
	}
}
