// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuGameModeBase.h"
#include "UI/MainMenuWidget.h"
#include "UI/MainMenuPlayWidget.h"
#include "UI/MainMenuOptionsWidget.h"
#include "Kismet/GameplayStatics.h"
#include "UI/MainMenuWidget.h"
#include "UI/MainMenuPlayerWidget.h"
#include "UI/HelpWidget.h"
#include "TimerManager.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "ButlerGameInstance.h"

void AMainMenuGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	TArray<int> IDsToCreate = { 0, 1, 2, 3 };

	UE_LOG(LogTemp, Warning, TEXT("Adding all local players already existing."));
	for (auto& Controller : GetGameInstance()->GetLocalPlayers())
	{
		auto ID = Controller->GetControllerId();
		UE_LOG(LogTemp, Warning, TEXT("Added local player with ID %i"), ID);
		IDsToCreate.Remove(ID);
		Controllers.Add(UGameplayStatics::GetPlayerControllerFromID(GetWorld(), ID));
	}

	UE_LOG(LogTemp, Warning, TEXT("Creating the remaining players.."));
	for (auto& ID : IDsToCreate)
	{
		UE_LOG(LogTemp, Warning, TEXT("Creating player with ID %i"), ID);
		Controllers.Add(UGameplayStatics::CreatePlayer(GetWorld(), ID));
	}

	Controllers.Sort([](APlayerController& p1, APlayerController& p2)
	{
			UE_LOG(LogTemp, Warning, TEXT("Controller sort"));
			return UGameplayStatics::GetPlayerControllerID(&p1) < UGameplayStatics::GetPlayerControllerID(&p2);
	});

	UE_LOG(LogTemp, Warning, TEXT("Number of controllers: %i"), Controllers.Num());
	for (int i = 0; i < Controllers.Num(); ++i)
	{
		if (IsValid(Controllers[i]))
		{
			UE_LOG(LogTemp, Warning, TEXT("Controllers[%i] is valid. ID: %i"), i, UGameplayStatics::GetPlayerControllerID(Controllers[i]));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Controllers[%i] is not valid."));
		}
	}

	if (!MainMenuWidgetClass || !MainMenuPlayWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("Subclasses not setup in Menu Gamemode!"));
		return;
	}

	if (!IsValid(Controllers[0]))
	{
		UE_LOG(LogTemp, Warning, TEXT("Controllers[0] is not valid. Not playable."));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Disabling split screen"));
	Controllers[0]->GetLocalPlayer()->ViewportClient->SetForceDisableSplitscreen(true);

	UE_LOG(LogTemp, Warning, TEXT("Creating main menu widget instance"));
	MainMenuWidgetInstance = CreateWidget<UMainMenuWidget>(Controllers[0], MainMenuWidgetClass);
	MainMenuWidgetInstance->AddToViewport();
	MainMenuWidgetInstance->FocusWidget(Controllers[0]);

	UE_LOG(LogTemp, Warning, TEXT("Creating main menu play widget instance"));
	MainMenuPlayWidgetInstance = CreateWidget<UMainMenuPlayWidget>(Controllers[0], MainMenuPlayWidgetClass);
	MainMenuPlayWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
	MainMenuPlayWidgetInstance->AddToViewport();

	UE_LOG(LogTemp, Warning, TEXT("Updating play widget on menu widget instance"));
	MainMenuWidgetInstance->PlayWidget = MainMenuPlayWidgetInstance;

	UE_LOG(LogTemp, Warning, TEXT("Creating help widget instance"));
	HelpWidgetInstance = CreateWidget<UHelpWidget>(Controllers[0], HelpWidgetClass);
	HelpWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
	HelpWidgetInstance->AddToViewport();

	UE_LOG(LogTemp, Warning, TEXT("Updating help widget on menu widget instance"));
	MainMenuWidgetInstance->HelpWidget = HelpWidgetInstance;

	UE_LOG(LogTemp, Warning, TEXT("Creating main menu options widget instance"));
	MainMenuOptionsWidgetInstance = CreateWidget<UMainMenuOptionsWidget>(Controllers[0], MainMenuOptionsWidgetClass);
	MainMenuOptionsWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
	MainMenuOptionsWidgetInstance->AddToViewport();
	
	UE_LOG(LogTemp, Warning, TEXT("Updating options widget on menu widget instance"));
	MainMenuWidgetInstance->OptionsWidget = MainMenuOptionsWidgetInstance;
	
	UE_LOG(LogTemp, Warning, TEXT("Binding delegates.."));
	MainMenuPlayWidgetInstance->PlayerWidget_0->OnToggleJoinedGame.BindUObject(this, &AMainMenuGameModeBase::OnPlayerToggledJoinedGame);
	MainMenuPlayWidgetInstance->PlayerWidget_0->OnToggleReadyGame.BindUObject(this, &AMainMenuGameModeBase::OnPlayerToggledReady);
	MainMenuPlayWidgetInstance->PlayerWidget_1->OnToggleJoinedGame.BindUObject(this, &AMainMenuGameModeBase::OnPlayerToggledJoinedGame);
	MainMenuPlayWidgetInstance->PlayerWidget_1->OnToggleReadyGame.BindUObject(this, &AMainMenuGameModeBase::OnPlayerToggledReady);
	MainMenuPlayWidgetInstance->PlayerWidget_2->OnToggleJoinedGame.BindUObject(this, &AMainMenuGameModeBase::OnPlayerToggledJoinedGame);
	MainMenuPlayWidgetInstance->PlayerWidget_2->OnToggleReadyGame.BindUObject(this, &AMainMenuGameModeBase::OnPlayerToggledReady);
	MainMenuPlayWidgetInstance->PlayerWidget_3->OnToggleJoinedGame.BindUObject(this, &AMainMenuGameModeBase::OnPlayerToggledJoinedGame);
	MainMenuPlayWidgetInstance->PlayerWidget_3->OnToggleReadyGame.BindUObject(this, &AMainMenuGameModeBase::OnPlayerToggledReady);
	UE_LOG(LogTemp, Warning, TEXT("Binding delegates done."));
}

