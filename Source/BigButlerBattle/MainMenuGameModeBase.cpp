// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuGameModeBase.h"
#include "UI/MainMenuWidget.h"
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

	Controllers[0]->GetLocalPlayer()->ViewportClient->SetForceDisableSplitscreen(true);
	MainMenuWidgetInstance = CreateWidget<UMainMenuWidget>(Controllers[0], MainMenuWidgetClass);
	MainMenuWidgetInstance->AddToViewport();

	MainMenuWidgetInstance->PlayerWidget_0->FocusWidget(Controllers[0]);
	MainMenuWidgetInstance->PlayerWidget_0->OnToggleJoinedGame.BindUObject(this, &AMainMenuGameModeBase::OnPlayerToggledJoinedGame);
	MainMenuWidgetInstance->PlayerWidget_0->OnToggleReadyGame.BindUObject(this, &AMainMenuGameModeBase::OnPlayerToggledReady);

	MainMenuWidgetInstance->PlayerWidget_1->FocusWidget(Controllers[1]);
	MainMenuWidgetInstance->PlayerWidget_1->OnToggleJoinedGame.BindUObject(this, &AMainMenuGameModeBase::OnPlayerToggledJoinedGame);
	MainMenuWidgetInstance->PlayerWidget_1->OnToggleReadyGame.BindUObject(this, &AMainMenuGameModeBase::OnPlayerToggledReady);

	MainMenuWidgetInstance->PlayerWidget_2->FocusWidget(Controllers[2]);
	MainMenuWidgetInstance->PlayerWidget_2->OnToggleJoinedGame.BindUObject(this, &AMainMenuGameModeBase::OnPlayerToggledJoinedGame);
	MainMenuWidgetInstance->PlayerWidget_2->OnToggleReadyGame.BindUObject(this, &AMainMenuGameModeBase::OnPlayerToggledReady);

	MainMenuWidgetInstance->PlayerWidget_3->FocusWidget(Controllers[3]);
	MainMenuWidgetInstance->PlayerWidget_3->OnToggleJoinedGame.BindUObject(this, &AMainMenuGameModeBase::OnPlayerToggledJoinedGame);
	MainMenuWidgetInstance->PlayerWidget_3->OnToggleReadyGame.BindUObject(this, &AMainMenuGameModeBase::OnPlayerToggledReady);
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
		MainMenuWidgetInstance->GameStartTime->SetText(FText::FromString(FString::FromInt(TimeLeft)));
	}
	
}
