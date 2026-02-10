#include "ArmorBase.h"
#include "Components/StaticMeshComponent.h"
#include "BackToZaraysk/Inventory/EquippableItemData.h"

AArmorBase::AArmorBase()
{
    // Этот pickup использует статический меш по умолчанию
    if (Mesh)
    {
        // Видимый куб как placeholder бронеплиты
        UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
        Mesh->SetStaticMesh(Cube);
        Mesh->SetVisibility(true);
        Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        Mesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
        Mesh->SetUseCCD(true);
        Mesh->SetSimulatePhysics(bEnablePhysics);
        // Масштаб по умолчанию 1x1x1 (важно: Mesh — RootComponent, его масштаб влияет и на SkeletalMesh-детей)
        Mesh->SetWorldScale3D(FVector(1.0f, 1.0f, 1.0f));
    }
    if (SkeletalMesh)
    {
        SkeletalMesh->SetVisibility(false);
        SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    ItemClass = UArmorBaseItemData::StaticClass();
    AttachSocketName = TEXT("spine_02");
}

void AArmorBase::BeginPlay()
{
    Super::BeginPlay();
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("✅ Бронежилет создан"));
    }
}


