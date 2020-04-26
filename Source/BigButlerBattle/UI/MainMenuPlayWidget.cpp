// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuPlayWidget.h"
#include "MainMenuWidget.h"
#include "MainMenuPlayerWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "CameraDirector.h"

UMainMenuPlayWidget::UMainMenuPlayWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{}

void UMainMenuPlayWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

bool UMainMenuPlayWidget::Initialize()
{
	const bool bInit = Super::Initialize();

	PlayerWidget_0->MainPlayWidget = this;
	PlayerWidget_1->MainPlayWidget = this;
	PlayerWidget_2->MainPlayWidget = this;
	PlayerWidget_3->MainPlayWidget = this;

	return bInit;
}

void UMainMenuPlayWidget::BackToMainMenu()
{
	UE_LOG(LogTemp, Warning, TEXT("Back to main menu"));
	if (!IsValid(MainMenuWidget))
	{
		UE_LOG(LogTemp, Warning, TEXT("MainMenuWidget not valid!"));
		return;
	}

	SetVisibility(ESlateVisibility::Hidden);
	MainMenuWidget->SetVisibility(ESlateVisibility::Visible);
	MainMenuWidget->FocusWidget(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (MainMenuWidget->CameraDirector)
	{
		MainMenuWidget->CameraDirector->PlaySequence();
	}
}
