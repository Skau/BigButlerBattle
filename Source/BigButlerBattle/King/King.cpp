// Fill out your copyright notice in the Description page of Project Settings.


#include "King.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"

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



