// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerWidget.h"
#include "PlayerScoreWidget.h"
#include "Utils/btd.h"
#include "Player/PlayerCharacterController.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/Image.h"
#include "BigButlerBattleGameModeBase.h"
#include "Tasks/Task.h"
#include "Blueprint/WidgetTree.h"
#include "Kismet/GameplayStatics.h"


bool UPlayerWidget::Initialize()
{
	bool bInit = Super::Initialize();

	if (Text_Info)
	{
		Text_Info->SetVisibility(ESlateVisibility::Hidden);
	}

	auto GM = Cast<ABigButlerBattleGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GM)
	{
		InitializeScores(GM->GetControllers());
	}

	return bInit;
}

void UPlayerWidget::UpdateTimer(const FString& String)
{
	if (Text_Timer)
	{
		Text_Timer->SetText(FText::FromString(String));
	}
}

void UPlayerWidget::OnMainItemStateChanged(int ControllerID, bool bPickedUp)
{
	UpdateMessage("Player " + FString::FromInt(ControllerID + 1) + (bPickedUp ? " picked up" : " dropped") + " the main item!");
}

void UPlayerWidget::OnMainItemSet()
{
	UpdateMessage("The King demands a new item!");
}

void UPlayerWidget::InitializeScores(const TArray<APlayerCharacterController*>& Controllers)
{
	if (!PlayerScoreWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerScoreWidgetClass is null! Update PlayerWidget BP with correct subclass"));
		return;
	}

	FSlateChildSize SizeBoxSize;
	SizeBoxSize.SizeRule = ESlateSizeRule::Fill;
	SizeBoxSize.Value = 0.1f;

	FSlateChildSize OverlaySize;
	OverlaySize.SizeRule = ESlateSizeRule::Automatic;
	OverlaySize.Value = 1.0f;

	const FLinearColor ImageColor(1, 1, 1, 0.09f);

	for (int i = 0; i < Controllers.Num(); ++i)
	{
		const auto SizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		auto HorizontalSlot = ScoreBox->AddChildToHorizontalBox(SizeBox);
		HorizontalSlot->SetSize(SizeBoxSize);

		auto Overlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());

		auto Image = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
		Image->SetColorAndOpacity(ImageColor);

		auto OverlaySlot = Overlay->AddChildToOverlay(Image);
		OverlaySlot->HorizontalAlignment = EHorizontalAlignment::HAlign_Fill;

		auto PlayerScoreWidget = CreateWidget<UPlayerScoreWidget>(this, PlayerScoreWidgetClass);
		PlayerScoreWidget->Initialize();

		auto ID = UGameplayStatics::GetPlayerControllerID(Controllers[i]);

		PlayerScoreWidget->SetPlayerName("Player " + FString::FromInt(ID + 1) + ": ");

		PlayerScores.Add(ID, PlayerScoreWidget);

		OverlaySlot = Overlay->AddChildToOverlay(PlayerScoreWidget);
		OverlaySlot->HorizontalAlignment = EHorizontalAlignment::HAlign_Fill;

		HorizontalSlot = ScoreBox->AddChildToHorizontalBox(Overlay);
		HorizontalSlot->SetSize(OverlaySize);
		HorizontalSlot->HorizontalAlignment = EHorizontalAlignment::HAlign_Fill;
	}

	const auto SizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	auto HorizontalSlot = ScoreBox->AddChildToHorizontalBox(SizeBox);
	HorizontalSlot->SetSize(SizeBoxSize);
}

void UPlayerWidget::UpdateScore(int ControllerID, int NewScore)
{
	if (PlayerScores.Find(ControllerID))
	{
		PlayerScores[ControllerID]->UpdateScore(NewScore);
	}
}

void UPlayerWidget::UpdateMessage(const FString& Message, const float Duration)
{
	if (Message.IsEmpty() || !Text_Info)
		return;

	Text_Info->SetText(FText::FromString(Message));
	Text_Info->SetVisibility(ESlateVisibility::Visible);

	btd::Delay(this, Duration, [=]() {
		Text_Info->SetVisibility(ESlateVisibility::Hidden);
	});
}

void UPlayerWidget::ShowKeybinds()
{
	if (Image_Keybinds)
	{
		Image_Keybinds->SetVisibility(ESlateVisibility::Visible);
	}
}

void UPlayerWidget::HideKeybinds()
{
	if (Image_Keybinds)
	{
		Image_Keybinds->SetVisibility(ESlateVisibility::Hidden);
	}
}