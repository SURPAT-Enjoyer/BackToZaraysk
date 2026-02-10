#include "EquipmentComponent.h"
#include "BackToZaraysk/Inventory/EquippableItemData.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Character.h"
#include "Engine/SkeletalMesh.h"
// For backpack pickup spawn on drop
#include "BackToZaraysk/GameData/Items/Test/PickupBackpack.h"
// For generic pickup mapping
#include "BackToZaraysk/Inventory/InventoryItemData.h"
#include "BackToZaraysk/GameData/Items/EquipmentBase.h"

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
    
    // Создаем и прикрепляем визуальный компонент (skeletal/static)
    USceneComponent* MeshComp = CreateEquipmentMeshComponent(SlotType, ItemData);
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
        
    if (USceneComponent** MeshPtr = EquipmentMeshComponents.Find(SlotType))
        {
        USceneComponent* Mesh = *MeshPtr;
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
        
        // Если нужно выбросить в мир — спавним правильный pickup и переносим в него текущий экземпляр ItemData
        if (bDropToWorld)
        {
            if (SlotType == Backpack)
            {
                if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
                {
                    const FVector SpawnLoc = Character->GetActorLocation() + Character->GetActorForwardVector() * 80.f;
                    const FRotator SpawnRot = Character->GetActorRotation();
                    FActorSpawnParameters Params; Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
                    if (UWorld* World = GetWorld())
                    {
                        if (APickupBackpack* Pickup = World->SpawnActor<APickupBackpack>(APickupBackpack::StaticClass(), SpawnLoc, SpawnRot, Params))
                        {
                            Pickup->ItemInstance = ItemData; // сохраняем все данные, включая PersistentStorage
                        }
                    }
                }
            }
            else if (SlotType != Vest) // Vest спавнится в InventoryComponent (там своя логика/диагностика)
            {
                if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
                {
                    FVector ViewLoc; FRotator ViewRot;
                    Character->GetActorEyesViewPoint(ViewLoc, ViewRot);
                    FVector SpawnLoc = ViewLoc + ViewRot.Vector() * 80.f + FVector(0.f, 0.f, 100.f);
                    // Для бронежилета — спавним от позиции root на меше (иначе может улетать вверх из-за вьюпоинта/камеры)
                    if (SlotType == Armor)
                    {
                        if (USkeletalMeshComponent* M = Character->GetMesh())
                        {
                            const FName RootSocket(TEXT("root"));
                            if (M->DoesSocketExist(RootSocket))
                            {
                                SpawnLoc = M->GetSocketLocation(RootSocket) + Character->GetActorForwardVector() * 80.f;
                            }
                            else
                            {
                                SpawnLoc = Character->GetActorLocation() + Character->GetActorForwardVector() * 80.f;
                            }
                        }
                    }
                    const FRotator SpawnRot = ViewRot;
                    FActorSpawnParameters Params;
                    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
                    if (UWorld* World = GetWorld())
                    {
                        extern TSubclassOf<AActor> GetPickupClassForItem_Internal(const UInventoryItemData* ItemData);
                        TSubclassOf<AActor> DropClass = GetPickupClassForItem_Internal(ItemData);
                        if (DropClass)
                        {
                            if (AActor* SpawnedActor = World->SpawnActor<AActor>(DropClass, SpawnLoc, SpawnRot, Params))
                            {
                                if (AEquipmentBase* AsEquip = Cast<AEquipmentBase>(SpawnedActor))
                                {
                                    AsEquip->ItemInstance = ItemData;
                                    AsEquip->ApplyItemInstanceVisuals();
                                    AsEquip->SetActorHiddenInGame(false);
                                    // Делаем физику/коллизию стабильной: держим её на статическом Mesh даже если визуал на SkeletalMesh
                                    if (AsEquip->Mesh)
                                    {
                                        AsEquip->Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                                        AsEquip->Mesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
                                        AsEquip->Mesh->SetSimulatePhysics(true);
                                        AsEquip->Mesh->SetEnableGravity(true);
                                        AsEquip->Mesh->SetUseCCD(true);
                                        AsEquip->Mesh->AddImpulse(FVector(0.f, 0.f, 150.f), NAME_None, true);
                                    }
                                    if (AsEquip->SkeletalMesh)
                                    {
                                        AsEquip->SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                                        AsEquip->SkeletalMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
                                        AsEquip->SkeletalMesh->SetUseCCD(true);
                                        if (AsEquip->SkeletalMesh->GetPhysicsAsset())
                                        {
                                            AsEquip->SkeletalMesh->SetSimulatePhysics(true);
                                            AsEquip->SkeletalMesh->SetEnableGravity(true);
                                        }
                                    }
                                }
                                else if (APickupBase* Spawned = Cast<APickupBase>(SpawnedActor))
                                {
                                    Spawned->ItemInstance = ItemData;
                                    Spawned->ApplyItemInstanceVisuals();
                                    if (Spawned->Mesh)
                                    {
                                        Spawned->Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                                        Spawned->Mesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
                                        Spawned->Mesh->SetSimulatePhysics(true);
                                        Spawned->Mesh->SetEnableGravity(true);
                                        Spawned->Mesh->SetUseCCD(true);
                                        Spawned->Mesh->AddImpulse(FVector(0.f, 0.f, 150.f), NAME_None, true);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange, 
                        FString::Printf(TEXT("🗑️ Предмет %s будет выброшен в мир"), 
                            *ItemData->DisplayName.ToString()));
                }
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

USceneComponent* UEquipmentComponent::CreateEquipmentMeshComponent(EEquipmentSlotType SlotType, UEquippableItemData* ItemData)
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
    USceneComponent* Created = nullptr;
    if (USkeletalMesh* Sk = Cast<USkeletalMesh>(ItemData->EquippedMesh))
    {
        USkeletalMeshComponent* SkComp = NewObject<USkeletalMeshComponent>(GetOwner(), FName(*ComponentName));
        if (!SkComp) return nullptr;
        SkComp->SetSkeletalMesh(Sk);
        Created = SkComp;
    }
    else if (UStaticMesh* St = Cast<UStaticMesh>(ItemData->EquippedMesh))
    {
        UStaticMeshComponent* StComp = NewObject<UStaticMeshComponent>(GetOwner(), FName(*ComponentName));
        if (!StComp) return nullptr;
        StComp->SetStaticMesh(St);
        Created = StComp;
    }
    else
    {
        return nullptr;
    }
    
    // Устанавливаем материал для лучшей видимости (только для скелетных)
    if (USkeletalMeshComponent* AsSk = Cast<USkeletalMeshComponent>(Created))
    {
        AsSk->SetMaterial(0, nullptr); // Используем материал по умолчанию
    }

    // Регистрируем компонент ПЕРЕД прикреплением
    Created->RegisterComponent();
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("🔧 Mesh component registered"));
    }
    
    // Прикрепляем к персонажу
    if (ItemData->AttachSocketName != NAME_None && CharacterMesh->DoesSocketExist(ItemData->AttachSocketName))
    {
        // Для скелетных мешей используем SnapToTarget для правильного прикрепления к сокету
        Created->AttachToComponent(CharacterMesh, 
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
        Created->AttachToComponent(CharacterMesh, 
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
    if (USkeletalMeshComponent* AsSk = Cast<USkeletalMeshComponent>(Created))
    {
        USkeletalMesh* SkeletalMeshForSkeleton = Cast<USkeletalMesh>(ItemData->EquippedMesh);
        if (SkeletalMeshForSkeleton && SkeletalMeshForSkeleton->GetSkeleton())
        {
            AsSk->SetLeaderPoseComponent(CharacterMesh);
        }
    }
    
    // Применяем относительный трансформ
    if (USceneComponent* C = Created)
    {
        C->SetRelativeTransform(ItemData->RelativeTransform);
    }
    
    if (GEngine)
    {
        FTransform AppliedTransform = Created->GetRelativeTransform();
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, 
            FString::Printf(TEXT("🔧 Applied RelativeTransform: Loc(%.2f,%.2f,%.2f) Rot(%.2f,%.2f,%.2f) Scale(%.2f,%.2f,%.2f)"), 
                AppliedTransform.GetLocation().X, AppliedTransform.GetLocation().Y, AppliedTransform.GetLocation().Z,
                AppliedTransform.Rotator().Pitch, AppliedTransform.Rotator().Yaw, AppliedTransform.Rotator().Roll,
                AppliedTransform.GetScale3D().X, AppliedTransform.GetScale3D().Y, AppliedTransform.GetScale3D().Z));
    }
    
    // Отключаем коллизию для экипированного предмета
    if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(Created))
    {
        Prim->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Prim->SetVisibility(true);
        Prim->SetHiddenInGame(false);
        Prim->UpdateBounds();
        Prim->MarkRenderTransformDirty();
    }
    
    // Финальная диагностика
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
            FString::Printf(TEXT("✅ Mesh component created successfully for slot %d"), (int32)SlotType));
        
        // Проверяем видимость
        if (Created && Created->IsVisible())
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("✅ Mesh component is visible"));
        }
        else
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("❌ Mesh component is not visible"));
        }
        
        // Дополнительная диагностика трансформа
        FTransform RelativeTransform = Created->GetRelativeTransform();
        FTransform WorldTransform = Created->GetComponentTransform();
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
    
    return Created;
}

void UEquipmentComponent::RemoveEquipmentMeshComponent(EEquipmentSlotType SlotType)
{
    if (USceneComponent** MeshCompPtr = EquipmentMeshComponents.Find(SlotType))
    {
        USceneComponent* MeshComp = *MeshCompPtr;
        if (MeshComp)
        {
            MeshComp->DestroyComponent();
        }
    }
}

