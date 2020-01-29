// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseUserWidget.h"
#include "Player/PlayerCharacterController.h"
#include "Components/Button.h"
#include "Application/SlateApplication.h"

UBaseUserWidget::UBaseUserWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{}

bool UBaseUserWidget::Initialize()
{
	bool bInit = Super::Initialize();

	return bInit;
}

void UBaseUserWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	for (auto& button : Buttons)
	{
		if (button->HasAnyUserFocus())
		{
			button->SetStyle(ButtonStyleHovered);
		}
		else
		{
			button->SetStyle(ButtonStyleDefault);
		}
	}
}

APlayerCharacterController* UBaseUserWidget::GetOwningPlayerCharacterController()
{
	return OwningCharacterController;
}

void UBaseUserWidget::FocusWidget(APlayerCharacterController* Controller, UWidget* WidgetToFocus)
{
	if (Controller)
	{
		UWidget* ActualWidgetToFocus = nullptr;

		if (WidgetToFocus == nullptr && DefaultWidgetToFocus)
			ActualWidgetToFocus = DefaultWidgetToFocus;
		else
			ActualWidgetToFocus = WidgetToFocus;

		WidgetFocusedLast = ActualWidgetToFocus;

		FInputModeUIOnly Mode;
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
		Mode.SetWidgetToFocus(ActualWidgetToFocus->GetCachedWidget());
		
		Controller->SetInputMode(Mode);
		Controller->CurrentMouseCursor = EMouseCursor::None;
		Controller->bShowMouseCursor = false;
		Controller->bEnableClickEvents = false;
		Controller->bEnableMouseOverEvents = false;
		FSlateApplication::Get().OnCursorSet();

		OwningCharacterController = Controller;
		OnPlayerCharacterControllerSet();
	}
}

void UBaseUserWidget::OnPlayerCharacterControllerSet()
{
}

void UBaseUserWidget::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent)
{
	// In case we want to use this function in blueprint too
	Super::NativeOnRemovedFromFocusPath(InFocusEvent);

	FocusWidget(OwningCharacterController, WidgetFocusedLast);
}
