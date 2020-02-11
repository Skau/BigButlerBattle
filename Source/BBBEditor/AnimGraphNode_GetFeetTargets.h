// Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AnimGraphDefinitions.h"
#include "AnimGraphNode_Base.h"
#include "Player/Animation/AnimNode_GetFeetTargets.h"
#include "Kismet2/BlueprintEditorUtils.h"

#include "AnimGraphNode_GetFeetTargets.generated.h"

/**
 * Gets the skateboard feet targets based on current pose and sockets.
 */

UCLASS()
class BBBEDITOR_API UAnimGraphNode_GetFeetTargets : public UAnimGraphNode_Base
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = Settings)
    FAnimNode_GetFeetTargets Node;

public:
    UAnimGraphNode_GetFeetTargets(const FObjectInitializer& ObjectInitializer);

    // UEdGraphNode interface
    virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
    virtual FLinearColor GetNodeTitleColor() const override;
    virtual FText GetTooltipText() const override;
    // End of UEdGraphNode interface

    // UAnimGraphNode_Base interface
    virtual FString GetNodeCategory() const override;
    virtual void CreateOutputPins() override;
    // End of UAnimGraphNode_Base interface
};