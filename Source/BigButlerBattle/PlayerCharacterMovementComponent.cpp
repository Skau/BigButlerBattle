// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacterMovementComponent.h"

UPlayerCharacterMovementComponent::UPlayerCharacterMovementComponent()
{

}

void UPlayerCharacterMovementComponent::TickComponent(float deltaTime, enum ELevelTick TickType, FActorComponentTickFunction* thisTickFunction)
{
	Super::TickComponent(deltaTime, TickType, thisTickFunction);

}