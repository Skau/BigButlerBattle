// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuWidget.h"
#include "MainMenuPlayWidget.h"
#include "MainMenuPlayerWidget.h"
#include "MainMenuOptionsWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

UMainMenuWidget::UMainMenuWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{}


void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

bool UMainMenuWidget::Initialize()
{
	const bool bInit = Super::Initialize();

	Button_Play->OnClicked.AddDynamic(this, &UMainMenuWidget::OnPlayPressed);
	Buttons.Add(Button_Play);

	Button_Options->OnClicked.AddDynamic(this, &UMainMenuWidget::OnOptionsPressed);
	Buttons.Add(Button_Options);

	Button_Quit->OnClicked.AddDynamic(this, &UMainMenuWidget::OnQuitPressed);
	Buttons.Add(Button_Quit);

	DefaultWidgetToFocus = Button_Play;

	return bInit;
}

void UMainMenuWidget::OnPlayPressed()
{
	UE_LOG(LogTemp, Warning, TEXT("Play pressed"));
	if (!IsValid(PlayWidget))
	{
		UE_LOG(LogTemp, Error, TEXT("PlayWidget is not valid!"));
		return;
	}

	SetVisibility(ESlateVisibility::Hidden);
	if(PlayWidget->MainMenuWidget != this)
		PlayWidget->MainMenuWidget = this;

	PlayWidget->SetVisibility(ESlateVisibility::Visible);

	if(auto Player = UGameplayStatics::GetPlayerController(GetWorld(), 0))
		PlayWidget->PlayerWidget_0->FocusWidget(Player, PlayWidget->PlayerWidget_0->Button_Join);

	if (auto Player = UGameplayStatics::GetPlayerController(GetWorld(), 1))
		PlayWidget->PlayerWidget_1->FocusWidget(Player, PlayWidget->PlayerWidget_1->Button_Join);

	if (auto Player = UGameplayStatics::GetPlayerController(GetWorld(), 2))
		PlayWidget->PlayerWidget_2->FocusWidget(Player, PlayWidget->PlayerWidget_2->Button_Join);

	if (auto Player = UGameplayStatics::GetPlayerController(GetWorld(), 3))
		PlayWidget->PlayerWidget_3->FocusWidget(Player, PlayWidget->PlayerWidget_3->Button_Join);

	DefaultWidgetToFocus = Button_Play;
}

void UMainMenuWidget::OnOptionsPressed()
{
	SetVisibility(ESlateVisibility::Hidden);
	if (OptionsWidget->MainMenuWidget != this)
		OptionsWidget->MainMenuWidget = this;

	OptionsWidget->SetVisibility(ESlateVisibility::Visible);

	OptionsWidget->FocusWidget(OwningPlayerController);

	// Open options
	
	DefaultWidgetToFocus = Button_Options;

}

void UMainMenuWidget::OnQuitPressed()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Quit, false);
}
