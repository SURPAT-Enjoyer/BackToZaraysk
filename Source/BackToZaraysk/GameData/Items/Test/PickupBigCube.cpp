#include "PickupBigCube.h"
#include "Components/StaticMeshComponent.h"
#include "BackToZaraysk/Inventory/InventoryItemData.h"

APickupBigCube::APickupBigCube()
{
    if (UStaticMesh* SM = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
    {
        Mesh->SetStaticMesh(SM);
    }
    if (UMaterialInterface* M = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/EngineMaterials/DefaultWhiteGrid.DefaultWhiteGrid")))
    {
        Mesh->SetMaterial(0, M);
    }
    // Большой куб - размер 2x2 в инвентаре, поэтому в мире он должен быть больше обычного куба
    Mesh->SetRelativeScale3D(FVector(0.25f)); // 0.25 вместо 0.125 - в 2 раза больше
    Mesh->SetSimulatePhysics(true);
    Mesh->SetEnableGravity(true);
    ItemClass = UInventoryItemData_2x2::StaticClass();
}
