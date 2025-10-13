#include "EquipmentComponent.h"
#include "BackToZaraysk/Inventory/EquippableItemData.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Engine/SkeletalMesh.h"

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
    
    // Проверяем валидность слота
    if (ItemData->EquipmentSlot == EEquipmentSlotType::None)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
                FString::Printf(TEXT("❌ EquipmentComponent: Invalid slot type for item '%s'"), *ItemData->DisplayName.ToString()));
        }
        return false;
    }
    
    // Проверяем, что предмет не экипирован уже
    if (ItemData->bIsEquipped)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
                FString::Printf(TEXT("⚠️ Item '%s' is already equipped"), *ItemData->DisplayName.ToString()));
        }
        return false;
    }
    
    // Проверяем валидность меша для экипировки
    if (!ItemData->EquippedMesh || !ItemData->EquippedMesh->IsValidLowLevel())
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
                FString::Printf(TEXT("❌ Item '%s' has invalid EquippedMesh - cannot equip"), *ItemData->DisplayName.ToString()));
        }
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
        
        // Дополнительная диагностика
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, 
            FString::Printf(TEXT("🔧 EquipmentMeshComponents count: %d"), EquipmentMeshComponents.Num()));
        
        if (USkeletalMeshComponent** MeshPtr = EquipmentMeshComponents.Find(SlotType))
        {
            USkeletalMeshComponent* Mesh = *MeshPtr;
            if (Mesh)
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, 
                    FString::Printf(TEXT("🔧 Mesh component exists: %s, Visible: %s"), 
                        *Mesh->GetName(), Mesh->IsVisible() ? TEXT("Yes") : TEXT("No")));
            }
        }
    }
    
    return true;
}

