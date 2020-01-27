// Fill out your copyright notice in the Description page of Project Settings.

#include "Railing.h"
#include "Components/SplineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "btd.h"
#include "Utils/DataTables.h"

/// ------------------------------- FBezierPoint -------------------------------





/// ------------------------------- ARailing -------------------------------

// Sets default values
ARailing::ARailing()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RailMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rail Mesh"));
	SetRootComponent(RailMesh);

	SplineComp = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	SplineComp->SetupAttachment(RootComponent);
	SplineComp->bSplineHasBeenEdited = true;

#if WITH_EDITORONLY_DATA
	SplineComp->ScaleVisualizationWidth = 5.f;
	SplineComp->bShouldVisualizeScale = true;
#endif

	BuildSpline();
}

// Called when the game starts or when spawned
void ARailing::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARailing::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

#if WITH_EDITOR
void ARailing::PostEditChangeProperty(struct FPropertyChangedEvent &PropertyChangedEvent)
{
    //Get the name of the property that was changed  
    FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;  

    // We test using GET_MEMBER_NAME_CHECKED so that if someone changes the property name  
    // in the future this will fail to compile and we can update it.  
    if (PropertyName == GET_MEMBER_NAME_CHECKED(ARailing, Splinepoints) || PropertyName == GET_MEMBER_NAME_CHECKED(ARailing, TangentMultiplier))
        BuildSpline();

    // Call the base class version  
    Super::PostEditChangeProperty(PropertyChangedEvent);  
}
#endif

void ARailing::BuildSpline()
{
	if (!SplineComp)
	{
		UE_LOG(LogTemp, Error, TEXT("Splinecomponent failed to be created!"));
		return;
	}

	if (Splinepoints)
	{
		SplineComp->ClearSplinePoints();

		const auto rows = Splinepoints->GetRowNames();

		for (int32 i{0}; i < rows.Num(); ++i)
		{
			auto row = Splinepoints->FindRow<FBezierPoint>(bSwapPointOrder ? rows[rows.Num() - i - 1] : rows[i], *Splinepoints->GetName());
			if (row)
			{
				auto transform = [](const FVector& f)
				{
					return FVector{f.Y, f.X, f.Z} * 100.f;
				};

				auto pos = transform(row->Position);

				/* In-tangent is multiplied by -1 because tangents are relative to the direction they're pointing.
				* So the in-tangent isn't a vector pointing from the control point, it's a negative direction vector
				* from the control point.
				*/
				auto inTan = transform(row->InTangent);
				auto outTan = -transform(row->OutTangent);

				if (bRotateSpline)
				{
					auto rotate = [](const FVector& f)
					{
						return FVector{f.Y, -f.X, f.Z};
					};

					pos = rotate(pos);
					inTan = rotate(inTan);
					outTan = rotate(outTan);
				}

				if (bInvertTangents)
				{
					inTan *= -1.f;
					outTan *= -1.f;
				}

				if (bSwapInOutTangents)
					btd::Swap(inTan, outTan);



				SplineComp->AddSplinePoint(pos, ESplineCoordinateSpace::Local);
				SplineComp->SetTangentsAtSplinePoint(i, inTan * TangentMultiplier, outTan * TangentMultiplier, ESplineCoordinateSpace::Local);

				// if (10.f < row->Position.Z)
				// 	SplineComp->SetUpVectorAtSplinePoint(i, FVector::UpVector, ESplineCoordinateSpace::World);
			}
		}
	}
}
