// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerWidget.h"
#include "PlayerScoreWidget.h"
#include "Utils/btd.h"
#include "Player/PlayerCharacterController.h"
#include "GameFramework/Character.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/VerticalBox.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "BigButlerBattleGameModeBase.h"
#include "Tasks/Task.h"
#include "Blueprint/WidgetTree.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Tasks/TaskObject.h"
#include "King/King.h"
#include "Runtime/UMG/Public/Blueprint/SlateBlueprintLibrary.h"


bool UPlayerWidget::Initialize()
{
	const bool bInit = Super::Initialize();

	GameMode = Cast<ABigButlerBattleGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GameMode)
	{
		InitializeScores(GameMode->GetControllers());

		PlayerIconWidgets.Add(PlayerIcon1);
		PlayerIconWidgets.Add(PlayerIcon2);
		PlayerIconWidgets.Add(PlayerIcon3);
	}

	return bInit;
}

void UPlayerWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);


	// Main item icon
	
	if(MainItemIconWidget && MainItem && !bHasMainItem)
	{
		FVector2D Pos;
		if (WorldToScreen(MainItem->GetActorLocation(), Pos))
		{
			if (MainItemIconWidget->GetVisibility() != ESlateVisibility::Visible)
				MainItemIconWidget->SetVisibility(ESlateVisibility::Visible);

			MainItemIconWidget->SetRenderTranslation(Pos);
		}
		else
		{
			MainItemIconWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	// King icon

	if(!King)
	{
		King = GameMode->GetKing();
	}

	if (KingIconWidget && King)
	{
		FVector WorldPos = King->GetActorLocation();
		WorldPos.Z += 300.f;
		FVector2D Pos;
		if (WorldToScreen(WorldPos, Pos))
		{
			KingIconWidget->SetRenderTranslation(Pos);
		}
	}

	// Player icons

	
	// Have to do some initializing here because ID is not set yet in Initialize()
	if(!PlayerControllers.Num())
	{
		for (auto& Controller : GameMode->GetControllers())
		{
			const auto ControllerID = UGameplayStatics::GetPlayerControllerID(Controller);
			if (ControllerID != ID)
			{
				PlayerControllers.Add(Controller);
			}
		}

		for (int i = 0; i < PlayerControllers.Num(); ++i)
		{
			const auto ControllerID = UGameplayStatics::GetPlayerControllerID(PlayerControllers[i]);
			PlayerIconWidgets[i]->SetBrushFromTexture(PlayerIcons[ControllerID]);
			PlayerIconWidgets[i]->SetVisibility(ESlateVisibility::Visible);
		}
	}
	
	for(int i = 0; i < PlayerControllers.Num(); ++i)
	{
		auto Widget = PlayerIconWidgets[i];
		
		if (Widget->Visibility == ESlateVisibility::Hidden)
			Widget->SetVisibility(ESlateVisibility::Visible);

		const auto Player = Cast<ACharacter>(PlayerControllers[i]->GetPawn());
		if(Player)
		{
			auto WorldPos = Player->GetActorLocation();
			WorldPos.Z += 100.f;
			FVector2D Pos;
			if(WorldToScreen(WorldPos, Pos))
			{
				Widget->SetRenderTranslation(Pos);
			}
		}
	}
}

void UPlayerWidget::UpdateTimerText(const int ControllerID, const bool bCanDeliver)
{
	Timer_Image->SetBrushFromTexture(PlayerIcons[ControllerID]);
	
	FString Message;

	if(bCanDeliver)
	{
		const FString Player = ControllerID == ID ? "You " : "Player " + FString::FromInt(ControllerID + 1) + " ";
		Message = Player + "can deliver!";
		if (Timer_Time)
			Timer_Time->SetVisibility(ESlateVisibility::Hidden);
	}
	else
	{
		const FString Player = ControllerID == ID ? "you " : "player " + FString::FromInt(ControllerID + 1) + " ";
		
		Message = "Time until " + Player + "can deliver:";
	}

	Timer_Text->SetText(FText::FromString(Message));
}

void UPlayerWidget::UpdateTimer(const FString& String) const
{
	if (Timer_Time->Visibility == ESlateVisibility::Hidden)
		Timer_Time->SetVisibility(ESlateVisibility::Visible);
	
	Timer_Time->SetText(FText::FromString(String));
}

void UPlayerWidget::OnMainItemStateChanged(const int ControllerID, const EMainItemState NewState)
{
	SetTimerVisibility(NewState == EMainItemState::PickedUp);

	const FString player = ControllerID == ID ? "You " : "Player " + FString::FromInt(ControllerID + 1);
	FString State = "";

	switch (NewState)
	{
	case EMainItemState::PickedUp:
		State = " picked up ";
		if (ControllerID == ID)
		{			
			MainItemIconWidget->SetVisibility(ESlateVisibility::Hidden);
			bHasMainItem = true;
		}
		if (MainItem)
			MainItem = MainItem->Next;
		break;
	case EMainItemState::Dropped:
		State = " dropped ";
		if (bHasMainItem)
		{			
			bHasMainItem = false;
			MainItemIconWidget->SetVisibility(ESlateVisibility::Visible);
		}
		if (MainItem)
			MainItem = MainItem->Next;
		break;
	case EMainItemState::Delivered:
		State = " delivered ";
		if (bHasMainItem)
			bHasMainItem = false;
	// Note: This part applies so Delivered and Destroyed and hence no break.
	case EMainItemState::Destroyed:
		MainItem = nullptr;
		MainItemIconWidget->SetVisibility(ESlateVisibility::Hidden);
		break;
	}

	AddMessage(ControllerID, player + State + " the item!");
}

void UPlayerWidget::OnMainItemSet(ATaskObject* Object)
{
	AddMessage(-1, "The King demands a new item!");
	MainItem = Object;
	if(IsValid(MainItem))
	{
		MainItemIconWidget->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		MainItem = nullptr;
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
		HorizontalSlot->HorizontalAlignment = HAlign_Fill;
	}

	const auto SizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	auto HorizontalSlot = ScoreBox->AddChildToHorizontalBox(SizeBox);
	HorizontalSlot->SetSize(SizeBoxSize);
}

void UPlayerWidget::UpdateScore(const int ControllerID, const int NewScore)
{
	if (PlayerScores.Find(ControllerID))
	{
		PlayerScores[ControllerID]->UpdateScore(NewScore);
	}
}

void UPlayerWidget::AddMessage(const int ControllerID, const FString& Message, const float Duration)
{
	if (Message.IsEmpty())
		return;

	auto HorizontalBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());

	auto Icon = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());

	Icon->SetBrushFromTexture(ControllerID >= 0 ? PlayerIcons[ControllerID] : MainItemIcon);
	Icon->SetBrushSize({ 32.f, 32.f });

	HorizontalBox->AddChildToHorizontalBox(Icon);

	auto NewTextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	NewTextBlock->SetText(FText::FromString(Message));
	NewTextBlock->SetFont(FontInfo);
	NewTextBlock->SetJustification(ETextJustify::Center);

	auto HorizontalSlot = HorizontalBox->AddChildToHorizontalBox(NewTextBlock);
	HorizontalSlot->SetHorizontalAlignment(HAlign_Center);
	HorizontalSlot->SetVerticalAlignment(VAlign_Center);
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
			const auto OldMessage = Messages[0];
			if (OldMessage != nullptr)
			{
				MessageBox->RemoveChild(OldMessage);
				Messages.RemoveSingle(OldMessage);
			}
		}
	});
}

