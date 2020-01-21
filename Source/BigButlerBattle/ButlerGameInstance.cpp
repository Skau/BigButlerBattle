// Fill out your copyright notice in the Description page of Project Settings.


#include "ButlerGameInstance.h"

void UButlerGameInstance::Init()
{
	Super::Init();

	if (!bUseCustomSeed)
	{
		FRandomStream stream;
		stream.GenerateNewSeed();
		Seed = stream.GetCurrentSeed();
	}
}
