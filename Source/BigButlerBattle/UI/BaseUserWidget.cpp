// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseUserWidget.h"
#include "Player/PlayerCharacterController.h"
#include "Components/Button.h"

UBaseUserWidget::UBaseUserWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{}

bool UBaseUserWidget::Initialize()
{
	bool bInitialized = Super::Initialize();

	return bInitialized;
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

		FInputModeUIOnly Mode;
		Mode.SetWidgetToFocus(ActualWidgetToFocus->GetCachedWidget());
		Controller->SetInputMode(Mode);

		OwningCharacterController = Controller;
		OnPlayerCharacterControllerSet();
	}
}

void UBaseUserWidget::OnPlayerCharacterControllerSet()
{
}
