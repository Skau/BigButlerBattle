// Fill out your copyright notice in the Description page of Project Settings.


#include "TaskObject.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Math/RandomStream.h"
#include "TimerManager.h"
#include "ButlerGameInstance.h"
#include "Task.h"
#include "King/King.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Player/PlayerCharacter.h"
#include "Misc/FileHelper.h"

ATaskObject::ATaskObject()
{
	PrimaryActorTick.bCanEverTick = true;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("Mesh Component");
	SetRootComponent(MeshComponent);

	MeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Overlap);
	MeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel2, ECollisionResponse::ECR_Overlap);
	MeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel1);
	MeshComponent->SetGenerateOverlapEvents(true);
	MeshComponent->SetNotifyRigidBodyCollision(true);
	MeshComponent->SetSimulatePhysics(true);
	MeshComponent->SetRenderCustomDepth(true);
}

void ATaskObject::SetSelected(const bool Value)
{
	MeshComponent->SetCustomDepthStencilValue(static_cast<int32>(Value));
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

	MeshComponent->OnComponentBeginOverlap.AddDynamic(this, &ATaskObject::OnCollisionBeginOverlap);
	MeshComponent->OnComponentHit.AddDynamic(this, &ATaskObject::OnHit);

	if (LaunchVelocity != FVector::ZeroVector)
	{
		SetEnable(true, true, true);
		MeshComponent->AddImpulse(LaunchVelocity);
		bRecordingTimeSinceThrown = true;
	}
	
	DynamicMaterial = MeshComponent->CreateDynamicMaterialInstance(0, MeshComponent->GetMaterial(0));
}


void ATaskObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bRecordingTimeSinceThrown)
	{
		TimeSinceThrown += DeltaTime;

		if (Instigator && TimeSinceThrown > 0.5f)
			Instigator = nullptr;

		if (TimeSinceThrown > CountAsPlayerTaskThreshold)
		{
			bRecordingTimeSinceThrown = false;
		}
	}
}

#if WITH_EDITOR
void ATaskObject::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

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
	else if ((PropertyName == GET_MEMBER_NAME_CHECKED(ATaskObject, DrinksDataTable)))
	{
		if (TaskType == EObjectType::Drink && DrinksDataTable)
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
	else if ((PropertyName == GET_MEMBER_NAME_CHECKED(ATaskObject, CutleryDataTable)))
	{
		if (TaskType == EObjectType::Cutlery && CutleryDataTable)
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

	UpdateDataTables();

	UDataTable* DataTableToUse;

	switch (TaskType)
	{
	case EObjectType::Drink:
	{
		DataTableToUse = DrinksDataTable;
		break;
	}
	case EObjectType::Food:
	{
		DataTableToUse = FoodDataTable;
		break;
	}
	case EObjectType::Cutlery:
	{
		DataTableToUse = CutleryDataTable;
		break;
	}
	default:
		return false;
	}

	if (!DataTableToUse)
		return false;

	auto Rows = DataTableToUse->GetRowMap();
	const auto RowNum = Rows.Num();
	if (!RowNum)
		return false;

	const auto RowName = DataTableToUse->GetRowNames()[Stream.RandRange(0, RowNum - 1)];
	TaskData = NewObject<UTask>(this, RowName);
	TaskData->Name = RowName.ToString();

	if (!TaskData->InitTaskData(Rows[RowName]))
		return false;

	if (TaskData && TaskData->Type != EObjectType::None)
	{
		if (const auto Mesh = TaskData->Mesh)
		{
			MeshComponent->SetStaticMesh(Mesh);
		}
		else
		{
			MeshComponent->SetStaticMesh((DefaultMesh) ? DefaultMesh : nullptr);
		}

		if (const auto Material = TaskData->Material)
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

		if (const auto Mesh = TaskData->Mesh)
		{
			MeshComponent->SetStaticMesh(Mesh);
		}
		else
		{
			MeshComponent->SetStaticMesh((DefaultMesh) ? DefaultMesh : nullptr);
		}

		if (const auto Material = TaskData->Material)
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

void ATaskObject::OnCollisionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->IsA(AKing::StaticClass()))
	{
		if (bRecordingTimeSinceThrown && TimeSinceThrown < CountAsPlayerTaskThreshold)
		{
			OnTaskObjectDelivered.ExecuteIfBound(this);
			bRecordingTimeSinceThrown = false;
		}
	}
}

void ATaskObject::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (bCanHit)
	{
		if (auto Other = Cast<APlayerCharacter>(OtherActor))
		{
			Other->EnableRagdoll(NormalImpulse * GetVelocity(), Hit.ImpactPoint);
		}
	}

	bCanHit = false;
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

	TaskData = NewObject<UTask>(this, "DefaultTask");
	TaskData->Name = "DefaultTask";
}

void ATaskObject::OnPickedUp()
{
	SetEnable(false, false, false);

	if (bRespawn)
	{
		FTimerDelegate TimerCallback;
		TimerCallback.BindLambda([&]
			{
				SetEnable(true, true, true);
			});

		FTimerHandle Handle;
		GetWorld()->GetTimerManager().SetTimer(Handle, TimerCallback, RespawnTime, false);
	}
}

void ATaskObject::SetEnable(const bool NewVisiblity, const bool NewCollision, const bool NewPhysics) const
{
	MeshComponent->SetVisibility(NewVisiblity, true);

	MeshComponent->SetSimulatePhysics(NewPhysics);
	MeshComponent->SetEnableGravity(NewPhysics);
	MeshComponent->SetMassOverrideInKg(NAME_None, 1.f);

	MeshComponent->SetCollisionEnabled((NewCollision) ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
	MeshComponent->SetGenerateOverlapEvents(NewCollision);
}

void ATaskObject::Launch(const FVector Direction, const float Force) const
{
	MeshComponent->AddImpulse(Direction * Force);
}

void ATaskObject::UpdateDataTables()
{
	DrinksDataTable = NewObject<UDataTable>();
	DrinksDataTable->RowStruct = FTaskTableData::StaticStruct();
	DrinksDataTable->bIgnoreExtraFields = true;

	FString File;
	FString FilePath = FString(FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir()) + FString("Taskdata/DrinksData.csv"));
	bool success = FFileHelper::LoadFileToString(File, *FilePath);
	check(success == true);

	DrinksDataTable->CreateTableFromCSVString(File);

	FoodDataTable = NewObject<UDataTable>();
	FoodDataTable->RowStruct = FTaskTableData::StaticStruct();
	FoodDataTable->bIgnoreExtraFields = true;

	FilePath = FString(FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir()) + FString("Taskdata/FoodData.csv"));
	success = FFileHelper::LoadFileToString(File, *FilePath);
	check(success == true);

	FoodDataTable->CreateTableFromCSVString(File);

	CutleryDataTable = NewObject<UDataTable>();
	CutleryDataTable->RowStruct = FTaskTableData::StaticStruct();
	CutleryDataTable->bIgnoreExtraFields = true;

	FilePath = FString(FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir()) + FString("Taskdata/CutleryData.csv"));
	success = FFileHelper::LoadFileToString(File, *FilePath);
	check(success == true);

	CutleryDataTable->CreateTableFromCSVString(File);
}
