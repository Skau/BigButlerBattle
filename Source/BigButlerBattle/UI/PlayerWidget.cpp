// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerWidget.h"
#include "PlayerScoreWidget.h"
#include "Utils/btd.h"
#include "Player/PlayerCharacterController.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/Image.h"
#include "BigButlerBattleGameModeBase.h"
#include "Tasks/Task.h"
#include "Blueprint/WidgetTree.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Tasks/TaskObject.h"
#include <Runtime\UMG\Public\Blueprint\SlateBlueprintLibrary.h>

bool UPlayerWidget::Initialize()
{
	bool bInit = Super::Initialize();

	auto GM = Cast<ABigButlerBattleGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GM)
	{
		InitializeScores(GM->GetControllers());
	}

	return bInit;
}

void UPlayerWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if(MainItemIcon && MainItem && !bHasMainItem)
	{
		FVector2D Pos;
		auto PC = UGameplayStatics::GetPlayerControllerFromID(GetWorld(), ID);
		if(PC->ProjectWorldLocationToScreen(MainItem->GetActorLocation(), Pos, true))
		{
			
			FVector2D ScreenPosition(FMath::RoundToInt(Pos.X), FMath::RoundToInt(Pos.Y));
			FVector2D ViewportPosition2D;
			USlateBlueprintLibrary::ScreenToViewport(PC, ScreenPosition, ViewportPosition2D);
			
			auto Size = GetDesiredSize();

			ViewportPosition2D.X = FMath::Clamp(ViewportPosition2D.X, 0.f, Size.X);
			ViewportPosition2D.Y = FMath::Clamp(ViewportPosition2D.Y, 0.f, Size.Y);

			// Add lerp maybe
			MainItemIcon->SetRenderTranslation(ViewportPosition2D);
		}
	}
}

void UPlayerWidget::UpdateTimer(const FString& String)
{
	if (Text_Timer)
	{
		Text_Timer->SetText(FText::FromString(String));
	}
}

void UPlayerWidget::OnMainItemStateChanged(int ControllerID, EMainItemState NewState)
{
	SetTimerVisiblity(NewState == EMainItemState::PickedUp);

	FString player = (ControllerID == ID) ? "You " : "Player " + FString::FromInt(ControllerID + 1);
	FString state = "";

	switch (NewState)
	{
	case EMainItemState::PickedUp:
		state = " picked up ";
		if (ControllerID == ID)
		{			
			MainItemIcon->SetVisibility(ESlateVisibility::Hidden);
			bHasMainItem = true;
		}
		if (MainItem)
			MainItem = MainItem->Next;
		break;
	case EMainItemState::Dropped:
		state = " dropped ";
		if (bHasMainItem)
		{			
			bHasMainItem = false;
			MainItemIcon->SetVisibility(ESlateVisibility::Visible);
		}
		if (MainItem)
			MainItem = MainItem->Next;
		break;
	case EMainItemState::Delivered:
		state = " delivered ";
		MainItem = nullptr;
		MainItemIcon->SetVisibility(ESlateVisibility::Hidden);
		if (bHasMainItem)
			bHasMainItem = false;
	}

	AddMessage(ControllerID, player + state + " the main item!");
}

void UPlayerWidget::OnMainItemSet(ATaskObject* Object)
{
	AddMessage(-1, "The King demands a new item!");
	MainItem = Object;
	if(MainItem)
	{
		MainItemIcon->SetVisibility(ESlateVisibility::Visible);
	}
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
	SizeBoxSize.Value = 0.25f;

	FSlateChildSize OverlaySize;
	OverlaySize.SizeRule = ESlateSizeRule::Automatic;
	OverlaySize.Value = 1.0f;

	const FLinearColor ImageColor(1, 1, 1, 0.09f);

	for (int i = 0; i < Controllers.Num(); ++i)
	{
		const auto SizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		auto HorizontalSlot = ScoreBox->AddChildToHorizontalBox(SizeBox);
		HorizontalSlot->SetSize(SizeBoxSize);

		auto PlayerScoreWidget = CreateWidget<UPlayerScoreWidget>(this, PlayerScoreWidgetClass);
		PlayerScoreWidget->Initialize();

		auto ControllerID = UGameplayStatics::GetPlayerControllerID(Controllers[i]);

		PlayerScoreWidget->SetPlayerName("Player " + FString::FromInt(ControllerID + 1) + ": ", PlayerIcons[ControllerID]);

		PlayerScores.Add(ControllerID, PlayerScoreWidget);

		HorizontalSlot = ScoreBox->AddChildToHorizontalBox(PlayerScoreWidget);
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

void UPlayerWidget::AddMessage(int ControllerID, const FString& Message, const float Duration)
{
	if (Message.IsEmpty())
		return;

	auto HorizontalBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());

	auto Icon = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());

	Icon->SetBrushFromTexture((ControllerID >= 0) ? PlayerIcons[ControllerID] : KingIcon);
	Icon->SetBrushSize({ 64.f, 64.f });

	HorizontalBox->AddChildToHorizontalBox(Icon);

	auto NewTextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	NewTextBlock->SetText(FText::FromString(Message));
	NewTextBlock->SetFont(FontInfo);
	NewTextBlock->SetColorAndOpacity(FSlateColor({ 1.f, 0.8f, 0.23f, 1.f }));
	NewTextBlock->SetJustification(ETextJustify::Center);

	HorizontalBox->AddChildToHorizontalBox(NewTextBlock);
	MessageBox->AddChildToVerticalBox(HorizontalBox);

	for (auto& OldMessage : Messages)
	{
		for (auto& Child : OldMessage->GetAllChildren())
		{
			Child->SetRenderOpacity(0.6f);
		}
	}

	Messages.Add(HorizontalBox);

	btd::Delay(this, Duration, [this]()
	{
		if (GetName().Contains("None")) // Yup..
			return;

		if(Messages.Num())
		{
			auto OldMessage = Messages[0];
			if (OldMessage != nullptr)
			{
				MessageBox->RemoveChild(OldMessage);
				Messages.RemoveSingle(OldMessage);
			}
		}
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

void UPlayerWidget::SetTimerVisiblity(bool Visible)
{
	TimerBox->SetVisibility(Visible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}
