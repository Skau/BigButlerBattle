// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BBBBPFLibrary.generated.h"

/**
 * 
 */
UCLASS()
class BIGBUTLERBATTLE_API UBBBBPFLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	//Returns the project version set in the 'Project Settings' > 'Description' section of the editor
	UFUNCTION(BlueprintPure, Category = "Project")
	static FString GetProjectVersion();

};
