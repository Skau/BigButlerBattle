// Fill out your copyright notice in the Description page of Project Settings.


#include "TaskObject.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Materials/MaterialInstanceConstant.h"
#include "ConstructorHelpers.h"

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

	if (!SetDataFromTable(ObjectType))
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

	if ((PropertyName == GET_MEMBER_NAME_CHECKED(ATaskObject, ObjectType)))
	{
		if (!SetDataFromTable(ObjectType))
		{
			SetDefault();
		}
	}
	else if ((PropertyName == GET_MEMBER_NAME_CHECKED(ATaskObject, FoodDataTable)))
	{
		if (ObjectType == EObjectType::Food && FoodDataTable)
		{
			if (!SetDataFromTable(EObjectType::Food))
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
		if (ObjectType == EObjectType::Wine && WineDataTable)
		{
			if (!SetDataFromTable(EObjectType::Wine))
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


bool ATaskObject::SetDataFromTable(EObjectType Type)
{
	switch (ObjectType)
	{
	case EObjectType::Wine:
	{
		if (!WineDataTable)
			return false;

		auto Rows = WineDataTable->GetRowMap();
		auto RowNum = Rows.Num();
		if (!RowNum)
			return false;

		auto RowName = WineDataTable->GetRowNames()[FMath::RandRange(0, RowNum - 1)];
		
		auto Row = (FWineTableData*)Rows[RowName];

		if (Row)
		{
			ObjectName = RowName.ToString();
			AssetData = Row;
		}
		else
		{
			ObjectName = "";
			AssetData = nullptr;
		}
	
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

		auto RowName = FoodDataTable->GetRowNames()[FMath::RandRange(0, RowNum - 1)];

		auto Row = (FFoodTableData*)Rows[RowName];

		if (Row)
		{
			ObjectName = RowName.ToString();
			AssetData = Row;
		}
		
		break;
	}
	default:
		return false;
	}

	if (AssetData)
	{
		if (auto Mesh = AssetData->Mesh)
		{
			MeshComponent->SetStaticMesh(Mesh);
		}
		else
		{
			MeshComponent->SetStaticMesh((DefaultMesh) ? DefaultMesh : nullptr);
		}

		if (auto Material = AssetData->Material)
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

	ObjectType = EObjectType::None;
	AssetData = nullptr;
	ObjectName = "";
}
