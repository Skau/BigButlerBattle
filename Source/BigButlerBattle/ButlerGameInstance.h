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

	int GetCurrentRandomSeed() const { return Seed; }

	TArray<FPlayerOptions> PlayerOptions;

	void SetMainSoundVolume(float Value);

	void SetBackgroundSoundVolume(float Value);

	void SetSoundEffectsSoundVolume(float Value);

	void LevelChanged(bool bNewLevelIsMainMenu);

protected:
	void Shutdown() override;

	UPROPERTY(EditDefaultsOnly, Category = "Random Generator")
	bool bUseCustomSeed = false;

	UPROPERTY(EditDefaultsOnly, Category = "Random Generator")
	int Seed = 0;

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
