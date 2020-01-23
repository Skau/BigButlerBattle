// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuWidget.h"
#include "Components/MenuAnchor.h"
#include "MainMenuPlayerWidget.h"

UMainMenuWidget::UMainMenuWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{}


void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

bool UMainMenuWidget::Initialize()
{
	bool initialized = Super::Initialize();
	return initialized;
}

