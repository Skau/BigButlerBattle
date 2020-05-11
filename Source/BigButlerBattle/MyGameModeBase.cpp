// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameModeBase.h"
#include "TimerManager.h"
#include "ButlerGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "MainMenuGameModeBase.h"

void AMyGameModeBase::AddTimerHandle(const FTimerHandle TimerHandle)
{
	TimerHandles.Add(TimerHandle);
}

void AMyGameModeBase::StartToLeaveMap()
{
	for (int i = 0; i < TimerHandles.Num(); ++i)
	{
		GetWorldTimerManager().ClearTimer(TimerHandles[i]);
	}
}

void AMyGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	auto Instance = Cast<UButlerGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	if (Instance)
	{
		Instance->LevelChanged(IsA<AMainMenuGameModeBase>());
	}
}