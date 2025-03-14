// Fill out your copyright notice in the Description page of Project Settings.


#include "ButlerGameInstance.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundClass.h"
#include "Kismet/GameplayStatics.h"
#include "Utils/btd.h"

UButlerGameInstance::UButlerGameInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UButlerGameInstance::Init()
{
	Super::Init();

	if (!bUseCustomSeed)
	{
		FRandomStream Stream;
		Stream.GenerateNewSeed();
		CustomSeed = Stream.GetCurrentSeed();
	}

	PlayerOptions.AddDefaulted(4);
}

float UButlerGameInstance::GetMainSoundVolume() const
{
	if (MainSoundClass)
	{
		return MainSoundClass->Properties.Volume;
	}
	return -1.f;
}

float UButlerGameInstance::UpdateMainSoundVolume(const bool bShouldIncrement) const
{
	if (MainSoundClass)
	{
		MainSoundClass->Properties.Volume += bShouldIncrement ? IncrementValue : -IncrementValue;
		MainSoundClass->Properties.Volume = FMath::Clamp(MainSoundClass->Properties.Volume, 0.f, 1.f);
		return MainSoundClass->Properties.Volume;
	}
	return -1.f;
}

float UButlerGameInstance::GetBackgroundSoundVolume() const
{
	if (BackgroundSoundClass)
	{
		return BackgroundSoundClass->Properties.Volume;
	}
	return -1.f;
}

float UButlerGameInstance::UpdateBackgroundSoundVolume(const bool bShouldIncrement) const
{
	if (BackgroundSoundClass)
	{
		BackgroundSoundClass->Properties.Volume += bShouldIncrement ? IncrementValue : -IncrementValue;
		BackgroundSoundClass->Properties.Volume = FMath::Clamp(BackgroundSoundClass->Properties.Volume, 0.f, 1.f);
		return BackgroundSoundClass->Properties.Volume;
	}
	return -1.f;
}

float UButlerGameInstance::GetSoundEffectsSoundVolume() const
{
	if (SoundEffectsSoundClass)
	{
		return SoundEffectsSoundClass->Properties.Volume;
	}
	return -1.f;
}

float UButlerGameInstance::UpdateSoundEffectsSoundVolume(const bool bShouldIncrement) const
{
	if (SoundEffectsSoundClass)
	{
		SoundEffectsSoundClass->Properties.Volume += bShouldIncrement ? IncrementValue : -IncrementValue;
		SoundEffectsSoundClass->Properties.Volume = FMath::Clamp(SoundEffectsSoundClass->Properties.Volume, 0.f, 1.f);
		return SoundEffectsSoundClass->Properties.Volume;
	}
	return -1.f;
}

void UButlerGameInstance::LevelChanged(const bool bNewLevelIsMainMenu)
{
	FadeBetweenMusic(bNewLevelIsMainMenu);
}

void UButlerGameInstance::Shutdown()
{
	if (AudioComponent && AudioComponent->IsPlaying())
		AudioComponent->Stop();

	SaveConfig();
}

void UButlerGameInstance::FadeBetweenMusic(bool bNewLevelIsMainMenu)
{
	if (!AudioComponent)
	{
		if (BackgroundMusic)
		{
			AudioComponent = UGameplayStatics::CreateSound2D(GetWorld(), BackgroundMusic, 1.0f, 1.0f, 0.0f, SoundConcurrency, true, true);
			AudioComponent->Play();
		}
	}

	Fade = static_cast<float>(bNewLevelIsMainMenu);

	if(!bNewLevelIsMainMenu)
		AudioComponent->SetIntParameter(FName{ "InGameSoundIndex" }, FMath::RandRange(0, 2));

	btd::Repeat(this, 0.5f, 2, [&, bNewLevelIsMainMenu]()
	{
		Fade += bNewLevelIsMainMenu ? -0.5f : 0.5f;
		AudioComponent->SetFloatParameter(FName{ "Fade" }, Fade);
	});
}
