// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomGameViewportClient.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"

EMouseCursor::Type UCustomGameViewportClient::GetCursor(FViewport* InViewport, int32 X, int32 Y)
{
    if (GameInstance && GameInstance->GetFirstLocalPlayerController())
    {
        return GameInstance->GetFirstLocalPlayerController()->GetMouseCursor();
    }

    return FViewportClient::GetCursor(InViewport, X, Y);
}
