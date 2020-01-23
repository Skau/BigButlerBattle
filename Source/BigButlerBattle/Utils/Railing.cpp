// Fill out your copyright notice in the Description page of Project Settings.

#include "Railing.h"
#include "Components/SplineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "btd.h"

// Sets default values
ARailing::ARailing()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BuildSpline();

	RailMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rail Mesh"));
	SetRootComponent(RailMesh);

	SplineComp = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	SplineComp->SetupAttachment(RootComponent);
	SplineComp->ScaleVisualizationWidth = 5.f;
	SplineComp->bShouldVisualizeScale = true;
	SplineComp->bSplineHasBeenEdited = true;
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

void ARailing::PostEditChangeProperty(struct FPropertyChangedEvent &PropertyChangedEvent)
{
    //Get the name of the property that was changed  
    FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;  

    // We test using GET_MEMBER_NAME_CHECKED so that if someone changes the property name  
    // in the future this will fail to compile and we can update it.  
    if ((PropertyName == GET_MEMBER_NAME_CHECKED(ARailing, Splinepoints)))  
        BuildSpline();

    // Call the base class version  
    Super::PostEditChangeProperty(PropertyChangedEvent);  
}

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
			auto row = Splinepoints->FindRow<FBezierPoint>(rows[i], *Splinepoints->GetName());
			if (row)
			{
				const float hermite = 3.f;

				auto pos = row->Position;
				if (bSwapY)
					pos.Y = pos.Y;
				if (bSwapXY)
					pos = btd::SwapXY(pos);
				if (bMultiplyByHundred)
					pos *= 100.f;

				
				auto inTan = row->InTangent - row->Position;
				if (bSwapY ^ bSwapTangentY)
					inTan.Y = -inTan.Y;
				if (bSwapXY)
					inTan = btd::SwapXY(inTan);
				if (bMultiplyByHundred)
					inTan *= 100.f;


				auto outTan = row->OutTangent - row->Position;
				if (bSwapY ^ bSwapTangentY)
					outTan.Y = -outTan.Y;
				if (bSwapXY)
					outTan = btd::SwapXY(outTan);		
				if (bMultiplyByHundred)
					outTan *= 100.f;


				SplineComp->AddSplinePoint(pos, ESplineCoordinateSpace::Local);
				SplineComp->SetTangentsAtSplinePoint(i, inTan, outTan, ESplineCoordinateSpace::Local);

				// if (10.f < row->Position.Z)
				// 	SplineComp->SetUpVectorAtSplinePoint(i, FVector::UpVector, ESplineCoordinateSpace::World);
			}
		}
	}
}
