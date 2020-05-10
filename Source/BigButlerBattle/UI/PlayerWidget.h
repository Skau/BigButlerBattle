// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BaseUserWidget.h"
#include "PlayerWidget.generated.h"

class UHorizontalBox;
class UTextBlock;
class UVerticalBox;
class UTexture2D;
class APlayerCharacterController;
class UPlayerScoreWidget;
class UImage;
class ATaskObject;
class APlayerCharacter;
class ABigButlerBattleGameModeBase;
class AKing;
enum class EMainItemState : uint8;

/**
 * 
 */
UCLASS()
class BIGBUTLERBATTLE_API UPlayerWidget : public UBaseUserWidget
{
	GENERATED_BODY()

public:
    bool Initialize() override;

	void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
    void UpdateTimer(const FString& String);

    void OnMainItemStateChanged(int ControllerID, EMainItemState NewState);

    void OnMainItemSet(ATaskObject* Object);

    // Used to create the widgets on start (called by gamemode)
    void InitializeScores(const TArray<APlayerCharacterController*>& Controllers);

    // Used everytime someone's score changes (called by gamemode)
    void UpdateScore(int ControllerID, int NewScore);

    void ShowKeybinds();

    void HideKeybinds();

    int ID = -1;

protected:
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UHorizontalBox* TimerBox;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UTextBlock* Text_Timer;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UImage* Image_Keybinds;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UHorizontalBox* ScoreBox;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UVerticalBox* MessageBox;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UWidget* MainItemIconWidget;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UWidget* KingIconWidget;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UImage* PlayerIcon1;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UImage* PlayerIcon2;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UImage* PlayerIcon3;
	
    UPROPERTY(EditDefaultsOnly)
    TSubclassOf<UPlayerScoreWidget> PlayerScoreWidgetClass;

    UPROPERTY(EditDefaultsOnly)
    FSlateFontInfo FontInfo;

    UPROPERTY(EditDefaultsOnly)
    TArray<UTexture2D*> PlayerIcons;

    UPROPERTY(EditDefaultsOnly)
    UTexture2D* MainItemIcon;

	

private:
    ATaskObject* MainItem;

    AKing* King;

    bool bHasMainItem = false;
	
    void AddMessage(int ControllerID, const FString& Message, const float Duration = 3.f);

    void SetTimerVisiblity(bool Visible);

    TArray<UImage*> PlayerIconWidgets;
	
    TArray<APlayerCharacterController*> PlayerControllers;

    ABigButlerBattleGameModeBase* GameMode;

    TMap<int, UPlayerScoreWidget*> PlayerScores;

    TArray<UHorizontalBox*> Messages;

    bool WorldToScreen(const FVector& WorldLocation, FVector2D& ScreenPosition);

    FVector2D ClampPosition(FVector2D Position) const;
};
