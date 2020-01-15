// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.


#include "BigButlerBattleGameModeBase.h"
#include "ButlerGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "PlayerCharacterController.h"
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
		if (IDs.Num() > 0)
		{
			for (int i = 0; i < IDs.Num(); ++i)
			{
				FActorSpawnParameters Params;
				Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
				auto Character = GetWorld()->SpawnActor<APlayerCharacter>(PlayerCharacterClass, Params);
				auto PC = Cast<APlayerCharacterController>(UGameplayStatics::GetPlayerControllerFromID(GetWorld(), IDs[i]));
				PC->Possess(Character);
				PC->PauseGame.BindUObject(this, &ABigButlerBattleGameModeBase::OnPlayerPaused);
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
		else
		{
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			auto Character = GetWorld()->SpawnActor<APlayerCharacter>(PlayerCharacterClass, Params);
			auto PC = Cast<APlayerCharacterController>(UGameplayStatics::GetPlayerControllerFromID(GetWorld(), 0)); 
			PC->Possess(Character);
			PC->PauseGame.BindUObject(this, &ABigButlerBattleGameModeBase::OnPlayerPaused);
		}
	}
}

void ABigButlerBattleGameModeBase::OnPlayerPaused(APlayerCharacterController* Controller)
{
	UE_LOG(LogTemp, Warning, TEXT("Player paused but it's not possible lmao"));
}
