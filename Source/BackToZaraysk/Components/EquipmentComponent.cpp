#include "EquipmentComponent.h"
#include "BackToZaraysk/Inventory/EquippableItemData.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"

UEquipmentComponent::UEquipmentComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UEquipmentComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Получаем меш персонажа
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        CharacterMesh = Character->GetMesh();
    }
    
    if (!CharacterMesh)
    {
        UE_LOG(LogTemp, Error, TEXT("EquipmentComponent: Character mesh not found!"));
    }
}

bool UEquipmentComponent::EquipItem(UEquippableItemData* ItemData)
{
    if (!ItemData)
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("❌ EquipmentComponent: ItemData is null"));
        return false;
    }
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, 
            FString::Printf(TEXT("🔍 EquipmentComponent::EquipItem called for: %s, Slot: %d"), 
                *ItemData->DisplayName.ToString(), (int32)ItemData->EquipmentSlot));
    }
    
    if (!CharacterMesh)
    {
        UE_LOG(LogTemp, Error, TEXT("EquipmentComponent: CharacterMesh is null"));
        return false;
    }
    
    EEquipmentSlotType SlotType = ItemData->EquipmentSlot;
    
    // Проверяем, не занят ли слот
    if (IsSlotOccupied(SlotType))
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
                TEXT("⚠️ Слот уже занят! Сначала снимите предмет."));
        }
        return false;
    }
    
    // Создаем и прикрепляем меш
    USkeletalMeshComponent* MeshComp = CreateEquipmentMeshComponent(SlotType, ItemData);
    if (!MeshComp)
    {
        UE_LOG(LogTemp, Error, TEXT("EquipmentComponent: Failed to create mesh component"));
        return false;
    }
    
    // Сохраняем данные
    EquippedItems.Add(SlotType, ItemData);
    EquipmentMeshComponents.Add(SlotType, MeshComp);
    ItemData->bIsEquipped = true;
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
            FString::Printf(TEXT("✅ Экипировано: %s"), *ItemData->DisplayName.ToString()));
    }
    
    return true;
}

bool UEquipmentComponent::UnequipItem(EEquipmentSlotType SlotType)
{
    if (!IsSlotOccupied(SlotType))
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
                TEXT("⚠️ Слот пуст!"));
        }
        return false;
    }
    
    // Получаем данные предмета
    UEquippableItemData* ItemData = EquippedItems[SlotType];
    if (ItemData)
    {
        ItemData->bIsEquipped = false;
    }
    
    // Удаляем меш
    RemoveEquipmentMeshComponent(SlotType);
    
    // Удаляем из карты
    EquippedItems.Remove(SlotType);
    EquipmentMeshComponents.Remove(SlotType);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
            FString::Printf(TEXT("✅ Снято: %s"), 
                ItemData ? *ItemData->DisplayName.ToString() : TEXT("Предмет")));
    }
    
    return true;
}

UEquippableItemData* UEquipmentComponent::GetEquippedItem(EEquipmentSlotType SlotType) const
{
    const UEquippableItemData* const* ItemPtr = EquippedItems.Find(SlotType);
    if (ItemPtr)
    {
        return const_cast<UEquippableItemData*>(*ItemPtr);
    }
    return nullptr;
}

bool UEquipmentComponent::IsSlotOccupied(EEquipmentSlotType SlotType) const
{
    return EquippedItems.Contains(SlotType);
}

USkeletalMeshComponent* UEquipmentComponent::CreateEquipmentMeshComponent(EEquipmentSlotType SlotType, UEquippableItemData* ItemData)
{
    if (!ItemData)
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("❌ CreateEquipmentMeshComponent: ItemData is null"));
        return nullptr;
    }
    
    if (!ItemData->EquippedMesh)
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
            FString::Printf(TEXT("❌ CreateEquipmentMeshComponent: EquippedMesh is null for %s"), *ItemData->DisplayName.ToString()));
        return nullptr;
    }
    
    if (!CharacterMesh)
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("❌ CreateEquipmentMeshComponent: CharacterMesh is null"));
        return nullptr;
    }
    
    // Создаем новый компонент меша
    FString ComponentName = FString::Printf(TEXT("EquipmentMesh_%d"), (int32)SlotType);
    USkeletalMeshComponent* MeshComp = NewObject<USkeletalMeshComponent>(GetOwner(), FName(*ComponentName));
    
    if (!MeshComp)
    {
        UE_LOG(LogTemp, Error, TEXT("EquipmentComponent: Failed to create mesh component"));
        return nullptr;
    }
    
    // Настраиваем компонент
    MeshComp->SetSkeletalMesh(ItemData->EquippedMesh);
    MeshComp->SetLeaderPoseComponent(CharacterMesh); // Синхронизация со скелетом персонажа
    MeshComp->RegisterComponent();
    
    // Прикрепляем к персонажу
    if (ItemData->AttachSocketName != NAME_None && CharacterMesh->DoesSocketExist(ItemData->AttachSocketName))
    {
        MeshComp->AttachToComponent(CharacterMesh, 
            FAttachmentTransformRules::SnapToTargetNotIncludingScale, 
            ItemData->AttachSocketName);
    }
    else
    {
        // Если сокет не указан или не найден, прикрепляем к корню меша
        MeshComp->AttachToComponent(CharacterMesh, 
            FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    }
    
    // Применяем относительный трансформ
    MeshComp->SetRelativeTransform(ItemData->RelativeTransform);
    
    // Отключаем коллизию для экипированного предмета
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    
    UE_LOG(LogTemp, Log, TEXT("EquipmentComponent: Created mesh component for slot %d"), (int32)SlotType);
    
    return MeshComp;
}

void UEquipmentComponent::RemoveEquipmentMeshComponent(EEquipmentSlotType SlotType)
{
    if (USkeletalMeshComponent** MeshCompPtr = EquipmentMeshComponents.Find(SlotType))
    {
        USkeletalMeshComponent* MeshComp = *MeshCompPtr;
        if (MeshComp)
        {
            MeshComp->DestroyComponent();
        }
    }
}

