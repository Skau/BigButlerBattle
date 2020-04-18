// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ObjectiveMarker.generated.h"

// Forward declarations
class UMaterialBillboardComponent;

UCLASS() class BIGBUTLERBATTLE_API AObjectiveMarker : public AActor
{
	GENERATED_BODY()

public:
	AObjectiveMarker();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UMaterialBillboardComponent* Billboard;

};
