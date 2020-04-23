// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerScoreWidget.generated.h"

class UTextBlock;

/**
 * 
 */
UCLASS()
class BIGBUTLERBATTLE_API UPlayerScoreWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	bool Initialize() override;

    void SetPlayerName(const FString& Name);
    void UpdateScore(int NewScore);

protected:
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UTextBlock* PlayerName;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UTextBlock* PlayerScore;

};
