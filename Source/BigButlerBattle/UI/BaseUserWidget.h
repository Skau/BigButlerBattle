// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Styling/SlateTypes.h"
#include "BaseUserWidget.generated.h"

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
	APlayerController* GetOwningPlayerController() const;

	void FocusWidget(APlayerController* Controller, UWidget* WidgetToFocus = nullptr);

protected:
	bool Initialize() override;

	void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY()
	UWidget* DefaultWidgetToFocus = nullptr;

	virtual void OnPlayerControllerSet();

	virtual void OnBackButtonPressed();

	UPROPERTY(BlueprintReadOnly)
	APlayerController* OwningPlayerController;

	TArray<UButton*> Buttons;

	UPROPERTY(EditDefaultsOnly)
	FButtonStyle ButtonStyleDefault;

	UPROPERTY(EditDefaultsOnly)
	FButtonStyle ButtonStyleHovered;

	void NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) override;

	UPROPERTY()
	FSlateSound ButtonPressedSound;

	UPROPERTY()
	FSlateSound ButtonHoveredSound;

private:
	UPROPERTY()
	UWidget* WidgetFocusedLast = nullptr;
};
