// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCameraComponent.h"
#include "PlayerCharacter.h"
#include "PlayerCharacterMovementComponent.h"
#include "Curves/CurveFloat.h"


UPlayerCameraComponent::UPlayerCameraComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	FieldOfView = 105.f;

	// =================== Postprocessing ====================================
	PostProcessSettings.bOverride_BloomIntensity = true;
	PostProcessSettings.bOverride_Bloom1Tint = true;
	PostProcessSettings.bOverride_Bloom2Tint = true;
	PostProcessSettings.bOverride_Bloom3Tint = true;
	PostProcessSettings.bOverride_Bloom4Tint = true;
	PostProcessSettings.bOverride_SceneFringeIntensity = true;
	PostProcessSettings.bOverride_ChromaticAberrationStartOffset = true;
	PostProcessSettings.bOverride_LensFlareIntensity = true;
	PostProcessSettings.bOverride_LensFlareTint = true;
	PostProcessSettings.bOverride_LensFlareBokehSize = true;
	PostProcessSettings.bOverride_LensFlareThreshold = true;
	PostProcessSettings.bOverride_VignetteIntensity = true;
	PostProcessSettings.bOverride_GrainJitter = true;
	PostProcessSettings.bOverride_GrainIntensity = true;
	PostProcessSettings.bOverride_DepthOfFieldFocalDistance = true;
	PostProcessSettings.bOverride_DepthOfFieldDepthBlurRadius = true;
	PostProcessSettings.bOverride_WhiteTemp = true;
	PostProcessSettings.bOverride_ColorGamma = true;
	PostProcessSettings.bOverride_ColorGainShadows = true;
	PostProcessSettings.bOverride_MotionBlurTargetFPS = true;
	PostProcessSettings.bOverride_ReflectionsType = true;

	PostProcessSettings.BloomIntensity = 2.427381f;
	PostProcessSettings.Bloom1Tint = FLinearColor { 0.782849f, 0.3465f, 0.3465f };
	PostProcessSettings.Bloom2Tint = FLinearColor{0.138f, 0.138f, 0.328476f};
	PostProcessSettings.Bloom3Tint = FLinearColor{0.012862f, 0.116085f, 0.1176f};
	PostProcessSettings.Bloom3Tint = FLinearColor{0.066f, 0.009738f, 0.063632f};
	PostProcessSettings.SceneFringeIntensity = 1.476191f;
	PostProcessSettings.ChromaticAberrationStartOffset = 0.466667f;
	PostProcessSettings.LensFlareIntensity = 0.152381f;
	PostProcessSettings.LensFlareTint = FLinearColor { 1.f, 0.861075f, 0.670596f};
	PostProcessSettings.LensFlareBokehSize = 2.438096f;
	PostProcessSettings.LensFlareThreshold = 25.924765f;
	PostProcessSettings.VignetteIntensity = 0.67619f;
	PostProcessSettings.GrainJitter = 0.076191f;
	PostProcessSettings.GrainIntensity = 0.095239f;
	PostProcessSettings.DepthOfFieldFocalDistance = 286.68573f;
	PostProcessSettings.DepthOfFieldDepthBlurRadius = 1.104762f;
	PostProcessSettings.WhiteTemp = 6757.143066f;
	PostProcessSettings.ColorGamma = FVector4 { 0.946601f, 0.971328f, 1.f, 1.f };
	PostProcessSettings.ColorGainShadows = FVector4 { 0.621799f, 0.758338f, 1.f, 1.f};
	PostProcessSettings.MotionBlurTargetFPS = 61;
	PostProcessSettings.ReflectionsType = EReflectionsType::ScreenSpace;
}

void UPlayerCameraComponent::BeginPlay()
{
	Super::BeginPlay();

	Player = Cast<APlayerCharacter>(GetOwner());
	check(Player != nullptr);

	// Ensure max and min are actual max and min.
	MinFOV = FMath::Max(FMath::Min(MinFOV, FMath::Min(MaxFOV, MaxPlayerInputFOV)), 1.f);
	MaxFOV = FMath::Min(FMath::Max(MinFOV, FMath::Max(MaxFOV, MaxPlayerInputFOV)), 180.f);

	if (!IsValid(FOVCurve))
	{
		FOVCurve = NewObject<UCurveFloat>(this, TEXT("Default Curve"));
		FString curveVals{};
		curveVals += FString{"0,"} + FString::SanitizeFloat(MinFOV, 1) + FString{"\n"};
		curveVals += FString{"1,"} + FString::SanitizeFloat(MaxPlayerInputFOV, 1) + FString{"\n"};
		curveVals += FString{"2,"} + FString::SanitizeFloat(MaxFOV, 1) + FString{"\n"};
		const auto errors = FOVCurve->CreateCurveFromCSVString(curveVals);
		for (const auto &err : errors)
		{
			UE_LOG(LogTemp, Error, TEXT("Err: %s"), *err);
		}
		check(0 == errors.Num());
	}
}

void UPlayerCameraComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FMinimalViewInfo Info;
	GetCameraView(DeltaTime, Info);

	if (!Player->HasEnabledRagdoll())
	{
		const auto MoveComp = Player->GetPlayerCharacterMovementComponent();
		const auto MaxInputVel = MoveComp->GetMaxInputSpeed();
		const auto MaxVel = MoveComp->MaxCustomMovementSpeed;
		const auto CurrentVel = bConstrainFOVChangeToVelocityInXYDirections ? Player->GetVelocity().Size2D() : Player->GetVelocity().Size();

		auto range = FMath::Max(CurrentVel / MaxInputVel, 0.f);
		if (1.f < range)
			range = FMath::Min(CurrentVel / MaxVel, 1.f);

		const auto DesiredFOV = FMath::Clamp(FOVCurve->GetFloatValue(range), MinFOV, MaxFOV);
		const auto Factor = FMath::Clamp(FieldOfViewSpeedChange * DeltaTime, 0.f, 1.0f);
		SetFieldOfView((FMath::IsNearlyZero(Info.FOV - DesiredFOV)) ? DesiredFOV : FMath::Lerp(Info.FOV, DesiredFOV, Factor));
	}
	else
	{
		if(!FMath::IsNearlyZero(Info.FOV - MinFOV))
			SetFieldOfView(MinFOV);
	}
}