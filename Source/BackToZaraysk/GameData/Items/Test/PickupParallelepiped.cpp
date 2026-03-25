#include "PickupParallelepiped.h"
#include "Components/StaticMeshComponent.h"
#include "BackToZaraysk/Inventory/InventoryItemData.h"

APickupParallelepiped::APickupParallelepiped()
{
    if (UStaticMesh* SM = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
    {
        Mesh->SetStaticMesh(SM);
    }
    if (UMaterialInterface* M = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/EngineMaterials/DefaultWhiteGrid.DefaultWhiteGrid")))
    {
        Mesh->SetMaterial(0, M);
    }
    Mesh->SetRelativeScale3D(FVector(0.25f, 0.125f, 0.125f));
    Mesh->SetSimulatePhysics(true);
    Mesh->SetEnableGravity(true);
    ItemClass = UInventoryItemData_2x1::StaticClass();
}







