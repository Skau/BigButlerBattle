// Fill out your copyright notice in the Description page of Project Settings.


#include "BBBSettings.h"

UBBBSettings::UBBBSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bUseCustomSeed(false)
	, CustomSeed(1337)
{
}