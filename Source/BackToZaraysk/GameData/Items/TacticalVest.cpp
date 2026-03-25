#include "TacticalVest.h"
#include "Components/SkeletalMeshComponent.h"
#include "BackToZaraysk/Inventory/EquippableItemData.h"

ATacticalVest::ATacticalVest()
{
    // Настраиваем тактический жилет для использования скелетного меша
    if (SkeletalMesh)
    {
        // Показываем скелетный меш, скрываем статический
        SkeletalMesh->SetVisibility(true);
        if (Mesh)
        {
            Mesh->SetVisibility(false);
            Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
        
        // Настраиваем физику для подбора с земли
        SkeletalMesh->SetSimulatePhysics(bEnablePhysics);
        SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        SkeletalMesh->SetCollisionResponseToAllChannels(ECR_Block);
        SkeletalMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap); // Для подбора
        
        // Настраиваем коллизию для взаимодействия
        SkeletalMesh->SetCollisionObjectType(ECC_WorldDynamic);
        SkeletalMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    }
    
    // Устанавливаем класс предмета
    ItemClass = UTacticalVestItemData::StaticClass();
    
    // Настройки сокета для жилета
    AttachSocketName = TEXT("spine_03");
}

void ATacticalVest::BeginPlay()
{
    Super::BeginPlay();
    
    // Логирование для отладки
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            TEXT("✅ Тактический жилет создан"));
    }
}

