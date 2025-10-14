#include "PickupCube.h"
#include "Components/StaticMeshComponent.h"
#include "BackToZaraysk/Inventory/InventoryItemData.h"

APickupCube::APickupCube()
{
    if (UStaticMesh* SM = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
    {
        Mesh->SetStaticMesh(SM);
    }
    if (UMaterialInterface* M = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/EngineMaterials/DefaultWhiteGrid.DefaultWhiteGrid")))
    {
        Mesh->SetMaterial(0, M);
    }
    Mesh->SetRelativeScale3D(FVector(0.125f));
    Mesh->SetSimulatePhysics(true);
    Mesh->SetEnableGravity(true);
}

void APickupCube::BeginPlay()
{
    Super::BeginPlay();
    ApplyItemInstanceVisuals();
}

void APickupCube::ApplyItemInstanceVisuals()
{
    // Если передали конкретный экземпляр ItemInstance c размерами — подберём масштаб визуала под размер сетки
    if (!ItemInstance) return;
    const int32 SX = FMath::Max(1, ItemInstance->SizeInCellsX);
    const int32 SY = FMath::Max(1, ItemInstance->SizeInCellsY);
    const float Base = 0.125f; // базовый масштаб для 1x1
    Mesh->SetRelativeScale3D(FVector(Base * SX, Base * SY, Base));
}







