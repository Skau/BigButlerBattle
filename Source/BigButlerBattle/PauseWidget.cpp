// Fill out your copyright notice in the Description page of Project Settings.


#include "PauseWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "PlayerCharacterController.h"
#include "Kismet/GameplayStatics.h"

bool UPauseWidget::Initialize()
{
	bool bInitialized = Super::Initialize();

	SetVisibility(ESlateVisibility::Hidden);

	Button_Continue->OnClicked.AddDynamic(this, &UPauseWidget::OnContinuePressed);
	Button_Quit->OnClicked.AddDynamic(this, &UPauseWidget::OnQuitPressed);

	Buttons.Add(Button_Continue);
	Buttons.Add(Button_Quit);

	DefaultWidgetToFocus = Button_Continue;

	return bInitialized;
}

void UPauseWidget::OnPlayerCharacterControllerSet()
{
	auto ID = UGameplayStatics::GetPlayerControllerID(GetOwningPlayerCharacterController());
	PlayerText->SetText(FText::FromString("By Player " + FString::FromInt(ID + 1)));
}

void UPauseWidget::OnContinuePressed()
{
	ContinueGame.ExecuteIfBound(OwningCharacterController);
}

void UPauseWidget::OnQuitPressed()
{
	QuitGame.ExecuteIfBound();
}
