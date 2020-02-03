// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Styling/SlateTypes.h"
#include "BaseUserWidget.generated.h"

class APlayerCharacterController;
class UButton;

/**
 * 
 */
UCLASS()
class BIGBUTLERBATTLE_API UBaseUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UBaseUserWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintPure)
	APlayerCharacterController* GetOwningPlayerCharacterController();

	void FocusWidget(APlayerCharacterController* Controller, UWidget* WidgetToFocus = nullptr);

protected:
	bool Initialize() override;

	void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UWidget* DefaultWidgetToFocus = nullptr;

	/* Overriden in sublcasses */
	virtual void OnPlayerCharacterControllerSet();

	UPROPERTY(BlueprintReadOnly)
	APlayerCharacterController* OwningCharacterController;

	virtual void OnBackButtonPressed();

	TArray<UButton*> Buttons;

	UPROPERTY(EditDefaultsOnly)
	FButtonStyle ButtonStyleDefault;

	UPROPERTY(EditDefaultsOnly)
	FButtonStyle ButtonStyleHovered;

	void NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) override;

private:
	UWidget* WidgetFocusedLast = nullptr;
};
