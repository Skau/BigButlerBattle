// Fill out your copyright notice in the Description page of Project Settings.


#include "King.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "BigButlerBattleGameModeBase.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Tasks/BaseTask.h"

AKing::AKing()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	SetRootComponent(MeshComponent);

	BoxCollision = CreateDefaultSubobject<UBoxComponent>("Box Collision");
	BoxCollision->SetupAttachment(RootComponent);

	BoxCollision->SetGenerateOverlapEvents(true);
	MeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel2);
}

void AKing::BeginPlay()
{
	Super::BeginPlay();

	if (auto GM = Cast<ABigButlerBattleGameModeBase>(UGameplayStatics::GetGameMode(GetWorld())))
	{
		GM->OnTasksGenerated.AddUObject(this, &AKing::OnTasksFinishedGenerated);
	}

	BoxCollision->OnComponentBeginOverlap.AddDynamic(this, &AKing::OnBoxCollisionOverlap);
}

void AKing::OnTasksFinishedGenerated(const TArray<UBaseTask*>& TasksIn)
{
	Tasks = TasksIn;
}

bool AKing::CheckIfDone(UBaseTask* TaskIn)
{
	if (!TaskIn || !Tasks.Num())
		return false;

	for (auto& Task : Tasks)
	{
		if (TaskIn->IsEqual(Task))
		{
			return true;
		}
	}

	return false;
}

void AKing::OnBoxCollisionOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

}



