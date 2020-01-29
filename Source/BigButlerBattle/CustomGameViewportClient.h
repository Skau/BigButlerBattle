// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameViewportClient.h"
#include "CustomGameViewportClient.generated.h"

/**
 * 
 */
UCLASS()
class BIGBUTLERBATTLE_API UCustomGameViewportClient : public UGameViewportClient
{
	GENERATED_BODY()

	EMouseCursor::Type GetCursor(FViewport* InViewport, int32 X, int32 Y) override;
	
};
