// Fill out your copyright notice in the Description page of Project Settings.


#include "OutOfBounds.h"
#include "Components/BoxComponent.h"
#include "Tasks/TaskObject.h"
#include "Player/PlayerCharacter.h"

AOutOfBounds::AOutOfBounds()
{
	PrimaryActorTick.bCanEverTick = false;

	Collision = CreateDefaultSubobject<UBoxComponent>("Collision");
	SetRootComponent(Collision);

	Collision->SetGenerateOverlapEvents(true);
	Collision->SetBoxExtent(FVector(1000000.f, 1000000.f, 10.f));
}

void AOutOfBounds::BeginPlay()
{
	Super::BeginPlay();
	
	Collision->OnComponentBeginOverlap.AddDynamic(this, &AOutOfBounds::OnComponentBeginOverlap);
}

void AOutOfBounds::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (auto Player = Cast<APlayerCharacter>(OtherActor))
	{
		if (bCanAffectPlayers)
		{
			Player->EnableRagdoll();
		}
	}
	else if(auto TaskObject = Cast<ATaskObject>(OtherActor))
	{
		TaskObject->Reset();
	}
	else
	{
		OtherActor->Destroy();
	}
}


