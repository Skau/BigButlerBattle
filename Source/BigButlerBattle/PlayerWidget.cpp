// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerWidget.h"
#include "Components/Button.h"


UPlayerWidget::UPlayerWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{}


void UPlayerWidget::NativeConstruct()
{
	Super::NativeConstruct();
}


bool UPlayerWidget::Initialize()
{
	if (!Super::Initialize())
		return false;


	ButtonTest->OnClicked.AddDynamic(this, &UPlayerWidget::Test);


	return true;
}


void UPlayerWidget::NativeTick(const FGeometry& MyGeometry, float DeltaTime)
{
	Super::NativeTick(MyGeometry, DeltaTime);
}


void UPlayerWidget::Test()
{
	UE_LOG(LogTemp, Warning, TEXT("Test"))
}
