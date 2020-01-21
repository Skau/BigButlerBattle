// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacterController.h"
#include "PlayerWidget.h"
#include "BaseUserWidget.h"
#include "PlayerCharacter.h"
#include "BigButlerBattleGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "DataTables.h"

APlayerCharacterController::APlayerCharacterController() {}

void APlayerCharacterController::BeginPlay()
{
	Super::BeginPlay();

	// Add in-game UI if we're actually in the game and not main menu
	PlayerCharacter = Cast<APlayerCharacter>(GetPawn());
	if (PlayerCharacter && PlayerWidgetType)
	{
		PlayerWidget = CreateWidget<UPlayerWidget>(this, PlayerWidgetType);
		PlayerWidget->AddToPlayerScreen();

		ButlerGameMode = Cast<ABigButlerBattleGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
		if (ButlerGameMode)
		{
			ButlerGameMode->OnTasksGenerated.BindUObject(this, &APlayerCharacterController::OnTasksGenerated);
		}
		// Delegates

		//PlayerCharacter->OnTaskObjectPickedUp.BindUObject(PlayerWidget, &UPlayerWidget::OnPlayerPickedUpObject);
	}
}

void APlayerCharacterController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void APlayerCharacterController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlayerCharacterController::SetupInputComponent()
{
	Super::SetupInputComponent();
	check(InputComponent != nullptr);

	InputComponent->BindAction("PauseGame", EInputEvent::IE_Pressed, this, &APlayerCharacterController::PauseGamePressed);
}

void APlayerCharacterController::PauseGamePressed()
{
	PauseGame.ExecuteIfBound(this);
}

void APlayerCharacterController::OnTasksGenerated(const TArray<FTask>& Tasks)
{
	UE_LOG(LogTemp, Warning, TEXT("Total tasks: %i"), Tasks.Num());

	if (!PlayerWidget)
		return;

	for (int i = 0; i < Tasks.Num(); ++i)
	{
		PlayerWidget->UpdateTaskSlotName(Tasks[i].ObjectName, i);
	}
}
