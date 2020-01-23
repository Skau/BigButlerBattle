// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Railing.generated.h"

// Forward declarations
class USplineComponent;
class UStaticMeshComponent;

USTRUCT(BlueprintType)
struct FBezierPoint : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Position;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector InTangent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector OutTangent;
};


UCLASS()
class BIGBUTLERBATTLE_API ARailing : public AActor
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Railing", meta = (DisplayName = "Splinepoints"))
	UDataTable *Splinepoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Railing", meta = (DisplayName = "Multiply by a hundo?"))
	bool bMultiplyByHundred = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Railing", meta = (DisplayName = "Swap x and y axis?"))
	bool bSwapXY = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Railing", meta = (DisplayName = "Swap y axis?"))
	bool bSwapY = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Railing", meta = (DisplayName = "Swap tangent y axis?"))
	bool bSwapTangentY = false;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* RailMesh;

	UPROPERTY(VisibleAnywhere)
	USplineComponent* SplineComp;

	// Sets default values for this actor's properties
	ARailing();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

#if WITH_EDITOR
	void PostEditChangeProperty(struct FPropertyChangedEvent &PropertyChangedEvent) override;
#endif

private:
	UFUNCTION(BlueprintCallable)
	void BuildSpline();

};
