// Fill out your copyright notice in the Description page of Project Settings.


#include "TaskObject.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Materials/MaterialInstanceConstant.h"
#include "ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Math/RandomStream.h"
#include "ButlerGameInstance.h"
#include "WineTask.h"
#include "FoodTask.h"

ATaskObject::ATaskObject()
{
	// PrimaryActorTick.bCanEverTick = true;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("Mesh Component");
	SetRootComponent(MeshComponent);

	MeshComponent->SetGenerateOverlapEvents(true);
	MeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel1);

	ConstructorHelpers::FObjectFinder<UDataTable> WineDataTableDefinition(TEXT("DataTable'/Game/TaskObjects/Wine/WineData.WineData'"));
	auto WineDataObject = WineDataTableDefinition.Object;
	if (WineDataObject)
	{
		WineDataTable = WineDataObject;
	}

	ConstructorHelpers::FObjectFinder<UDataTable> FoodDataTableDefinition(TEXT("DataTable'/Game/TaskObjects/Food/FoodData.FoodData'"));
	auto FoodDataObject = FoodDataTableDefinition.Object;
	if (FoodDataObject)
	{
		FoodDataTable = FoodDataObject;
	}
}


void ATaskObject::BeginPlay()
{
	Super::BeginPlay();

	bool Success = false;

	if (TaskData && TaskData->Type != EObjectType::None)
	{
		Success = SetDataFromAssetData();
	}
	else
	{
		if(TaskType != EObjectType::None)
			Success = SetDataFromTable();
	}

	if (!Success)
	{
		SetDefault();
	}
}


void ATaskObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

#if WITH_EDITOR
void ATaskObject::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == NAME_None)
		return;

	if ((PropertyName == GET_MEMBER_NAME_CHECKED(ATaskObject, TaskType)))
	{
		if (!SetDataFromTable())
		{
			SetDefault();
		}
	}
	else if ((PropertyName == GET_MEMBER_NAME_CHECKED(ATaskObject, FoodDataTable)))
	{
		if (TaskType == EObjectType::Food && FoodDataTable)
		{
			if (!SetDataFromTable())
			{
				SetDefault();
			}
		}
		else
		{
			SetDefault();
		}
	}
	else if ((PropertyName == GET_MEMBER_NAME_CHECKED(ATaskObject, WineDataTable)))
	{
		if (TaskType == EObjectType::Wine && WineDataTable)
		{
			if (!SetDataFromTable())
			{
				SetDefault();
			}
		}
		else
		{
			SetDefault();
		}
	}
	else if ((PropertyName == GET_MEMBER_NAME_CHECKED(ATaskObject, TaskData)))
	{
		if (TaskData && TaskData->Type != EObjectType::None)
		{
			if (!SetDataFromAssetData())
			{
				SetDefault();
			}
		}
		else
		{
			SetDefault();
		}
	}
}
#endif


bool ATaskObject::SetDataFromTable()
{
	FRandomStream Stream;
	auto Instance = Cast<UButlerGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	if (Instance)
	{
		Stream.Initialize(Instance->GetCurrentRandomSeed() + (GetActorLocation().X * 1234.25f) + (GetActorLocation().Y * 5678.98f) + 1.75f + GetActorLocation().Z * 3456);
	}
	else
	{
		Stream.GenerateNewSeed();
	}

	switch (TaskType)
	{
	case EObjectType::Wine:
	{
		if (!WineDataTable)
			return false;

		auto Rows = WineDataTable->GetRowMap();
		auto RowNum = Rows.Num();
		if (!RowNum)
			return false;

		auto RowName = WineDataTable->GetRowNames()[Stream.RandRange(0, RowNum - 1)];
		TaskData = NewObject<UWineTask>(this, RowName);
		TaskData->Name = RowName.ToString();

		if (!TaskData->InitTaskData(Rows[RowName]))
			return false;

		break;
	}
	case EObjectType::Food:
	{
		if (!FoodDataTable)
			return false;

		auto Rows = FoodDataTable->GetRowMap();
		auto RowNum = Rows.Num();
		if (!RowNum)
			return false;

		auto RowName = FoodDataTable->GetRowNames()[Stream.RandRange(0, RowNum - 1)];
		TaskData = NewObject<UFoodTask>(this, RowName);
		TaskData->Name = RowName.ToString();

		if (!TaskData->InitTaskData(Rows[RowName]))
			return false;

		break;
	}
	default:
		return false;
	}

	if (TaskData && TaskData->Type != EObjectType::None)
	{
		if (auto Mesh = TaskData->Mesh)
		{
			MeshComponent->SetStaticMesh(Mesh);
		}
		else
		{
			MeshComponent->SetStaticMesh((DefaultMesh) ? DefaultMesh : nullptr);
		}

		if (auto Material = TaskData->Material)
		{
			MeshComponent->SetMaterial(0, Material);
		}
		else
		{
			MeshComponent->SetMaterial(0, (DefaultMaterial) ? DefaultMaterial : nullptr);
		}
		return true;
	}
	else
	{
		return false;
	}

}

bool ATaskObject::SetDataFromAssetData()
{
	if (TaskData && TaskData->Type != EObjectType::None)
	{
		TaskType = TaskData->Type;

		if (auto Mesh = TaskData->Mesh)
		{
			MeshComponent->SetStaticMesh(Mesh);
		}
		else
		{
			MeshComponent->SetStaticMesh((DefaultMesh) ? DefaultMesh : nullptr);
		}

		if (auto Material = TaskData->Material)
		{
			MeshComponent->SetMaterial(0, Material);
		}
		else
		{
			MeshComponent->SetMaterial(0, (DefaultMaterial) ? DefaultMaterial : nullptr);
		}
		return true;
		
	}
	else
	{
		return false;
	}
}

void ATaskObject::SetDefault()
{
	if (DefaultMesh)
	{
		MeshComponent->SetStaticMesh(DefaultMesh);
	}

	if (DefaultMaterial)
	{
		MeshComponent->SetMaterial(0, DefaultMaterial);
	}

	TaskData = NewObject<UBaseTask>(this, "DefaultTask");
	TaskData->Name = "DefaultTask";
}
