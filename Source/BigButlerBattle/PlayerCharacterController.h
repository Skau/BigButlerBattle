// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PlayerCharacterController.generated.h"


DECLARE_DELEGATE_OneParam(PauseGameSignature, APlayerCharacterController*);

class APlayerCharacter;
class UPlayerWidget;
/**
 * 
 */
UCLASS()
class BIGBUTLERBATTLE_API APlayerCharacterController : public APlayerController
{
	GENERATED_BODY()

public:
	APlayerCharacterController();

	PauseGameSignature PauseGame;

protected:
	void BeginPlay() override;

	void OnPossess(APawn* InPawn) override;

	void Tick(float DeltaTime) override;

	void SetupInputComponent() override;

	UPROPERTY(BlueprintReadOnly)
	APlayerCharacter* ControlledPlayer = nullptr;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPlayerWidget> PlayerWidgetType;

	UPROPERTY(BlueprintReadOnly)
	UPlayerWidget* PlayerWidget;

private:
	void PauseGamePressed();
};
