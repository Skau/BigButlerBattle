// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerWidget.generated.h"

class UButton;

/**
 * 
 */
UCLASS()
class BIGBUTLERBATTLE_API UPlayerWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPlayerWidget(const FObjectInitializer& ObjectInitializer);

protected:

	void NativeConstruct() override;

	bool Initialize() override;

	void NativeTick(const FGeometry& MyGeometry, float DeltaTime) override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* ButtonTest;

private:
	UFUNCTION()
	void Test();
};
