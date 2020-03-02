// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseUserWidget.h"
#include "Components/Button.h"
#include "Application/SlateApplication.h"

UBaseUserWidget::UBaseUserWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{}

bool UBaseUserWidget::Initialize()
{
	const bool bInit = Super::Initialize();

	return bInit;
}

void UBaseUserWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	for (auto& Button : Buttons)
	{
		if (Button->HasAnyUserFocus())
		{
			Button->SetStyle(ButtonStyleHovered);
		}
		else
		{
			Button->SetStyle(ButtonStyleDefault);
		}
	}
}

APlayerController* UBaseUserWidget::GetOwningPlayerController() const
{
	return OwningPlayerController;
}

void UBaseUserWidget::FocusWidget(APlayerController* Controller, UWidget* WidgetToFocus)
{
	if (Controller)
	{
		UWidget* ActualWidgetToFocus = (WidgetToFocus == nullptr && DefaultWidgetToFocus) ? DefaultWidgetToFocus : WidgetToFocus;
		WidgetFocusedLast = ActualWidgetToFocus;

		if (ActualWidgetToFocus)
		{
			FInputModeGameAndUI Mode;
			Mode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
			Mode.SetWidgetToFocus(ActualWidgetToFocus->GetCachedWidget());

			//UE_LOG(LogTemp, Warning, TEXT("Focusing %s"), *ActualWidgetToFocus->GetName());

			Controller->SetInputMode(Mode);
			Controller->CurrentMouseCursor = EMouseCursor::None;
			Controller->bShowMouseCursor = false;
			Controller->bEnableClickEvents = false;
			Controller->bEnableMouseOverEvents = false;
			FSlateApplication::Get().OnCursorSet();

			OwningPlayerController = Controller;
			OnPlayerControllerSet();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("No actual widget to focus! Controller name: %s, This widget name: %s"), *Controller->GetName(), *GetName());
		}
	}
	else
	{
		// This isn't necessarily bad, it just means the controller is not active.
		// We always try to focus every controller with their individual MainMenuPlayerWidget for example.
		UE_LOG(LogTemp, Warning, TEXT("Controller was nullptr! This widget name: %s"), *GetName()); 
	}
}

void UBaseUserWidget::OnPlayerControllerSet()
{
}

void UBaseUserWidget::OnBackButtonPressed()
{
}

void UBaseUserWidget::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent)
{
	// In case we want to use this function in blueprint too
	Super::NativeOnRemovedFromFocusPath(InFocusEvent);

	FocusWidget(OwningPlayerController, WidgetFocusedLast);
}
