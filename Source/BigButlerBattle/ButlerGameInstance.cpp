// Fill out your copyright notice in the Description page of Project Settings.


#include "ButlerGameInstance.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundClass.h"
#include "Kismet/GameplayStatics.h"
#include "AudioDevice.h"
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
		Seed = Stream.GetCurrentSeed();
	}

	PlayerOptions.AddDefaulted(4);

	btd::Delay(this, 0.5f, [=]()
	{
		auto LevelName = UGameplayStatics::GetCurrentLevelName(GetWorld());
		auto CurrentlyMainMenu = LevelName == "MainMenu";
		if (BackgroundMusic)
		{
			AudioComponent = UGameplayStatics::CreateSound2D(GetWorld(), BackgroundMusic, 1.0f, 1.0f, 0.0f, SoundConcurrency, true, true);
			AudioComponent->Play();
		}
		else
			UE_LOG(LogTemp, Error, TEXT("GameInstance: %s sound not set!"), *LevelName)
	});
}

void UButlerGameInstance::SetMainSoundVolume(float Value)
{
	if (MainSoundClass)
	{
		MainSoundClass->Properties.Volume = Value;
	}
}

void UButlerGameInstance::SetBackgroundSoundVolume(float Value)
{
	if (BackgroundSoundClass)
	{
		BackgroundSoundClass->Properties.Volume = Value;
	}
}

void UButlerGameInstance::SetSoundEffectsSoundVolume(float Value)
{
	if (SoundEffectsSoundClass)
	{
		SoundEffectsSoundClass->Properties.Volume = Value;
	}
}

void UButlerGameInstance::LevelChanged(bool bNewLevelIsMainMenu)
{
	FadeBetweenMusic(bNewLevelIsMainMenu);
}

void UButlerGameInstance::Shutdown()
{
	if (AudioComponent && AudioComponent->IsPlaying())
		AudioComponent->Stop();
}

void UButlerGameInstance::FadeBetweenMusic(bool bNewLevelIsMainMenu)
{
	if (!AudioComponent)
		return;

	Fade = (float)bNewLevelIsMainMenu;

	if(!bNewLevelIsMainMenu)
		AudioComponent->SetIntParameter(FName{ "InGameSoundIndex" }, FMath::RandRange(0, 2));

	btd::Repeat(this, 0.5f, 2, [&, bNewLevelIsMainMenu]()
	{
		Fade += bNewLevelIsMainMenu ? -0.5f : 0.5f;
		AudioComponent->SetFloatParameter(FName{ "Fade" }, Fade);
	});
}
