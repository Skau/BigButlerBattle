// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseUserWidget.h"
#include "PauseWidget.generated.h"

DECLARE_DELEGATE_OneParam(FContinueGameSignature, int);
DECLARE_DELEGATE(FQuitGameSignature);

class UTextBlock;
class UButton;
class UWidgetSwitcher;
class USoundSettingsWidget;
class UCameraSettingsWidget;
class UTexture2D;
class UImage;

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
    UButton* Button_Options;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UButton* Button_Quit;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UButton* Button_Sound;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UButton* Button_Camera;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UButton* Button_Back;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UImage* PlayerIcon;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UWidgetSwitcher* Switcher;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    USoundSettingsWidget* SoundSettings;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UCameraSettingsWidget* CameraSettings;

    UPROPERTY(EditDefaultsOnly)
    TArray<UTexture2D*> PlayerIcons;

    FContinueGameSignature ContinueGame;
    FQuitGameSignature QuitGame;

    void Reset();

protected:
    void OnPlayerControllerSet() override;

    UFUNCTION(BlueprintCallable)
    void OnBackButtonPressed() override;

private:
    enum class EWidgetSwitcherIndex
    {
        Main,
        Options,
        Sound,
        Camera
    };

    void SetCurrentWidgetSwitcherIndex(EWidgetSwitcherIndex NewIndex);

    UFUNCTION()
    void OnContinuePressed();

    UFUNCTION()
    void OnOptionsPressed();

    UFUNCTION()
    void OnQuitPressed();

    UFUNCTION()
    void OnSoundPressed();

    UFUNCTION()
    void OnCameraPressed();

    EWidgetSwitcherIndex CurrentIndex = EWidgetSwitcherIndex::Main;
};