bool UEquipmentComponent::UnequipItem(EEquipmentSlotType SlotType, bool bDropToWorld)
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
        
        // Если нужно выбросить в мир
        if (bDropToWorld)
        {
            // TODO: Добавить логику выброса предмета в мир
            // Пока что просто логируем
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange, 
                    FString::Printf(TEXT("🗑️ Предмет %s будет выброшен в мир"), 
                        *ItemData->DisplayName.ToString()));
            }
        }
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
    
    // Проверяем, что меш валиден
    if (!IsValid(ItemData->EquippedMesh))
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
            FString::Printf(TEXT("❌ CreateEquipmentMeshComponent: EquippedMesh is invalid for %s"), *ItemData->DisplayName.ToString()));
        return nullptr;
    }
    
    // Дополнительная проверка на существование меша
    if (!ItemData->EquippedMesh->IsValidLowLevel())
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
            FString::Printf(TEXT("❌ CreateEquipmentMeshComponent: EquippedMesh is not valid low level for %s"), *ItemData->DisplayName.ToString()));
        return nullptr;
    }
    
    if (!CharacterMesh)
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("❌ CreateEquipmentMeshComponent: CharacterMesh is null"));
        return nullptr;
    }
    
    if (!GetOwner())
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("❌ CreateEquipmentMeshComponent: Owner is null"));
        return nullptr;
    }
    
    // Создаем новый компонент меша
    FString ComponentName = FString::Printf(TEXT("EquipmentMesh_%d"), (int32)SlotType);
    USkeletalMeshComponent* MeshComp = NewObject<USkeletalMeshComponent>(GetOwner(), FName(*ComponentName));
    
    if (!MeshComp)
    {
        UE_LOG(LogTemp, Error, TEXT("EquipmentComponent: Failed to create mesh component"));
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("❌ Failed to create mesh component"));
        return nullptr;
    }
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
            FString::Printf(TEXT("🔧 Created mesh component: %s"), *ComponentName));
    }
    
    // Настраиваем компонент
    USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(ItemData->EquippedMesh);
    if (SkeletalMesh)
    {
        MeshComp->SetSkeletalMesh(SkeletalMesh);
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                FString::Printf(TEXT("🔧 Set SkeletalMesh: %s"), *SkeletalMesh->GetName()));
        }
    }
    else
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
                FString::Printf(TEXT("❌ Failed to cast EquippedMesh to USkeletalMesh: %s"), 
                    ItemData->EquippedMesh ? *ItemData->EquippedMesh->GetName() : TEXT("NULL")));
        }
    }
    
    // Устанавливаем материал для лучшей видимости
    MeshComp->SetMaterial(0, nullptr); // Используем материал по умолчанию

    // Регистрируем компонент ПЕРЕД прикреплением
    MeshComp->RegisterComponent();
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("🔧 Mesh component registered"));
    }
    
    // Прикрепляем к персонажу
    if (ItemData->AttachSocketName != NAME_None && CharacterMesh->DoesSocketExist(ItemData->AttachSocketName))
    {
        // Для скелетных мешей используем SnapToTarget для правильного прикрепления к сокету
        MeshComp->AttachToComponent(CharacterMesh, 
            FAttachmentTransformRules::SnapToTargetNotIncludingScale, 
            ItemData->AttachSocketName);
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                FString::Printf(TEXT("🔧 Attached to socket: %s (skeletal attachment)"), 
                    *ItemData->AttachSocketName.ToString()));
        }
    }
    else
    {
        // Прикрепляем к корню меша (к персонажу)
        MeshComp->AttachToComponent(CharacterMesh, 
            FAttachmentTransformRules::SnapToTargetNotIncludingScale);
        
        if (GEngine)
        {
            if (ItemData->AttachSocketName == NAME_None)
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                    TEXT("🔧 Attached to character root (no socket)"));
            }
            else
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
                    FString::Printf(TEXT("⚠️ Socket '%s' not found, attached to root"), 
                        *ItemData->AttachSocketName.ToString()));
            }
        }
    }
    
    // Устанавливаем LeaderPoseComponent для прикрепления скелета к скелету
    // LeaderPoseComponent нужен только для скелетных мешей, которые должны следовать анимации персонажа
    USkeletalMesh* SkeletalMeshForSkeleton = Cast<USkeletalMesh>(ItemData->EquippedMesh);
    if (SkeletalMeshForSkeleton && SkeletalMeshForSkeleton->GetSkeleton())
    {
        // Проверяем, что у меша есть скелет (это скелетный меш)
        MeshComp->SetLeaderPoseComponent(CharacterMesh);
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                TEXT("🔧 LeaderPoseComponent enabled for skeletal mesh"));
        }
    }
    else
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
                TEXT("⚠️ LeaderPoseComponent disabled - not a skeletal mesh"));
        }
    }
    
    // Применяем относительный трансформ
    MeshComp->SetRelativeTransform(ItemData->RelativeTransform);
    
    if (GEngine)
    {
        FTransform AppliedTransform = MeshComp->GetRelativeTransform();
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, 
            FString::Printf(TEXT("🔧 Applied RelativeTransform: Loc(%.2f,%.2f,%.2f) Rot(%.2f,%.2f,%.2f) Scale(%.2f,%.2f,%.2f)"), 
                AppliedTransform.GetLocation().X, AppliedTransform.GetLocation().Y, AppliedTransform.GetLocation().Z,
                AppliedTransform.Rotator().Pitch, AppliedTransform.Rotator().Yaw, AppliedTransform.Rotator().Roll,
                AppliedTransform.GetScale3D().X, AppliedTransform.GetScale3D().Y, AppliedTransform.GetScale3D().Z));
    }
    
    // Отключаем коллизию для экипированного предмета
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    
    // Принудительно делаем видимым
    MeshComp->SetVisibility(true);
    MeshComp->SetHiddenInGame(false);
    
    // Дополнительные настройки для гарантии видимости
    MeshComp->SetVisibility(true);
    MeshComp->MarkRenderStateDirty();
    
    // Принудительно обновляем трансформ
    MeshComp->UpdateBounds();
    MeshComp->MarkRenderTransformDirty();
    
    // Финальная диагностика
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
            FString::Printf(TEXT("✅ Mesh component created successfully for slot %d"), (int32)SlotType));
        
        // Проверяем видимость
        if (MeshComp->IsVisible())
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("✅ Mesh component is visible"));
        }
        else
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("❌ Mesh component is not visible"));
        }
        
        // Дополнительная диагностика трансформа
        FTransform RelativeTransform = MeshComp->GetRelativeTransform();
        FTransform WorldTransform = MeshComp->GetComponentTransform();
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, 
            FString::Printf(TEXT("🔧 RelativeTransform: Location(%s), Rotation(%s), Scale(%s)"), 
                *RelativeTransform.GetLocation().ToString(),
                *RelativeTransform.GetRotation().Rotator().ToString(),
                *RelativeTransform.GetScale3D().ToString()));
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, 
            FString::Printf(TEXT("🌍 WorldTransform: Location(%s), Rotation(%s), Scale(%s)"), 
                *WorldTransform.GetLocation().ToString(),
                *WorldTransform.GetRotation().Rotator().ToString(),
                *WorldTransform.GetScale3D().ToString()));
    }
    
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

