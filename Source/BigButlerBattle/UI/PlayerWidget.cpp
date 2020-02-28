// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerWidget.h"
#include "TaskWidget.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/Image.h"
#include "BigButlerBattleGameModeBase.h"
#include "Tasks/Task.h"
#include "Blueprint/WidgetTree.h"

void UPlayerWidget::InitializeTaskWidgets(const TArray<TPair<UTask*, ETaskState>>& Tasks)
{
	if (!TaskWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("TaskWidget Subclass not set in PlayerWidget"));
		return;
	}

	FSlateChildSize SizeBoxSize;
	SizeBoxSize.SizeRule = ESlateSizeRule::Fill;
	SizeBoxSize.Value = 0.1f;

	FSlateChildSize OverlaySize;
	OverlaySize.SizeRule = ESlateSizeRule::Fill;
	OverlaySize.Value = 1.0f;

	const FLinearColor ImageColor(1, 1, 1, 0.09f);

	for (int i = 0; i < Tasks.Num(); ++i)
	{
		const auto SizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		auto HorizontalSlot = TaskBox->AddChildToHorizontalBox(SizeBox);
		HorizontalSlot->SetSize(SizeBoxSize);

		auto Overlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());


		auto Image = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
		Image->SetColorAndOpacity(ImageColor);

		auto OverlaySlot = Overlay->AddChildToOverlay(Image);
		OverlaySlot->HorizontalAlignment = EHorizontalAlignment::HAlign_Fill;


		auto TaskWidget = CreateWidget<UTaskWidget>(this, TaskWidgetClass);
		TaskWidget->Initialize();
		TaskWidget->TaskBlock->SetText(FText::FromString(Tasks[i].Key->Name));
		TaskWidgets.Add(TaskWidget);

		OverlaySlot = Overlay->AddChildToOverlay(TaskWidget);
		OverlaySlot->HorizontalAlignment = EHorizontalAlignment::HAlign_Fill;

		HorizontalSlot = TaskBox->AddChildToHorizontalBox(Overlay);
		HorizontalSlot->SetSize(OverlaySize);
		HorizontalSlot->HorizontalAlignment = EHorizontalAlignment::HAlign_Fill;
	}

	const auto SizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	auto HorizontalSlot = TaskBox->AddChildToHorizontalBox(SizeBox);
	HorizontalSlot->SetSize(SizeBoxSize);
}

void UPlayerWidget::UpdateTaskSlotName(const int Index, const FString& Name)
{
	if (Index >= 0 && Index < TaskWidgets.Num())
	{
		TaskWidgets[Index]->TaskBlock->SetText(FText::FromString(Name));
	}
}

void UPlayerWidget::UpdateTaskState(const int Index, const ETaskState State)
{
	if (Index >= 0 && Index < TaskWidgets.Num())
	{
		switch (State)
		{
		case ETaskState::NotPresent:
		{
			TaskWidgets[Index]->TaskBlock->SetColorAndOpacity(NotPresent);
		}
		break;
		case ETaskState::Present:
		{
			TaskWidgets[Index]->TaskBlock->SetColorAndOpacity(Present);
		}
		break;
		case ETaskState::Finished:
		{
			TaskWidgets[Index]->TaskBlock->SetColorAndOpacity(Finished);
		}
		break;
		default:
			break;
		}
	}
}
