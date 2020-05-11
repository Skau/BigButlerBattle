// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ButlerGameInstance.generated.h"

class UAudioComponent;
class USoundClass;
class USoundBase;
class USoundConcurrency;

struct FPlayerOptions
{
	bool InvertCameraYaw = false;
	bool InvertCameraPitch = false;
};

/**
 * 
 */
UCLASS()
class BIGBUTLERBATTLE_API UButlerGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UButlerGameInstance(const FObjectInitializer& ObjectInitializer);

    void Init() override;

	int GetCurrentRandomSeed() const { return CustomSeed; }

	TArray<FPlayerOptions> PlayerOptions;

	float GetMainSoundVolume() const;

	float UpdateMainSoundVolume(bool bShouldIncrement) const;

	float GetBackgroundSoundVolume() const;

	float UpdateBackgroundSoundVolume(bool bShouldIncrement) const;

	float GetSoundEffectsSoundVolume() const;

	float UpdateSoundEffectsSoundVolume(bool bShouldIncrement) const;

	void LevelChanged(bool bNewLevelIsMainMenu);

protected:
	void Shutdown() override;

	UPROPERTY(EditDefaultsOnly)
	bool bUseCustomSeed;

	UPROPERTY(EditDefaultsOnly)
	int CustomSeed;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	float IncrementValue = 0.1f;

	UPROPERTY(EditDefaultsOnly)
	USoundClass* MainSoundClass;

	UPROPERTY(EditDefaultsOnly)
	USoundClass* BackgroundSoundClass;

	UPROPERTY(EditDefaultsOnly)
	USoundClass* SoundEffectsSoundClass;

	UPROPERTY(EditDefaultsOnly)
	USoundBase* BackgroundMusic;

	UPROPERTY(EditDefaultsOnly)
	USoundConcurrency* SoundConcurrency;

private:
	UPROPERTY()
	UAudioComponent* AudioComponent;

	void FadeBetweenMusic(bool bNewLevelIsMainMenu);
	float Fade = 0.f;
};