void AMainMenuGameModeBase::OnPlayerToggledJoinedGame(const bool Value, const int ID)
{
	UE_LOG(LogTemp, Warning, TEXT("OnPlayerToggledJoinedGame, Joined: %i, Controller ID %i"), Value, ID);

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

	EndCountdown();
}

void AMainMenuGameModeBase::OnPlayerToggledReady(const bool Value, const int ID)
{
	UE_LOG(LogTemp, Warning, TEXT("OnPlayerToggledReady, Ready: %i, Controller ID %i"), Value, ID);

	Value ? NumReadiedPlayers++ : NumReadiedPlayers--;

	EndCountdown();

	if (NumJoinedPlayers >= MinimumPlayersToStartGame && NumReadiedPlayers == NumJoinedPlayers)
	{
		StartCountdown();
	}
}


void AMainMenuGameModeBase::StartCountdown()
{
	UE_LOG(LogTemp, Warning, TEXT("Starting game countdown.."));

	if (MainMenuPlayWidgetInstance->GameTimerBox->Visibility != ESlateVisibility::Visible)
	{
		MainMenuPlayWidgetInstance->GameTimerBox->SetVisibility(ESlateVisibility::Visible);

		GetWorldTimerManager().SetTimer(HandleStartGame, this, &AMainMenuGameModeBase::Countdown, .1f, true, 0.0f);
	}
}

void AMainMenuGameModeBase::Countdown()
{
	ElapsedCountdownTime += GetWorldTimerManager().GetTimerElapsed(HandleStartGame);
	const float TimeLeft = TimeUntilGameStart - ElapsedCountdownTime;

	if (TimeLeft <= 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Starting game..."));
		Controllers[0]->GetLocalPlayer()->ViewportClient->SetForceDisableSplitscreen(false);
	
		for(auto& ID : PlayerNotJoinedIDs)
		{
			if (const auto controller = UGameplayStatics::GetPlayerControllerFromID(GetWorld(), ID))
			{
				UE_LOG(LogTemp, Warning, TEXT("Removing Controller ID %i, because it never joined."), ID);
				UGameplayStatics::RemovePlayer(controller, true);
			}
		}
		Cast<UButlerGameInstance>(GetGameInstance())->LevelChanged(false);
		UGameplayStatics::OpenLevel(GetWorld(), LevelToPlay);
	}
	else
	{
		MainMenuPlayWidgetInstance->GameStartTime->SetText(FText::FromString(FString::FromInt(TimeLeft)));
	}
}

void AMainMenuGameModeBase::EndCountdown()
{
	if (GetWorldTimerManager().IsTimerActive(HandleStartGame))
	{
		UE_LOG(LogTemp, Warning, TEXT("Ending game countdown.."));

		GetWorldTimerManager().ClearTimer(HandleStartGame);
		ElapsedCountdownTime = 0.0f;

		if (MainMenuPlayWidgetInstance->GameTimerBox->Visibility != ESlateVisibility::Hidden)
		{
			MainMenuPlayWidgetInstance->GameTimerBox->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}
