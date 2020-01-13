// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.


#include "BigButlerBattleGameModeBase.h"

float ABigButlerBattleGameModeBase::GetAngleBetween(FVector Vector1, FVector Vector2)
{
	return FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Vector1, Vector2) / (Vector1.Size() * Vector2.Size())));
}

float ABigButlerBattleGameModeBase::GetAngleBetweenNormals(FVector Normal1, FVector Normal2)
{
	return FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Normal1, Normal2)));
}