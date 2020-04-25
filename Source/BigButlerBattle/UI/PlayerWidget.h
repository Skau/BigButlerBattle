// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BaseUserWidget.h"
#include "PlayerWidget.generated.h"

class UHorizontalBox;
class UTextBlock;
class UImage;
class APlayerCharacterController;
class UPlayerScoreWidget;

/**
 * 
 */
UCLASS()
class BIGBUTLERBATTLE_API UPlayerWidget : public UBaseUserWidget
{
	GENERATED_BODY()

public:
    bool Initialize() override;

    void UpdateTimer(const FString& String);

    void OnMainItemStateChanged(int ControllerID, bool bPickedUp);

    void OnMainItemSet();

    // Used to create the widgets on start (called by gamemode)
    void InitializeScores(const TArray<APlayerCharacterController*>& Controllers);

    // Used everytime someone's score changes (called by gamemode)
    void UpdateScore(int ControllerID, int NewScore);

    void ShowKeybinds();

    void HideKeybinds();

protected:
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UTextBlock* Text_Info;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UTextBlock* Text_Timer;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UImage* Image_Keybinds;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UHorizontalBox* ScoreBox;



    UPROPERTY(EditDefaultsOnly)
    TSubclassOf<UPlayerScoreWidget> PlayerScoreWidgetClass;

private:
    void UpdateMessage(const FString& Message, const float Duration = 3.f);

    TMap<int, UPlayerScoreWidget*> PlayerScores;

};
