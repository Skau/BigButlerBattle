// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "PlayerCharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"


APlayerCharacter::APlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: ACharacter(ObjectInitializer.SetDefaultSubobjectClass<UPlayerCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;

	SkateboardMesh = CreateDefaultSubobject<USkeletalMeshComponent>("SkateboardMesh");
	SkateboardMesh->SetupAttachment(RootComponent);

	bUseControllerRotationYaw = false;
}

void APlayerCharacter::EnableRagdoll()
{
	if (!bCanFall || bEnabledRagdoll)
		return;

	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetAllBodiesSimulatePhysics(true);
	
	bEnabledRagdoll = true;
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	Movement = Cast<UPlayerCharacterMovementComponent>(GetMovementComponent());
	check(Movement != nullptr);
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bEnabledRagdoll && bCurrentlyHoldingHandbrake && Movement->Velocity.Size() > HandbreakeVelocityThreshold && !Movement->IsFalling())
	{
		AddActorLocalRotation({ 0, RightAxis * DeltaTime * HandbrakeRotationFactor, 0 });
	}
}

