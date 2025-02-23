// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Railing.generated.h"

// Forward declarations
class USplineComponent;
class UStaticMeshComponent;
class UBoxComponent;


UCLASS()
class BIGBUTLERBATTLE_API ARailing : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Railing", meta = (DisplayName = "Splinepoints"))
	UDataTable* Splinepoints = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Railing", meta = (DisplayName = "Swap in and out tangents?"))
	bool bSwapInOutTangents = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Railing", meta = (DisplayName = "Swap point order?"))
	bool bSwapPointOrder = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Railing", meta = (DisplayName = "Rotate spline -90 in yaw?"))
	bool bRotateSpline = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Railing", meta = (DisplayName = "Invert direction of tangents?"))
	bool bInvertTangents = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Railing", meta = (DisplayName = "Tangent multiplier"))
	float TangentMultiplier = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Railing")
	bool bLoopedRail = false;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* RailMesh;

	UPROPERTY(VisibleAnywhere)
	USplineComponent* SplineComp;

	// Sets default values for this actor's properties
	ARailing();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void BuildSpline() const;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

#if WITH_EDITOR
	void PostEditChangeProperty(struct FPropertyChangedEvent &PropertyChangedEvent) override;
#endif

};
