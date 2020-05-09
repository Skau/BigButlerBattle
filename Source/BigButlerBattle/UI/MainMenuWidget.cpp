// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuWidget.h"
#include "MainMenuPlayWidget.h"
#include "MainMenuPlayerWidget.h"
#include "HelpWidget.h"
#include "MainMenuOptionsWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "CameraDirector.h"
#include "Utils/btd.h"
#include "Animation/UMGSequencePlayer.h"

UMainMenuWidget::UMainMenuWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ButlerTransforms = {
		{ FRotator{0.f, 60.f,  0.f}, FVector{-368.f,-5108.f, 304.f} },
		{ FRotator{0.f, 85.f,  0.f}, FVector{-148.f,-5108.f, 304.f} },
		{ FRotator{0.f, 95.f,  0.f}, FVector{ 91.f, -5108.f, 304.f} },
		{ FRotator{0.f, 120.f, 0.f}, FVector{ 306.f,-5108.f, 304.f} }
	};
}

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

bool UMainMenuWidget::Initialize()
{
	const bool bInit = Super::Initialize();

	if (Button_Play)
	{
		Button_Play->OnClicked.AddDynamic(this, &UMainMenuWidget::OnPlayPressed);
		Buttons.Add(Button_Play);
	}

	if (Button_Help)
	{
		Button_Help->OnClicked.AddDynamic(this, &UMainMenuWidget::OnHelpPressed);
		Buttons.Add(Button_Help);
	}

	if (Button_Options)
	{
		Button_Options->OnClicked.AddDynamic(this, &UMainMenuWidget::OnOptionsPressed);
		Buttons.Add(Button_Options);
	}

	if (Button_Quit)
	{
		Button_Quit->OnClicked.AddDynamic(this, &UMainMenuWidget::OnQuitPressed);
		Buttons.Add(Button_Quit);
	}

	CameraDirector = Cast<ACameraDirector>(UGameplayStatics::GetActorOfClass(GetWorld(), ACameraDirector::StaticClass()));

	DefaultWidgetToFocus = Button_Play;
	
	UProperty* prop = GetClass()->PropertyLink;
	while (prop != nullptr)
	{
		if (prop->GetClass() == UObjectProperty::StaticClass())
		{
			UObjectProperty* objectProp = Cast<UObjectProperty>(prop);
			
			if (objectProp->PropertyClass == UWidgetAnimation::StaticClass())
			{
				UObject* object = objectProp->GetObjectPropertyValue_InContainer(this);

				Animation = Cast<UWidgetAnimation>(object);
				if (Animation != nullptr)
					break;
			}
		}
		prop = prop->PropertyLinkNext;
	}

	PlayAnimation();

	return bInit;
}

void UMainMenuWidget::PlayAnimation()
{
	PlayAnimationForward(Animation);
}

void UMainMenuWidget::OnPlayPressed()
{
	UE_LOG(LogTemp, Warning, TEXT("Play pressed"));
	if (!IsValid(PlayWidget))
	{
		UE_LOG(LogTemp, Error, TEXT("PlayWidget is not valid!"));
		return;
	}


	if(PlayWidget->MainMenuWidget != this)
		PlayWidget->MainMenuWidget = this;

	if (CameraDirector)
	{
		CameraDirector->BlendToCharacterSelectionCamera();
	}

	auto Player = PlayAnimationReverse(Animation);
	btd::Delay(this, Animation->GetEndTime() - Animation->GetStartTime(), [this]()
	{
		SetVisibility(ESlateVisibility::Hidden);
	});

	btd::Delay(this, CameraDirector->CharacterSelectCameraBlendTime, [=]()
	{
		PlayWidget->SetVisibility(ESlateVisibility::Visible);

		if (auto Player = UGameplayStatics::GetPlayerController(GetWorld(), 0))
		{
			PlayWidget->PlayerWidget_0->FocusWidget(Player, PlayWidget->PlayerWidget_0->Button_Join);
			PlayWidget->PlayerWidget_0->SpawnCharacter(ButlerTransforms[0]);
		}


		if (auto Player = UGameplayStatics::GetPlayerController(GetWorld(), 1))
		{
			PlayWidget->PlayerWidget_1->FocusWidget(Player, PlayWidget->PlayerWidget_1->Button_Join);
			PlayWidget->PlayerWidget_1->SpawnCharacter(ButlerTransforms[1]);
		}


		if (auto Player = UGameplayStatics::GetPlayerController(GetWorld(), 2))
		{
			PlayWidget->PlayerWidget_2->FocusWidget(Player, PlayWidget->PlayerWidget_2->Button_Join);
			PlayWidget->PlayerWidget_2->SpawnCharacter(ButlerTransforms[2]);
		}


		if (auto Player = UGameplayStatics::GetPlayerController(GetWorld(), 3))
		{
			PlayWidget->PlayerWidget_3->FocusWidget(Player, PlayWidget->PlayerWidget_3->Button_Join);
			PlayWidget->PlayerWidget_3->SpawnCharacter(ButlerTransforms[3]);
		}


		DefaultWidgetToFocus = Button_Play;
	});
}

void UMainMenuWidget::OnHelpPressed()
{
	SetVisibility(ESlateVisibility::Hidden);
	if (HelpWidget->MainMenuWidget != this)
		HelpWidget->MainMenuWidget = this;

	HelpWidget->SetVisibility(ESlateVisibility::Visible);

	HelpWidget->FocusWidget(OwningPlayerController);

	DefaultWidgetToFocus = Button_Help;
}

void UMainMenuWidget::OnOptionsPressed()
{
	SetVisibility(ESlateVisibility::Hidden);
	if (OptionsWidget->MainMenuWidget != this)
		OptionsWidget->MainMenuWidget = this;

	OptionsWidget->SetVisibility(ESlateVisibility::Visible);

	OptionsWidget->FocusWidget(OwningPlayerController);

	DefaultWidgetToFocus = Button_Options;
}

void UMainMenuWidget::OnQuitPressed()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Quit, false);
}