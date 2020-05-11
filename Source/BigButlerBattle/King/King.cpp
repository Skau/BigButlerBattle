// Fill out your copyright notice in the Description page of Project Settings.


#include "King.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Player/PlayerCharacter.h"

AKing::AKing()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	SetRootComponent(MeshComponent);

	BoxCollision = CreateDefaultSubobject<UBoxComponent>("Box Collision");
	BoxCollision->SetupAttachment(RootComponent);

	BoxCollision->SetGenerateOverlapEvents(true);
	BoxCollision->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel2);
	BoxCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Overlap);
}

bool AKing::CheckIfAnyInRange()
{
	for(auto& Player : PlayersInRange)
	{
		if(IsValid(Player) && Player->bHasMainItem)
		{
			Player->OnDeliverTasks.ExecuteIfBound(Player->GetInventory());
			return true;
		}
	}
	return false;
}

void AKing::AddClosePlayer(APlayerCharacter* Player)
{
	if(IsValid(Player))
		PlayersInRange.AddUnique(Player);
}

void AKing::RemoveClosePlayer(APlayerCharacter* Player)
{
	if(PlayersInRange.Num())
		PlayersInRange.RemoveSingle(Player);
}


