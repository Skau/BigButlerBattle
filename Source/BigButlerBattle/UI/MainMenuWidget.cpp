// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuWidget.h"
#include "MainMenuPlayWidget.h"
#include "MainMenuPlayerWidget.h"
#include "Components/Button.h"
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
	bool initialized = Super::Initialize();

	Button_Play->OnClicked.AddDynamic(this, &UMainMenuWidget::OnPlayPressed);
	Buttons.Add(Button_Play);

	Button_Options->OnClicked.AddDynamic(this, &UMainMenuWidget::OnOptionsPressed);
	Buttons.Add(Button_Options);

	Button_Quit->OnClicked.AddDynamic(this, &UMainMenuWidget::OnQuitPressed);
	Buttons.Add(Button_Quit);

	DefaultWidgetToFocus = Button_Play;

	return initialized;
}

void UMainMenuWidget::OnPlayPressed()
{
	SetVisibility(ESlateVisibility::Hidden);
	if(PlayWidget->MainMenuWidget != this)
		PlayWidget->MainMenuWidget = this;

	PlayWidget->SetVisibility(ESlateVisibility::Visible);
	
	PlayWidget->PlayerWidget_0->FocusWidget(Controllers[0], PlayWidget->PlayerWidget_0->Button_Join);
	PlayWidget->PlayerWidget_1->FocusWidget(Controllers[1], PlayWidget->PlayerWidget_1->Button_Join);
	PlayWidget->PlayerWidget_2->FocusWidget(Controllers[2], PlayWidget->PlayerWidget_2->Button_Join);
	PlayWidget->PlayerWidget_3->FocusWidget(Controllers[3], PlayWidget->PlayerWidget_3->Button_Join);

	DefaultWidgetToFocus = Button_Play;
}

void UMainMenuWidget::OnOptionsPressed()
{
	//SetVisibility(ESlateVisibility::Hidden);


	// Open options
	
	DefaultWidgetToFocus = Button_Options;

}

void UMainMenuWidget::OnQuitPressed()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Quit, false);
}
