// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCameraComponent.h"
#include "PlayerCharacter.h"
#include "PlayerCharacterMovementComponent.h"


UPlayerCameraComponent::UPlayerCameraComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPlayerCameraComponent::BeginPlay()
{
	Super::BeginPlay();

	Player = Cast<APlayerCharacter>(GetOwner());
	check(Player != nullptr);
}

void UPlayerCameraComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FMinimalViewInfo Info;
	GetCameraView(DeltaTime, Info);

	if (!Player->HasEnabledRagdoll())
	{
		const auto MoveComp = Player->GetPlayerCharacterMovementComponent();
		const auto MaxVel = MoveComp->MaxCustomMovementSpeed;
		const auto CurrentVel = Player->GetVelocity().Size();

		const auto DesiredFOV = FMath::Lerp(MinFOV, MaxFOV, CurrentVel / MaxVel);
		const auto Factor = FMath::Clamp(FieldOfViewSpeedChange * DeltaTime, 0.f, 1.0f);
		SetFieldOfView((FMath::IsNearlyZero(Info.FOV - DesiredFOV)) ? DesiredFOV : FMath::Lerp(Info.FOV, DesiredFOV, Factor));
	}
	else
	{
		if(!FMath::IsNearlyZero(Info.FOV - MinFOV))
			SetFieldOfView(MinFOV);
	}
}