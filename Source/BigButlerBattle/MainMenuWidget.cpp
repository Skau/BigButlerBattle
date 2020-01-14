// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuWidget.h"
#include "Components/MenuAnchor.h"
#include "MainMenuPlayerWidget.h"

UMainMenuWidget::UMainMenuWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{}


void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

bool UMainMenuWidget::Initialize()
{
	bool initialized = Super::Initialize();
	
	PlayerWidget_0->SetPlayerName(FText::FromString("Player 1"));
	PlayerWidget_1->SetPlayerName(FText::FromString("Player 2"));
	PlayerWidget_2->SetPlayerName(FText::FromString("Player 3"));
	PlayerWidget_3->SetPlayerName(FText::FromString("Player 4"));

	return initialized;
}

