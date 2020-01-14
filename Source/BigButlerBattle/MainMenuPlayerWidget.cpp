// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuPlayerWidget.h"
#include "Components/TextBlock.h"
#include "Components/CheckBox.h"

UMainMenuPlayerWidget::UMainMenuPlayerWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{}

void UMainMenuPlayerWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

bool UMainMenuPlayerWidget::Initialize()
{
	return Super::Initialize();
}

void UMainMenuPlayerWidget::SetPlayerName(FText Text)
{
	PlayerNameText->SetText(Text);
}

ECheckBoxState UMainMenuPlayerWidget::GetPlayerReadyState()
{
	return CheckBox->GetCheckedState();
}

void UMainMenuPlayerWidget::SetPlayerReadyState(ECheckBoxState State)
{
	CheckBox->SetCheckedState(State);
}