#include "PickupBackpack.h"
#include "Components/StaticMeshComponent.h"
#include "BackToZaraysk/Inventory/EquippableItemData.h"
#include "BackToZaraysk/Inventory/InventoryComponent.h"
#include "BackToZaraysk/Inventory/EquipmentSlotType.h"

APickupBackpack::APickupBackpack()
{
    // Загружаем куб как базовую модель
    if (UStaticMesh* SM = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
    {
        Mesh->SetStaticMesh(SM);
    }
    
    // Устанавливаем белый материал
    if (UMaterialInterface* M = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/EngineMaterials/DefaultWhiteGrid.DefaultWhiteGrid")))
    {
        Mesh->SetMaterial(0, M);
    }
    
    // Масштабируем куб в форму параллелепипеда рюкзака
    // Размеры: ширина 0.8, глубина 0.4, высота 1.2
    Mesh->SetRelativeScale3D(FVector(0.8f, 0.4f, 1.2f));
    
    // Настройки физики
    Mesh->SetSimulatePhysics(true);
    Mesh->SetEnableGravity(true);
    
    // Устанавливаем класс данных рюкзака
    ItemClass = UBackpackItemData::StaticClass();
}

bool APickupBackpack::CanBePickedUp(UInventoryComponent* InventoryComponent) const
{
    if (!InventoryComponent) return false;
    
    // Проверяем, есть ли уже экипированный рюкзак
    UEquippableItemData* CurrentBackpack = Cast<UEquippableItemData>(InventoryComponent->GetEquippedItem(Backpack));
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, 
            FString::Printf(TEXT("🔍 APickupBackpack::CanBePickedUp - Current backpack: %s"), 
                CurrentBackpack ? TEXT("Found") : TEXT("None")));
    }
    
    // Можно подобрать, если нет экипированного рюкзака
    return CurrentBackpack == nullptr;
}

void APickupBackpack::OnPickedUp(UInventoryComponent* InventoryComponent)
{
    if (!InventoryComponent) return;
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("🎒 APickupBackpack::OnPickedUp - Auto-equipping backpack"));
    }
    
    // Создаем или используем существующий экземпляр данных предмета (с сохранённым содержимым)
    UInventoryItemData* Data = nullptr;
    if (ItemInstance)
    {
        Data = ItemInstance;
    }
    else if (ItemClass)
    {
        UObject* NewObj = NewObject<UObject>(this, ItemClass);
        Data = Cast<UInventoryItemData>(NewObj);
    }
    
    if (Data && Cast<UEquippableItemData>(Data))
    {
        UEquippableItemData* EquipData = Cast<UEquippableItemData>(Data);
        // Гарантируем корректные флаги и размер хранилища для рюкзака по умолчанию
        if (UBackpackItemData* BackpackData = Cast<UBackpackItemData>(EquipData))
        {
            BackpackData->bHasAdditionalStorage = true;
            if (BackpackData->AdditionalGridSize.X <= 0 || BackpackData->AdditionalGridSize.Y <= 0)
            {
                BackpackData->AdditionalGridSize = FIntPoint(8, 6);
            }
        }

        // Сначала добавляем предмет в инвентарь, чтобы EquipItemFromInventory смог его найти
        InventoryComponent->AddToBackpack(EquipData);
        
        // Автоматически экипируем рюкзак
        if (InventoryComponent->EquipItemFromInventory(EquipData))
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                    FString::Printf(TEXT("✅ Backpack auto-equipped: %s"), *EquipData->DisplayName.ToString()));
            }
            // После успешной экипировки ItemInstance принадлежит инвентарю — разрываем ссылку у Pickup, чтобы не было двойного владения
            ItemInstance = nullptr;
            
            // Уничтожаем pickup объект
            Destroy();
        }
        else
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("❌ Failed to auto-equip backpack"));
            }
        }
    }
}
