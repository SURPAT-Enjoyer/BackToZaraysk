#include "HelmetBase.h"
#include "Components/StaticMeshComponent.h"
#include "BackToZaraysk/Inventory/EquippableItemData.h"
#include "Engine/StaticMesh.h"

AHelmetBase::AHelmetBase()
{
    // World pickup: статический куб как placeholder шлема
    if (Mesh)
    {
        UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
        Mesh->SetStaticMesh(Cube);
        Mesh->SetVisibility(true);
        Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        Mesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
        Mesh->SetUseCCD(true);
        Mesh->SetSimulatePhysics(bEnablePhysics);
        Mesh->SetWorldScale3D(WorldVisualScale);
    }

    if (SkeletalMesh)
    {
        SkeletalMesh->SetVisibility(false);
        SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    ItemClass = UHelmetBaseItemData::StaticClass();
    AttachSocketName = TEXT("head");
}

void AHelmetBase::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    if (Mesh)
    {
        Mesh->SetWorldScale3D(WorldVisualScale);
    }

    if (ItemInstance)
    {
        ItemInstance->SizeInCellsX = FMath::Max(1, InventoryGridSize.X);
        ItemInstance->SizeInCellsY = FMath::Max(1, InventoryGridSize.Y);
    }
}

void AHelmetBase::BeginPlay()
{
    Super::BeginPlay();

    // Если это pickup, размещённый в уровне: создаём ItemInstance и применяем параметры
    if (!ItemInstance && ItemClass)
    {
        if (UInventoryItemData* NewData = NewObject<UInventoryItemData>(this, ItemClass))
        {
            NewData->SizeInCellsX = FMath::Max(1, InventoryGridSize.X);
            NewData->SizeInCellsY = FMath::Max(1, InventoryGridSize.Y);
            ItemInstance = NewData;
        }
    }

    // Фикс для BP-наследников:
    // Если в Blueprint на компоненте Mesh выставлен конкретный StaticMesh (например кепка),
    // не даём ItemInstance (по умолчанию куб) перетереть его при ApplyItemInstanceVisuals().
    if (UEquippableItemData* Eq = Cast<UEquippableItemData>(ItemInstance))
    {
        if (Mesh && Mesh->GetStaticMesh())
        {
            UStaticMesh* CurrentMesh = Mesh->GetStaticMesh();
            UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
            const bool bIsCube = (Cube && CurrentMesh == Cube) || CurrentMesh->GetPathName().Contains(TEXT("/Engine/BasicShapes/Cube"));

            if (!bIsCube)
            {
                // Подхватываем меш из BP в ItemData, чтобы ApplyItemInstanceVisuals оставил именно его
                Eq->EquippedMesh = CurrentMesh;
            }
        }
    }

    // При выбрасывании/спавне из инвентаря ItemInstance уже может быть выставлен — тогда просто применим визуал
    if (ItemInstance)
    {
        ApplyItemInstanceVisuals();
    }
}

