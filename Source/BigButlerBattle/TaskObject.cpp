// Fill out your copyright notice in the Description page of Project Settings.


#include "TaskObject.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"

ATaskObject::ATaskObject()
{
	// PrimaryActorTick.bCanEverTick = true;

	ColliderComponent = CreateDefaultSubobject<UBoxComponent>("Collider");
	SetRootComponent(ColliderComponent);

	ColliderComponent->SetGenerateOverlapEvents(true);
	ColliderComponent->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel1);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("Mesh Component");
	MeshComponent->SetupAttachment(RootComponent);
}


void ATaskObject::BeginPlay()
{
	Super::BeginPlay();
	
}


void ATaskObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

