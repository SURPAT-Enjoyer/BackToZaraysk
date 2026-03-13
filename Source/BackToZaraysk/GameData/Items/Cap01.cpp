#include "Cap01.h"
#include "Components/StaticMeshComponent.h"
#include "BackToZaraysk/Inventory/EquippableItemData.h"

ACap01::ACap01()
{
    // Используем специализированные данные предмета (модель/иконка/слот)
    ItemClass = UCap01ItemData::StaticClass();
    AttachSocketName = TEXT("head");
    WorldVisualScale = FVector(1.0f, 1.0f, 1.0f);

    // Визуал pickup в мире — настоящая кепка
    if (Mesh)
    {
        UStaticMesh* CapMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/BackToZaraysk/Core/Items/Equipment/SM_Cap_Bege.SM_Cap_Bege"));
        if (CapMesh)
        {
            Mesh->SetStaticMesh(CapMesh);
        }
        Mesh->SetVisibility(true);
        Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        Mesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
        Mesh->SetUseCCD(true);
        Mesh->SetSimulatePhysics(bEnablePhysics);
        Mesh->SetWorldScale3D(WorldVisualScale);
    }
}

void ACap01::OnConstruction(const FTransform& Transform)
{
    // ВАЖНО: BP может переопределить ItemClass в Defaults. Принудительно фиксируем его на Cap01,
    // иначе ApplyItemInstanceVisuals может выставить куб из HelmetBase.
    ItemClass = UCap01ItemData::StaticClass();
    AttachSocketName = TEXT("head");
    WorldVisualScale = FVector(1.0f, 1.0f, 1.0f);

    Super::OnConstruction(Transform);

    if (Mesh)
    {
        // Гарантируем масштаб 1:1:1 и кепку как меш у пикапа
        Mesh->SetWorldScale3D(WorldVisualScale);
        if (UStaticMesh* CapMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/BackToZaraysk/Core/Items/Equipment/SM_Cap_Bege.SM_Cap_Bege")))
        {
            Mesh->SetStaticMesh(CapMesh);
        }
    }
}

void ACap01::BeginPlay()
{
    // До базового BeginPlay фиксируем ItemClass, чтобы AHelmetBase::BeginPlay создал правильный ItemInstance.
    ItemClass = UCap01ItemData::StaticClass();
    AttachSocketName = TEXT("head");

    Super::BeginPlay();

    // После базового BeginPlay ещё раз гарантируем, что визуал взят из Cap01, а не из куба.
    if (ItemInstance && !ItemInstance->IsA(UCap01ItemData::StaticClass()))
    {
        // Сохраним размеры, если их меняли в редакторе
        const int32 SX = FMath::Max(1, ItemInstance->SizeInCellsX);
        const int32 SY = FMath::Max(1, ItemInstance->SizeInCellsY);

        ItemInstance = NewObject<UCap01ItemData>(this, UCap01ItemData::StaticClass());
        if (ItemInstance)
        {
            ItemInstance->SizeInCellsX = SX;
            ItemInstance->SizeInCellsY = SY;
        }
    }

    ApplyItemInstanceVisuals();
}

