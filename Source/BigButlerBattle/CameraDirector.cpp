// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraDirector.h"
#include "LevelSequencePlayer.h"
#include "Kismet/GameplayStatics.h"
#include "LevelSequenceActor.h"

ACameraDirector::ACameraDirector()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ACameraDirector::BeginPlay()
{
	Super::BeginPlay();

	PlaySequence();
}

void ACameraDirector::PlaySequence()
{
	if (!IsValid(SequencePlayer))
	{
		FMovieSceneSequencePlaybackSettings Settings;
		Settings.bAutoPlay = true;
		FMovieSceneSequenceLoopCount LoopCount;
		LoopCount.Value = 99;
		Settings.LoopCount = LoopCount;
		ALevelSequenceActor* SequenceActor;
		SequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(GetWorld(), MenuSequence, Settings, SequenceActor);
		return;
	}

	if (SequenceCamera)
	{
		auto PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		PC->SetViewTargetWithBlend(SequenceCamera, CharacterSelectCameraBlendTime, EViewTargetBlendFunction::VTBlend_Linear);
		SequencePlayer->Play();
	}
}

void ACameraDirector::BlendToCharacterSelectionCamera() const
{
	if (SequencePlayer->IsPlaying())
	{
		SequencePlayer->Stop();
	}

	auto PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	PC->SetViewTargetWithBlend(CharacterSelectionCamera, CharacterSelectCameraBlendTime, EViewTargetBlendFunction::VTBlend_Linear);
}

