#include "BulletproofVestBege.h"
#include "BackToZaraysk/Inventory/EquippableItemData.h"
#include "Components/SkeletalMeshComponent.h"

ABulletproofVestBege::ABulletproofVestBege()
{
    // Используем новый ItemData (в нём задан EquippedMesh)
    ItemClass = UBulletproofVestBegeItemData::StaticClass();
    AttachSocketName = TEXT("spine_02");

    // ВАЖНО: нельзя создавать NewObject() внутри конструктора UObject/AActor без корректного механизма сабобъектов
    // (редактор падает на старте при создании CDO). Поэтому просто ставим меш напрямую для превью pickup в мире.
    if (SkeletalMesh)
    {
        const FString MeshPath = TEXT("/Game/BackToZaraysk/Core/Items/Equipment/SK_Bulletproof_Bege.SK_Bulletproof_Bege");
        if (USkeletalMesh* Sk = LoadObject<USkeletalMesh>(nullptr, *MeshPath))
        {
            SkeletalMesh->SetSkeletalMesh(Sk);
            SkeletalMesh->SetVisibility(true, true);
            SkeletalMesh->SetHiddenInGame(false, true);
        }
    }
    if (Mesh)
    {
        Mesh->SetVisibility(false, true);
        Mesh->SetHiddenInGame(true, true);
        Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // Коллизия/физика как у экипируемых предметов
    if (SkeletalMesh && SkeletalMesh->IsVisible())
    {
        SkeletalMesh->SetSimulatePhysics(bEnablePhysics);
        SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        SkeletalMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
        SkeletalMesh->SetUseCCD(true);
    }
    else if (Mesh)
    {
        Mesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
        Mesh->SetUseCCD(true);
    }
}

void ABulletproofVestBege::BeginPlay()
{
    Super::BeginPlay();
    // Если предмет заспавнен при выбросе с ItemInstance — применим визуал из инстанса
    if (ItemInstance)
    {
        ApplyItemInstanceVisuals();
    }
}


