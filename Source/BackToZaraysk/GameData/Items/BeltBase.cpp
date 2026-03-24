#include "BeltBase.h"
#include "Components/StaticMeshComponent.h"
#include "BackToZaraysk/Inventory/EquippableItemData.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"

ABeltBase::ABeltBase()
{
    EnsureStorageGrids();

    if (Mesh)
    {
        if (!Mesh->GetStaticMesh())
        {
            if (UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
            {
                Mesh->SetStaticMesh(Cube);
            }
        }
        Mesh->SetVisibility(true, true);
        Mesh->SetHiddenInGame(false, true);
        Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        Mesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
        Mesh->SetWorldScale3D(FVector(0.35f, 0.18f, 0.10f));
        Mesh->SetSimulatePhysics(bEnablePhysics);
    }

    if (SkeletalMesh)
    {
        SkeletalMesh->SetVisibility(false, true);
        SkeletalMesh->SetHiddenInGame(true, true);
        SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    ItemClass = UBeltBaseItemData::StaticClass();
    AttachSocketName = AttachBoneName;

}

void ABeltBase::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    EnsureStorageGrids();
    ApplyEditorOverridesToItemInstance();
}

void ABeltBase::BeginPlay()
{
    Super::BeginPlay();

    bool bCreatedItemInstanceHere = false;
    if (!ItemInstance && ItemClass)
    {
        if (UInventoryItemData* NewData = NewObject<UInventoryItemData>(this, ItemClass))
        {
            ItemInstance = NewData;
            bCreatedItemInstanceHere = true;
        }
    }

    ApplyEditorOverridesToItemInstance();

    if (UEquippableItemData* Eq = Cast<UEquippableItemData>(ItemInstance))
    {
        if (bCreatedItemInstanceHere && Mesh && Mesh->GetStaticMesh())
        {
            Eq->EquippedMesh = Mesh->GetStaticMesh();
        }
    }

    if (ItemInstance)
    {
        ApplyItemInstanceVisuals();
    }
}

#if WITH_EDITOR
void ABeltBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    EnsureStorageGrids();
    ApplyEditorOverridesToItemInstance();
}
#endif

void ABeltBase::EnsureStorageGrids()
{
    StorageGridCount = FMath::Max(1, StorageGridCount);

    if (StorageGridSizes.Num() == 0 && StorageGridOffsets.Num() == 0)
    {
        StorageGridSizes = { FIntPoint(4, 2) };
        StorageGridOffsets = { FIntPoint(0, 0) };
        StorageGridCount = 1;
    }

    if (StorageGridSizes.Num() != StorageGridCount)
    {
        StorageGridSizes.SetNum(StorageGridCount);
    }
    if (StorageGridOffsets.Num() != StorageGridCount)
    {
        StorageGridOffsets.SetNum(StorageGridCount);
    }

    for (FIntPoint& P : StorageGridSizes)
    {
        P.X = FMath::Max(1, P.X);
        P.Y = FMath::Max(1, P.Y);
    }
}

void ABeltBase::ApplyEditorOverridesToItemInstance()
{
    if (!ItemInstance) return;

    ItemInstance->SizeInCellsX = FMath::Max(1, InventorySizeX);
    ItemInstance->SizeInCellsY = FMath::Max(1, InventorySizeY);

    if (UEquippableItemData* Eq = Cast<UEquippableItemData>(ItemInstance))
    {
        Eq->EquipmentSlot = Belt;
        Eq->bIsModdable = bIsModdable;
        Eq->ModSideGrids = ModSideGrids;

        Eq->bHasAdditionalStorage = true;
        Eq->AdditionalGrids = StorageGridSizes;
        Eq->AdditionalGridOffsets = StorageGridOffsets;

        int32 MaxX = 0;
        int32 MaxY = 0;
        for (int32 i = 0; i < Eq->AdditionalGrids.Num(); ++i)
        {
            const FIntPoint Sz = Eq->AdditionalGrids[i];
            const FIntPoint Off = Eq->AdditionalGridOffsets.IsValidIndex(i) ? Eq->AdditionalGridOffsets[i] : FIntPoint(0, 0);
            MaxX = FMath::Max(MaxX, FMath::Max(0, Off.X) + FMath::Max(1, Sz.X));
            MaxY = FMath::Max(MaxY, FMath::Max(0, Off.Y) + FMath::Max(1, Sz.Y));
        }
        Eq->AdditionalGridSize = FIntPoint(FMath::Max(1, MaxX), FMath::Max(1, MaxY));

        FName EffectiveSocket = AttachBoneName;
        if (EffectiveSocket.IsNone())
        {
            EffectiveSocket = (!AttachSocketName.IsNone() && AttachSocketName != TEXT("None"))
                ? AttachSocketName
                : FName(TEXT("spine_01"));
        }

        Eq->AttachSocketName = EffectiveSocket;
        AttachBoneName = EffectiveSocket;
        AttachSocketName = EffectiveSocket;
    }

    ApplyInventoryIconOverride();
}

void ABeltBase::ApplyInventoryIconOverride()
{
    if (!ItemInstance) return;
    ItemInstance->Icon = (InventoryIconTexture && InventoryIconTexture->IsValidLowLevel()) ? InventoryIconTexture : nullptr;
}

