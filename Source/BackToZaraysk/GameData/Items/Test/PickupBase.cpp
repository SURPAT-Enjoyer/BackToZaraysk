#include "PickupBase.h"
#include "Components/StaticMeshComponent.h"

APickupBase::APickupBase()
{
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    SetRootComponent(Mesh);
    Mesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
    Mesh->SetSimulatePhysics(true);
    Mesh->SetEnableGravity(true);
}

void APickupBase::ApplyItemInstanceVisuals()
{
    // Базовая реализация — ничего не делает
}







