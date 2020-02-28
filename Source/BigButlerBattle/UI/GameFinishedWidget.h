// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BaseUserWidget.h"
#include "GameFinishedWidget.generated.h"

class UTextBlock;
class UButton;

DECLARE_DELEGATE(FQuitGameSignature);

/**
 * 
 */
UCLASS()
class BIGBUTLERBATTLE_API UGameFinishedWidget : public UBaseUserWidget
{
	GENERATED_BODY()
	
public:
    bool Initialize() override;

    void SetWonText(const FString& Text) const;

    FQuitGameSignature QuitGame;

protected:
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UTextBlock* WonText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UButton* Button_Quit;

private:
    UFUNCTION()
    void OnQuitPressed();

};
