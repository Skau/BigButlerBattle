// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CameraDirector.generated.h"

class ULevelSequence;
class ALevelSequenceActor;
class ULevelSequencePlayer;

UCLASS()
class BIGBUTLERBATTLE_API ACameraDirector : public AActor
{
	GENERATED_BODY()
	
public:	
	ACameraDirector();

	void PlaySequence();

	void BlendToCharacterSelectionCamera();

	UPROPERTY(EditAnywhere)
	float CharacterSelectCameraBlendTime = 0.5f;

protected:
	void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	ULevelSequence* MenuSequence;

	UPROPERTY(EditAnywhere)
	AActor* SequenceCamera;

	UPROPERTY(EditAnywhere)
	AActor* CharacterSelectionCamera;

	UPROPERTY()
	ULevelSequencePlayer* SequencePlayer;

};
