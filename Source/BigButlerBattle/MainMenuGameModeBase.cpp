// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuGameModeBase.h"
#include "UI/MainMenuWidget.h"
#include "UI/MainMenuPlayWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Player/PlayerCharacterController.h"
#include "UI/MainMenuWidget.h"
#include "UI/MainMenuPlayerWidget.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "ButlerGameInstance.h"

AMainMenuGameModeBase::AMainMenuGameModeBase()
{

}

void AMainMenuGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	TArray<int> IDsToCreate = { 0, 1, 2, 3 };

	for (auto& Controller : GetGameInstance()->GetLocalPlayers())
	{
		auto ID = Controller->GetControllerId();
		IDsToCreate.Remove(ID);
		Controllers.Add(Cast<APlayerCharacterController>(UGameplayStatics::GetPlayerControllerFromID(GetWorld(), ID)));
	}

	for (auto& ID : IDsToCreate)
	{
		Controllers.Add(Cast<APlayerCharacterController>(UGameplayStatics::CreatePlayer(GetWorld(), ID)));
	}


	Controllers.Sort([](APlayerCharacterController& p1, APlayerCharacterController& p2)
	{
			return UGameplayStatics::GetPlayerControllerID(&p1) < UGameplayStatics::GetPlayerControllerID(&p2);
	});

	if (!MainMenuWidgetClass || !MainMenuPlayWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("Subclasses not setup in Menu Gamemode!"));
		return;
	}

	Controllers[0]->GetLocalPlayer()->ViewportClient->SetForceDisableSplitscreen(true);
	MainMenuWidgetInstance = CreateWidget<UMainMenuWidget>(Controllers[0], MainMenuWidgetClass);
	MainMenuWidgetInstance->AddToViewport();
	MainMenuWidgetInstance->FocusWidget(Controllers[0]);

	MainMenuPlayWidgetInstance = CreateWidget<UMainMenuPlayWidget>(Controllers[0], MainMenuPlayWidgetClass);
	MainMenuPlayWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
	MainMenuPlayWidgetInstance->AddToViewport();
	
	MainMenuWidgetInstance->PlayWidget = MainMenuPlayWidgetInstance;
	MainMenuWidgetInstance->Controllers = Controllers;
	
	MainMenuPlayWidgetInstance->PlayerWidget_0->OnToggleJoinedGame.BindUObject(this, &AMainMenuGameModeBase::OnPlayerToggledJoinedGame);
	MainMenuPlayWidgetInstance->PlayerWidget_0->OnToggleReadyGame.BindUObject(this, &AMainMenuGameModeBase::OnPlayerToggledReady);
	MainMenuPlayWidgetInstance->PlayerWidget_1->OnToggleJoinedGame.BindUObject(this, &AMainMenuGameModeBase::OnPlayerToggledJoinedGame);
	MainMenuPlayWidgetInstance->PlayerWidget_1->OnToggleReadyGame.BindUObject(this, &AMainMenuGameModeBase::OnPlayerToggledReady);
	MainMenuPlayWidgetInstance->PlayerWidget_2->OnToggleJoinedGame.BindUObject(this, &AMainMenuGameModeBase::OnPlayerToggledJoinedGame);
	MainMenuPlayWidgetInstance->PlayerWidget_2->OnToggleReadyGame.BindUObject(this, &AMainMenuGameModeBase::OnPlayerToggledReady);
	MainMenuPlayWidgetInstance->PlayerWidget_3->OnToggleJoinedGame.BindUObject(this, &AMainMenuGameModeBase::OnPlayerToggledJoinedGame);
	MainMenuPlayWidgetInstance->PlayerWidget_3->OnToggleReadyGame.BindUObject(this, &AMainMenuGameModeBase::OnPlayerToggledReady);
}

void AMainMenuGameModeBase::OnPlayerToggledJoinedGame(bool Value, int ID)
{
	if (Value)
	{
		NumJoinedPlayers++;
		PlayerNotJoinedIDs.Remove(ID);
	}
	else
	{
		NumJoinedPlayers--;
		PlayerNotJoinedIDs.Add(ID);
	}

	if (MainMenuPlayWidgetInstance->GameTimerBox->Visibility != ESlateVisibility::Hidden)
	{
		MainMenuPlayWidgetInstance->GameTimerBox->SetVisibility(ESlateVisibility::Hidden);
	}
	GetWorldTimerManager().ClearTimer(HandleStartGame);
	ElapsedCountdownTime = 0.0f;
}

void AMainMenuGameModeBase::OnPlayerToggledReady(bool Value, int ID)
{
	Value ? NumReadiedPlayers++ : NumReadiedPlayers--;

	if (MainMenuPlayWidgetInstance->GameTimerBox->Visibility != ESlateVisibility::Hidden)
	{
		MainMenuPlayWidgetInstance->GameTimerBox->SetVisibility(ESlateVisibility::Hidden);
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
	if (MainMenuPlayWidgetInstance->GameTimerBox->Visibility != ESlateVisibility::Visible)
	{
		MainMenuPlayWidgetInstance->GameTimerBox->SetVisibility(ESlateVisibility::Visible);
	}

	ElapsedCountdownTime += GetWorldTimerManager().GetTimerElapsed(HandleStartGame);
	float TimeLeft = TimeUntilGameStart - ElapsedCountdownTime;
	if (TimeLeft <= 0.f)
	{
		Controllers[0]->GetLocalPlayer()->ViewportClient->SetForceDisableSplitscreen(false);
	
		for(auto& ID : PlayerNotJoinedIDs)
		{
			if (auto controller = UGameplayStatics::GetPlayerControllerFromID(GetWorld(), ID))
			{
				UGameplayStatics::RemovePlayer(controller, false);
			}
		}
		
		UGameplayStatics::OpenLevel(GetWorld(), LevelToPlay);
	}
	else
	{
		MainMenuPlayWidgetInstance->GameStartTime->SetText(FText::FromString(FString::FromInt(TimeLeft)));
	}
	
}
