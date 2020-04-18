#include "ObjectiveMarker.h"
#include "Components/MaterialBillboardComponent.h"

AObjectiveMarker::AObjectiveMarker()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    Billboard = CreateDefaultSubobject<UMaterialBillboardComponent>(TEXT("Billboard"));
    SetRootComponent(Billboard);
    Billboard->AddElement(nullptr, nullptr, true, 1.f, 1.f, nullptr);
}