void UPlayerWidget::ShowKeybinds() const
{
	if (Image_Keybinds)
	{
		Image_Keybinds->SetVisibility(ESlateVisibility::Visible);
	}
}

void UPlayerWidget::HideKeybinds() const
{
	if (Image_Keybinds)
	{
		Image_Keybinds->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UPlayerWidget::SetTimerVisibility(const bool Visible) const
{
	TimerBox->SetVisibility(Visible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

bool UPlayerWidget::WorldToScreen(const FVector& WorldLocation, FVector2D& RelativeScreenPosition) const
{
	const auto PC = UGameplayStatics::GetPlayerControllerFromID(GetWorld(), ID);
	FVector2D Pos;
	if (PC->ProjectWorldLocationToScreen(WorldLocation, Pos, true))
	{
		USlateBlueprintLibrary::ScreenToViewport(PC, Pos, RelativeScreenPosition);

		const auto Geometry = GetCachedGeometry();
		const auto LocalSize = Geometry.GetLocalSize();
		const auto Middle = Geometry.AbsoluteToLocal(Geometry.GetAbsolutePosition()) + LocalSize / 2.0f;

		RelativeScreenPosition.X = FMath::Clamp(RelativeScreenPosition.X, Middle.X - LocalSize.X / 2.0f, Middle.X + LocalSize.X / 2.0f),
		RelativeScreenPosition.Y = FMath::Clamp(RelativeScreenPosition.Y, Middle.Y - LocalSize.Y / 2.0f, Middle.Y + LocalSize.Y / 2.0f);
		
		return true;
	}

	return false;
}