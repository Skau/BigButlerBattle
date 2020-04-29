// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameModeBase.h"
#include "TimerManager.h"

void AMyGameModeBase::AddTimerHandle(FTimerHandle TimerHandle)
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
