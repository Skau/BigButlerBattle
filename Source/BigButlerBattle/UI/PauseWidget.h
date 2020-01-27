// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseUserWidget.h"
#include "PauseWidget.generated.h"

DECLARE_DELEGATE_OneParam(ContinueGameSignature, int);
DECLARE_DELEGATE(QuitGameSignature);

class UTextBlock;
class UButton;

/**
 * 
 */
UCLASS()
class BIGBUTLERBATTLE_API UPauseWidget : public UBaseUserWidget
{
	GENERATED_BODY()

public:
    bool Initialize() override;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UTextBlock* PlayerText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UButton* Button_Continue;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UButton* Button_Quit;

    ContinueGameSignature ContinueGame;
    QuitGameSignature QuitGame;

protected:
    void OnPlayerCharacterControllerSet() override;

private:
  
    UFUNCTION()
    void OnContinuePressed();

    UFUNCTION()
    void OnQuitPressed();
};
