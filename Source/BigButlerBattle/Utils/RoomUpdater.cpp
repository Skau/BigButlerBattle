// Fill out your copyright notice in the Description page of Project Settings.


#include "RoomUpdater.h"
#include "Components/BoxComponent.h"
#include "Spawnpoint.h"
#include "Player/PlayerCharacter.h"

ARoomUpdater::ARoomUpdater()
{
	PrimaryActorTick.bCanEverTick = false;

	Collision = CreateDefaultSubobject<UBoxComponent>("Collision");
	Collision->SetupAttachment(RootComponent);

	Collision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Collision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	Collision->SetGenerateOverlapEvents(true);
}

void ARoomUpdater::BeginPlay()
{
	Super::BeginPlay();

	Collision->OnComponentBeginOverlap.AddDynamic(this, &ARoomUpdater::OnCollisionBeginOverlap);
}

void ARoomUpdater::OnCollisionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (auto Player = Cast<APlayerCharacter>(OtherActor))
	{
		Player->CurrentRoom = (Player->CurrentRoom == RoomConnection1) ? RoomConnection2 : RoomConnection1;
	}
}
