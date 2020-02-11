// Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.

#include "AnimGraphNode_GetFeetTargets.h"
#include "AnimationGraphSchema.h"

/////////////////////////////////////////////////////
// UAnimGraphNode_GetFeetTargets

UAnimGraphNode_GetFeetTargets::UAnimGraphNode_GetFeetTargets(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{

}

//Title Color!
FLinearColor UAnimGraphNode_GetFeetTargets::GetNodeTitleColor() const
{
    return FLinearColor(0, 12, 12, 1);
}

FText UAnimGraphNode_GetFeetTargets::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
    return FText::FromString(TEXT("Get Skateboard Feet Targets"));
}

FText UAnimGraphNode_GetFeetTargets::GetTooltipText() const
{
    return FText::FromString(TEXT("Gets the skateboard feet targets based on current pose and sockets."));
}

//Node Category
FString UAnimGraphNode_GetFeetTargets::GetNodeCategory() const
{
    return FString("BBB Swag Tools");
}

void UAnimGraphNode_GetFeetTargets::CreateOutputPins()
{
	CreatePin(EGPD_Output, UAnimationGraphSchema::PC_Struct, FComponentSpacePoseLink::StaticStruct(), TEXT("ComponentPose"));
}