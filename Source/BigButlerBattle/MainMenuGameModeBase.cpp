// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuGameModeBase.h"
#include "MainMenuWidget.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerCharacterController.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/KismetMathLibrary.h"
#include "MenuPlayerController.h"
#include "TimerManager.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "ButlerGameInstance.h"

AMainMenuGameModeBase::AMainMenuGameModeBase()
{

}

void AMainMenuGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	Instance = Cast<UButlerGameInstance>(GetGameInstance());

	auto Controller0 = Cast<AMenuPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	Controller0->GetLocalPlayer()->ViewportClient->SetForceDisableSplitscreen(true);
	Controllers.Add(Controller0);

	int i = 0;
	while (i < 3)
	{
		auto Controller = Cast<AMenuPlayerController>(UGameplayStatics::CreatePlayer(GetWorld()));
		Controller->ID = i + 1;
		Controllers.Add(Controller);
		++i;
	}

	MainMenuWidgetInstance = Cast<UMainMenuWidget>(CreateWidget(Controller0, MainMenuWidgetClass));
	MainMenuWidgetInstance->AddToViewport();

	Controllers[0]->SetPlayerWidgets(MainMenuWidgetInstance->SwitcherPlayer_0, MainMenuWidgetInstance->PlayerWidget_0);
	Controllers[1]->SetPlayerWidgets(MainMenuWidgetInstance->SwitcherPlayer_1, MainMenuWidgetInstance->PlayerWidget_1);
	Controllers[2]->SetPlayerWidgets(MainMenuWidgetInstance->SwitcherPlayer_2, MainMenuWidgetInstance->PlayerWidget_2);
	Controllers[3]->SetPlayerWidgets(MainMenuWidgetInstance->SwitcherPlayer_3, MainMenuWidgetInstance->PlayerWidget_3);

	for (int i = 0; i < Controllers.Num(); ++i)
	{
		Controllers[i]->OnToggleJoinedGame.BindUObject(this, &AMainMenuGameModeBase::OnPlayerToggledJoinedGame);
		Controllers[i]->OnToggleReadyGame.BindUObject(this, &AMainMenuGameModeBase::OnPlayerToggledReady);
	}
}

void AMainMenuGameModeBase::OnPlayerToggledJoinedGame(bool Value, int ID)
{
	if (Value)
	{
		NumJoinedPlayers++;
		Instance->PlayerIDs.Add(ID);
	}
	else
	{
		NumJoinedPlayers--;
		Instance->PlayerIDs.Remove(ID);
	}

	if (MainMenuWidgetInstance->GameTimerBox->Visibility != ESlateVisibility::Hidden)
	{
		MainMenuWidgetInstance->GameTimerBox->SetVisibility(ESlateVisibility::Hidden);
	}
	GetWorldTimerManager().ClearTimer(HandleStartGame);
	ElapsedCountdownTime = 0.0f;
}

void AMainMenuGameModeBase::OnPlayerToggledReady(bool Value, int ID)
{
	Value ? NumReadiedPlayers++ : NumReadiedPlayers--;

	if (MainMenuWidgetInstance->GameTimerBox->Visibility != ESlateVisibility::Hidden)
	{
		MainMenuWidgetInstance->GameTimerBox->SetVisibility(ESlateVisibility::Hidden);
	}
	GetWorldTimerManager().ClearTimer(HandleStartGame);
	ElapsedCountdownTime = 0.0f;
	if (NumJoinedPlayers >= MinimumPlayersToStartGame && NumReadiedPlayers == NumJoinedPlayers)
	{
		GetWorldTimerManager().SetTimer(HandleStartGame, this, &AMainMenuGameModeBase::GameStartCountdown, 0.1f, true, 0.0f);
	}
}

void AMainMenuGameModeBase::GameStartCountdown()
{
	if (MainMenuWidgetInstance->GameTimerBox->Visibility != ESlateVisibility::Visible)
	{
		MainMenuWidgetInstance->GameTimerBox->SetVisibility(ESlateVisibility::Visible);
	}

	ElapsedCountdownTime += GetWorldTimerManager().GetTimerElapsed(HandleStartGame);
	float TimeLeft = TimeUntilGameStart - ElapsedCountdownTime;
	if (TimeLeft <= 0.f)
	{
		Controllers[0]->GetLocalPlayer()->ViewportClient->SetForceDisableSplitscreen(false);
		UGameplayStatics::OpenLevel(GetWorld(), LevelToPlay);
	}
	else
	{
		MainMenuWidgetInstance->GameStartTime->SetText(FText::FromString(FString::FromInt(TimeLeft)));
	}
	
}
