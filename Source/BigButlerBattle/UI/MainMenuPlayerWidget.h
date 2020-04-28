// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseUserWidget.h"
#include "MainMenuPlayerWidget.generated.h"

DECLARE_DELEGATE_TwoParams(FToggleJoinGameSignature, bool, int);
DECLARE_DELEGATE_TwoParams(FToggleReadyGameSignature, bool, int);

class UButton;
class UWidgetSwitcher;
class UTextBlock;
class UCheckBox;
class UMainMenuPlayWidget;
class UCameraSettingsWidget;
class UButlerGameInstance;
class APlayerCharacter;
class UNiagaraSystem;

/**
 * 
 */
UCLASS()
class BIGBUTLERBATTLE_API UMainMenuPlayerWidget : public UBaseUserWidget
{
	GENERATED_BODY()
	

public:
	UMainMenuPlayerWidget(const FObjectInitializer& ObjectInitializer);

	FToggleJoinGameSignature OnToggleJoinedGame;
	FToggleReadyGameSignature OnToggleReadyGame;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* Button_Join;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UWidgetSwitcher* Switcher;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* PlayerNameText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* Button_Ready;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* ButtonReadyText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* Button_CameraOptions;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCameraSettingsWidget* CameraSettings;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<APlayerCharacter> CharacterClass;

	UMainMenuPlayWidget* MainPlayWidget;

	void SpawnCharacter(FTransform Transform);

	APlayerCharacter* CharacterInstance = nullptr;
protected:
	bool Initialize() override;

	void OnPlayerControllerSet() override;

	UFUNCTION(BlueprintCallable)
	void OnBackButtonPressed() override;

	UPROPERTY(EditDefaultsOnly)
	UNiagaraSystem* SpawnParticleEffect;

private:
	enum class EWidgetSwitcherIndex
	{
		Join = 0,
		Main,
		CameraOptions,
		Ready
	};

	void SetCurrentWidgetSwitcherIndex(EWidgetSwitcherIndex NewIndex);

	UFUNCTION()
	void OnJoinPressed();

	void UpdateJoinedStatus(bool bHasJoined) const;

	UFUNCTION()
	void OnReadyPressed();

	void UpdateReadyStatus(bool bIsReady) const;

	UFUNCTION()
	void OnCameraOptionsPressed();

	EWidgetSwitcherIndex CurrentIndex = EWidgetSwitcherIndex::Join;

	int ID = -1;

	FVector ButlerSpawnPosition;

	void ShowCharacter();

	void HideCharacter();

};
