#include "EquipmentComponent.h"
#include "BackToZaraysk/Inventory/EquippableItemData.h"
#include "BackToZaraysk/Inventory/InventoryItemData.h"
#include "BackToZaraysk/GameData/Items/ArmorModSideGrid.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Character.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Components/CapsuleComponent.h"
#include "BackToZaraysk/Inventory/InventoryBlueprintLibrary.h"
#include "BackToZaraysk/GameData/Items/EquipmentBase.h"

static constexpr float ArmorModCellSize = 5.0f;

UEquipmentComponent::UEquipmentComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    SetComponentTickEnabled(false); // включаем только когда нужно (Helmet follow)
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

void UEquipmentComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (BoneFollowBySlot.Num() == 0 || !CharacterMesh)
    {
        if (IsComponentTickEnabled())
        {
            SetComponentTickEnabled(false);
        }
        return;
    }

    // Ручное следование сокетам (нужно, когда меш персонажа скрыт и аттач к нему визуально "не работает")
    for (const TPair<TEnumAsByte<EEquipmentSlotType>, FBoneFollowEntry>& Pair : BoneFollowBySlot)
    {
        const EEquipmentSlotType Slot = Pair.Key;
        const FBoneFollowEntry& Entry = Pair.Value;

        USceneComponent* const* CompPtr = EquipmentMeshComponents.Find(Slot);
        USceneComponent* Comp = CompPtr ? *CompPtr : nullptr;
        if (!Comp) continue;

        const FName Socket = Entry.SocketName;
        if (Socket == NAME_None) continue;

        FTransform SocketWorld = CharacterMesh->GetComponentTransform();
        if (CharacterMesh->DoesSocketExist(Socket))
        {
            SocketWorld = CharacterMesh->GetSocketTransform(Socket, RTS_World);
        }

        const FTransform DesiredWorld = Entry.RelativeToSocket * SocketWorld;
        Comp->SetWorldTransform(DesiredWorld, false, nullptr, ETeleportType::TeleportPhysics);
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

    // Если экипирована бронь с модами — прикрепить визуалы модов к мешу брони
    if (SlotType == Armor && ItemData->bIsModdable && ItemData->InstalledMods.Num() > 0)
    {
        RefreshArmorModAttachments(MeshComp, ItemData);
    }
    
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
            if (SlotType != Vest) // Vest спавнится в InventoryComponent (там своя логика/диагностика)
            {
                if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
                {
                    FVector ViewLoc; FRotator ViewRot;
                    Character->GetActorEyesViewPoint(ViewLoc, ViewRot);
                    FVector SpawnLoc = ViewLoc + ViewRot.Vector() * 80.f + FVector(0.f, 0.f, 100.f);
                    // Для рюкзаков — дропаем стабильно от позиции персонажа, не от камеры
                    if (SlotType == Backpack)
                    {
                        SpawnLoc = Character->GetActorLocation() + Character->GetActorForwardVector() * 80.f;
                    }
                    // Для бронежилета — спавним от позиции кости груди (spine_03), чтобы дроп был примерно на уровне груди
                    if (SlotType == Armor)
                    {
                        if (USkeletalMeshComponent* M = Character->GetMesh())
                        {
                            const FName ChestSocket(TEXT("spine_03"));
                            const FName RootSocket(TEXT("root"));
                            if (M->DoesSocketExist(ChestSocket))
                            {
                                SpawnLoc = M->GetSocketLocation(ChestSocket) + Character->GetActorForwardVector() * 80.f;
                            }
                            else if (M->DoesSocketExist(RootSocket))
                            {
                                SpawnLoc = M->GetSocketLocation(RootSocket) + Character->GetActorForwardVector() * 80.f;
                            }
                            else
                            {
                                SpawnLoc = Character->GetActorLocation() + Character->GetActorForwardVector() * 80.f;
                            }
                        }
                    }
                    // Для головных уборов — также спавним от root, чтобы дроп был стабильным (без зависимости от камеры)
                    else if (SlotType == Helmet)
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
                        TSubclassOf<AActor> DropClass = GetPickupClassForItem_Internal(ItemData);
                        if (DropClass)
                        {
                            if (AActor* SpawnedActor = World->SpawnActor<AActor>(DropClass, SpawnLoc, SpawnRot, Params))
                            {
                                if (AEquipmentBase* AsEquip = Cast<AEquipmentBase>(SpawnedActor))
                                {
                                    AsEquip->ItemInstance = ItemData;
                                    // Для брони сразу гасим статический меш (плейсхолдер/куб), чтобы не было фантома и деформации
                                    if (SlotType == Armor && AsEquip->Mesh)
                                    {
                                        AsEquip->Mesh->SetVisibility(false, true);
                                        AsEquip->Mesh->SetHiddenInGame(true, true);
                                        AsEquip->Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                                        AsEquip->Mesh->SetSimulatePhysics(false);
                                    }
                                    AsEquip->ApplyItemInstanceVisuals();
                                    AsEquip->SetActorHiddenInGame(false);
                                    if (SlotType == Armor)
                                    {
                                        UEquippableItemData* EqData = Cast<UEquippableItemData>(ItemData);
                                        if (EqData && EqData->bIsModdable && EqData->InstalledMods.Num() > 0)
                                        {
                                            USceneComponent* MeshToAttach = (AsEquip->SkeletalMesh && AsEquip->SkeletalMesh->GetSkeletalMeshAsset()) ? (USceneComponent*)AsEquip->SkeletalMesh : (USceneComponent*)AsEquip->Mesh;
                                            if (MeshToAttach)
                                                AttachArmorModsToMesh(AsEquip, MeshToAttach, EqData);
                                        }
                                        // Повторно фиксируем: только скелетный меш виден и с физикой, статический — скрыт и без физики
                                        if (AsEquip->Mesh)
                                        {
                                            AsEquip->Mesh->SetVisibility(false, true);
                                            AsEquip->Mesh->SetHiddenInGame(true, true);
                                            AsEquip->Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                                            AsEquip->Mesh->SetSimulatePhysics(false);
                                        }
                                    }
                                    // Включаем физику только у видимого меша. Для брони с модами НЕ включаем симуляцию скелета —
                                    // иначе по-костная физика (Physics Asset) даёт рэгдолл и сильную деформацию меша (чёрная масса, растягивание).
                                    UEquippableItemData* EqDataForPhysics = Cast<UEquippableItemData>(ItemData);
                                    const bool bArmorWithMods = (SlotType == Armor && EqDataForPhysics && EqDataForPhysics->InstalledMods.Num() > 0);
                                    const bool bSkeletalVisible = AsEquip->SkeletalMesh && AsEquip->SkeletalMesh->GetSkeletalMeshAsset() && AsEquip->SkeletalMesh->IsVisible();
                                    if (bSkeletalVisible && AsEquip->SkeletalMesh)
                                    {
                                        AsEquip->SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                                        AsEquip->SkeletalMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
                                        AsEquip->SkeletalMesh->SetUseCCD(true);
                                        if (!bArmorWithMods && AsEquip->SkeletalMesh->GetPhysicsAsset())
                                        {
                                            AsEquip->SkeletalMesh->SetSimulatePhysics(true);
                                            AsEquip->SkeletalMesh->SetEnableGravity(true);
                                            AsEquip->SkeletalMesh->AddImpulse(FVector(0.f, 0.f, 150.f), NAME_None, true);
                                        }
                                        if (AsEquip->Mesh)
                                        {
                                            AsEquip->Mesh->SetVisibility(false, true);
                                            AsEquip->Mesh->SetHiddenInGame(true, true);
                                            AsEquip->Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                                            AsEquip->Mesh->SetSimulatePhysics(false);
                                        }
                                    }
                                    else if (AsEquip->Mesh)
                                    {
                                        AsEquip->Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                                        AsEquip->Mesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
                                        AsEquip->Mesh->SetSimulatePhysics(true);
                                        AsEquip->Mesh->SetEnableGravity(true);
                                        AsEquip->Mesh->SetUseCCD(true);
                                        AsEquip->Mesh->AddImpulse(FVector(0.f, 0.f, 150.f), NAME_None, true);
                                        if (AsEquip->SkeletalMesh)
                                        {
                                            AsEquip->SkeletalMesh->SetVisibility(false, true);
                                            AsEquip->SkeletalMesh->SetHiddenInGame(true, true);
                                            AsEquip->SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                                            AsEquip->SkeletalMesh->SetSimulatePhysics(false);
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

void UEquipmentComponent::RefreshEquippedArmorMods()
{
    UEquippableItemData* ArmorData = GetEquippedItem(Armor);
    USceneComponent** MeshPtr = EquipmentMeshComponents.Find(Armor);
    if (ArmorData && MeshPtr && *MeshPtr)
        RefreshArmorModAttachments(*MeshPtr, ArmorData);
}

void UEquipmentComponent::SetupDroppedEquipmentVisual(AEquipmentBase* Equip, UInventoryItemData* ItemData)
{
    if (!Equip || !ItemData) return;
    Equip->ItemInstance = ItemData;
    UEquippableItemData* EqData = Cast<UEquippableItemData>(ItemData);
    const bool bIsArmor = (EqData && EqData->EquipmentSlot == Armor);
    const bool bArmorWithMods = (bIsArmor && EqData->InstalledMods.Num() > 0);

    if (bIsArmor && Equip->Mesh)
    {
        // Для брони в мире статический Mesh используем только как физическую оболочку,
        // поэтому на старте скрываем его – визуал будет у SkeletalMesh.
        Equip->Mesh->SetVisibility(false, true);
        Equip->Mesh->SetHiddenInGame(true, true);
        Equip->Mesh->SetSimulatePhysics(false);
        // Коллизию не отключаем полностью: для варианта "броня с модами" используем Mesh как физику.
        Equip->Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    Equip->ApplyItemInstanceVisuals();
    if (bIsArmor && Equip->Mesh)
    {
        Equip->Mesh->SetVisibility(false, true);
        Equip->Mesh->SetHiddenInGame(true, true);
        Equip->Mesh->SetSimulatePhysics(false);
        // Коллизию для Mesh окончательно настраиваем ниже, в зависимости от наличия модов.
    }
    if (bIsArmor && bArmorWithMods)
    {
        USceneComponent* MeshToAttach = (Equip->SkeletalMesh && Equip->SkeletalMesh->GetSkeletalMeshAsset()) ? (USceneComponent*)Equip->SkeletalMesh : (USceneComponent*)Equip->Mesh;
        if (MeshToAttach)
            AttachArmorModsToMesh(Equip, MeshToAttach, EqData);
    }
    if (bIsArmor)
    {
        Equip->SetActorHiddenInGame(false);

        if (Equip->SkeletalMesh && Equip->SkeletalMesh->GetSkeletalMeshAsset())
        {
            // Всегда даём коллизию для визуала брони
            Equip->SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            Equip->SkeletalMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
            Equip->SkeletalMesh->SetUseCCD(true);

            if (!bArmorWithMods && Equip->SkeletalMesh->GetPhysicsAsset())
            {
                // Обычная броня без модов – используем физику скелетного меша
                Equip->SkeletalMesh->SetSimulatePhysics(true);
                Equip->SkeletalMesh->SetEnableGravity(true);
                Equip->SkeletalMesh->AddImpulse(FVector(0.f, 0.f, 150.f), NAME_None, true);
            }
            else
            {
                // Для брони с модами отключаем рэгдолл, но не даём провалиться:
                // визуал – SkeletalMesh без симуляции, физика – через статический Mesh.
                Equip->SkeletalMesh->SetSimulatePhysics(false);
                Equip->SkeletalMesh->SetEnableGravity(false);

                if (Equip->Mesh)
                {
                    Equip->Mesh->SetVisibility(false, true);
                    Equip->Mesh->SetHiddenInGame(false, true); // невидим, но активен
                    Equip->Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                    Equip->Mesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
                    Equip->Mesh->SetSimulatePhysics(true);
                    Equip->Mesh->SetEnableGravity(true);
                    Equip->Mesh->SetUseCCD(true);
                    Equip->Mesh->AddImpulse(FVector(0.f, 0.f, 150.f), NAME_None, true);
                }
            }
        }
    }
    else
    {
        Equip->SetActorHiddenInGame(false);
        if (Equip->Mesh)
        {
            Equip->Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            Equip->Mesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
            Equip->Mesh->SetSimulatePhysics(true);
            Equip->Mesh->SetEnableGravity(true);
            Equip->Mesh->SetUseCCD(true);
            Equip->Mesh->AddImpulse(FVector(0.f, 0.f, 150.f), NAME_None, true);
        }
        if (Equip->SkeletalMesh)
        {
            Equip->SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            Equip->SkeletalMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
            Equip->SkeletalMesh->SetUseCCD(true);
            if (Equip->SkeletalMesh->GetPhysicsAsset())
            {
                Equip->SkeletalMesh->SetSimulatePhysics(true);
                Equip->SkeletalMesh->SetEnableGravity(true);
            }
        }
        if (EqData && EqData->EquipmentSlot == Vest && Equip->SkeletalMesh)
        {
            USkeletalMesh* ForcedMesh = nullptr;
            if (const UTacticalVestItemData* VestCDO = GetDefault<UTacticalVestItemData>())
                ForcedMesh = Cast<USkeletalMesh>(VestCDO->EquippedMesh);
            if (!ForcedMesh)
                ForcedMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/Insurgent_2/Mesh/Separate_Parts/SK_ChestRigSmall.SK_ChestRigSmall"));
            if (ForcedMesh)
            {
                Equip->SkeletalMesh->SetSkeletalMesh(ForcedMesh);
                Equip->SkeletalMesh->SetVisibility(true, true);
                Equip->SkeletalMesh->SetHiddenInGame(false, true);
                Equip->SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                Equip->SkeletalMesh->SetSimulatePhysics(true);
                Equip->SkeletalMesh->SetEnableGravity(true);
            }
            if (Equip->Mesh)
            {
                Equip->Mesh->SetVisibility(false, true);
                Equip->Mesh->SetHiddenInGame(true, true);
                Equip->Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                Equip->Mesh->SetSimulatePhysics(false);
            }
        }
    }
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
    
    // Прикрепляем к персонажу.
    // ВАЖНО: для Helmet используем ручное следование сокету, чтобы визуал работал даже если CharacterMesh скрыт (OwnerNoSee/Visibility).
    if (SlotType == Helmet)
    {
        USceneComponent* Root = GetOwner() ? GetOwner()->GetRootComponent() : nullptr;
        if (!Root)
        {
            Root = CharacterMesh;
        }

        // Чтобы сокеты считались, даже если меш скрыт
        CharacterMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

        // Ставим в правильное место в мире и аттачим к Root с KeepWorld
        FTransform SocketWorld = CharacterMesh->GetComponentTransform();
        const FName SocketName = (ItemData->AttachSocketName != NAME_None) ? ItemData->AttachSocketName : FName(TEXT("head"));
        if (CharacterMesh->DoesSocketExist(SocketName))
        {
            SocketWorld = CharacterMesh->GetSocketTransform(SocketName, RTS_World);
        }
        const FTransform DesiredWorld = ItemData->RelativeTransform * SocketWorld;
        Created->AttachToComponent(Root, FAttachmentTransformRules::KeepWorldTransform);
        Created->SetWorldTransform(DesiredWorld);

        BoneFollowBySlot.Add(SlotType, FBoneFollowEntry{SocketName, ItemData->RelativeTransform});
        SetComponentTickEnabled(true);

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green,
                FString::Printf(TEXT("🧢 Helmet visual: root-attached + bone-follow socket=%s"), *SocketName.ToString()));
        }
    }
    else
    {
        if (ItemData->AttachSocketName != NAME_None && CharacterMesh->DoesSocketExist(ItemData->AttachSocketName))
        {
            Created->AttachToComponent(CharacterMesh,
                FAttachmentTransformRules::SnapToTargetNotIncludingScale,
                ItemData->AttachSocketName);
        }
        else
        {
            Created->AttachToComponent(CharacterMesh,
                FAttachmentTransformRules::SnapToTargetNotIncludingScale);
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
        // Для Helmet мы уже выставили WorldTransform относительно сокета, а компонент может быть не на CharacterMesh.
        if (SlotType != Helmet)
        {
            C->SetRelativeTransform(ItemData->RelativeTransform);
        }
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
        // Копируем флаги видимости, чтобы не попадать в "не видно вообще" из-за OwnerNoSee/OnlyOwnerSee на CharacterMesh
        if (CharacterMesh)
        {
            Prim->SetOnlyOwnerSee(CharacterMesh->bOnlyOwnerSee);
            Prim->SetOwnerNoSee(CharacterMesh->bOwnerNoSee);
        }
        Prim->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Prim->SetVisibility(true, true);
        Prim->SetHiddenInGame(false, true);
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

void UEquipmentComponent::RefreshArmorModAttachments(USceneComponent* ArmorMeshComponent, UEquippableItemData* ArmorItemData)
{
    if (!ArmorMeshComponent || !ArmorItemData) return;

    // Удаляем старые визуалы модов (дочерние компоненты с именем ArmorMod_*)
    TArray<USceneComponent*> Children = ArmorMeshComponent->GetAttachChildren();
    for (USceneComponent* Child : Children)
    {
        if (Child && Child->GetName().StartsWith(TEXT("ArmorMod_")))
            Child->DestroyComponent();
    }

    if (ArmorItemData->ModSideGrids.Num() == 0) return;

    for (int32 i = 0; i < ArmorItemData->InstalledMods.Num(); ++i)
    {
        const FInstalledArmorMod& Inst = ArmorItemData->InstalledMods[i];
        if (!Inst.ModItemData || Inst.SideIndex < 0 || Inst.SideIndex >= ArmorItemData->ModSideGrids.Num()) continue;

        UObject* ModMeshObj = Inst.ModItemData->WorldMesh;
        if (!ModMeshObj || !ModMeshObj->IsValidLowLevel()) continue;

        const FArmorModSideGrid& G = ArmorItemData->ModSideGrids[Inst.SideIndex];
        const FIntPoint Foot = UEquippableItemData::GetModFootprint(Inst.ModItemData);
        const int32 SizeX = FMath::Max(1, Foot.X);
        const int32 SizeY = FMath::Max(1, Foot.Y);
        const float GridCenterX = (Inst.CellX + SizeX * 0.5f) * ArmorModCellSize;
        const float GridCenterY = (Inst.CellY + SizeY * 0.5f) * ArmorModCellSize;
        const FVector GridCenterPos = G.Origin + GridCenterX * G.AxisRight + GridCenterY * G.AxisUp;
        const FVector Normal = FVector::CrossProduct(G.AxisRight, G.AxisUp).GetSafeNormal();

        FVector LocalPos;
        if (const UEquipModBaseItemData* ModData = Cast<UEquipModBaseItemData>(Inst.ModItemData)) if (!ModData->AttachmentGridOrigin.IsNearlyZero())
        {
            const FVector OriginOffset = ModData->AttachmentGridOrigin.X * G.AxisRight + ModData->AttachmentGridOrigin.Y * G.AxisUp + ModData->AttachmentGridOrigin.Z * Normal;
            LocalPos = GridCenterPos - OriginOffset;
        }
        else
        {
            // Пивот меша в центре: ставим компонент в центр области, чтобы мод визуально центрировался
            LocalPos = GridCenterPos;
        }

        FMatrix RotMat(G.AxisRight, G.AxisUp, Normal, FVector::ZeroVector);
        FRotator LocalRot = RotMat.Rotator();
        FTransform LocalTM(LocalRot, LocalPos, FVector(1.f, 1.f, 1.f));

        USceneComponent* ModComp = nullptr;
        if (USkeletalMesh* Sk = Cast<USkeletalMesh>(ModMeshObj))
        {
            USkeletalMeshComponent* SkComp = NewObject<USkeletalMeshComponent>(GetOwner(), FName(*FString::Printf(TEXT("ArmorMod_%d"), i)));
            if (SkComp) { SkComp->SetSkeletalMesh(Sk); ModComp = SkComp; }
        }
        else if (UStaticMesh* St = Cast<UStaticMesh>(ModMeshObj))
        {
            UStaticMeshComponent* StComp = NewObject<UStaticMeshComponent>(GetOwner(), FName(*FString::Printf(TEXT("ArmorMod_%d"), i)));
            if (StComp) { StComp->SetStaticMesh(St); ModComp = StComp; }
        }
        if (!ModComp) continue;

        ModComp->RegisterComponent();
        ModComp->AttachToComponent(ArmorMeshComponent, FAttachmentTransformRules::KeepRelativeTransform);
        ModComp->SetRelativeTransform(LocalTM);
        if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(ModComp))
        {
            Prim->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            Prim->SetVisibility(true, true);
            Prim->SetHiddenInGame(false, true);
        }
    }
}

void UEquipmentComponent::AttachArmorModsToMesh(AActor* Owner, USceneComponent* ArmorMeshComponent, UEquippableItemData* ArmorItemData)
{
    if (!Owner || !ArmorMeshComponent || !ArmorItemData || ArmorItemData->ModSideGrids.Num() == 0) return;
    if (!ArmorItemData->bIsModdable || ArmorItemData->InstalledMods.Num() == 0) return;

    TArray<USceneComponent*> Children = ArmorMeshComponent->GetAttachChildren();
    for (USceneComponent* Child : Children)
        if (Child && Child->GetName().StartsWith(TEXT("ArmorMod_")))
            Child->DestroyComponent();

    for (int32 i = 0; i < ArmorItemData->InstalledMods.Num(); ++i)
    {
        const FInstalledArmorMod& Inst = ArmorItemData->InstalledMods[i];
        if (!Inst.ModItemData || Inst.SideIndex < 0 || Inst.SideIndex >= ArmorItemData->ModSideGrids.Num()) continue;

        UObject* ModMeshObj = Inst.ModItemData->WorldMesh;
        if (!ModMeshObj || !ModMeshObj->IsValidLowLevel()) continue;

        const FArmorModSideGrid& G = ArmorItemData->ModSideGrids[Inst.SideIndex];
        const FIntPoint Foot = UEquippableItemData::GetModFootprint(Inst.ModItemData);
        const int32 SizeX = FMath::Max(1, Foot.X);
        const int32 SizeY = FMath::Max(1, Foot.Y);
        const float GridCenterX = (Inst.CellX + SizeX * 0.5f) * ArmorModCellSize;
        const float GridCenterY = (Inst.CellY + SizeY * 0.5f) * ArmorModCellSize;
        const FVector GridCenterPos = G.Origin + GridCenterX * G.AxisRight + GridCenterY * G.AxisUp;
        const FVector Normal = FVector::CrossProduct(G.AxisRight, G.AxisUp).GetSafeNormal();

        FVector LocalPos;
        if (const UEquipModBaseItemData* ModData = Cast<UEquipModBaseItemData>(Inst.ModItemData)) if (!ModData->AttachmentGridOrigin.IsNearlyZero())
        {
            const FVector OriginOffset = ModData->AttachmentGridOrigin.X * G.AxisRight + ModData->AttachmentGridOrigin.Y * G.AxisUp + ModData->AttachmentGridOrigin.Z * Normal;
            LocalPos = GridCenterPos - OriginOffset;
        }
        else
        {
            // Пивот меша в центре: ставим компонент в центр области, чтобы мод визуально центрировался
            LocalPos = GridCenterPos;
        }

        FMatrix RotMat(G.AxisRight, G.AxisUp, Normal, FVector::ZeroVector);
        FRotator LocalRot = RotMat.Rotator();
        FTransform LocalTM(LocalRot, LocalPos, FVector(1.f, 1.f, 1.f));

        USceneComponent* ModComp = nullptr;
        if (USkeletalMesh* Sk = Cast<USkeletalMesh>(ModMeshObj))
        {
            USkeletalMeshComponent* SkComp = NewObject<USkeletalMeshComponent>(Owner, FName(*FString::Printf(TEXT("ArmorMod_%d"), i)));
            if (SkComp) { SkComp->SetSkeletalMesh(Sk); ModComp = SkComp; }
        }
        else if (UStaticMesh* St = Cast<UStaticMesh>(ModMeshObj))
        {
            UStaticMeshComponent* StComp = NewObject<UStaticMeshComponent>(Owner, FName(*FString::Printf(TEXT("ArmorMod_%d"), i)));
            if (StComp) { StComp->SetStaticMesh(St); ModComp = StComp; }
        }
        if (!ModComp) continue;

        ModComp->SetupAttachment(ArmorMeshComponent);
        ModComp->SetRelativeTransform(LocalTM);
        ModComp->RegisterComponent();
        if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(ModComp))
        {
            Prim->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            Prim->SetVisibility(true, true);
            Prim->SetHiddenInGame(false, true);
        }
    }
}

void UEquipmentComponent::RemoveEquipmentMeshComponent(EEquipmentSlotType SlotType)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    // Сначала уничтожаем все компоненты с визуалами модов (ArmorMod_*) на владельце — и дочерние к мешу, и «забытые» на персонаже
    TArray<UActorComponent*> AllComps;
    Owner->GetComponents(AllComps);
    TArray<UActorComponent*> ToDestroy;
    for (UActorComponent* Comp : AllComps)
    {
        if (Comp && Comp->GetName().StartsWith(TEXT("ArmorMod_")))
            ToDestroy.Add(Comp);
    }
    for (UActorComponent* Comp : ToDestroy)
    {
        if (IsValid(Comp))
            Comp->DestroyComponent();
    }

    if (USceneComponent** MeshCompPtr = EquipmentMeshComponents.Find(SlotType))
    {
        USceneComponent* MeshComp = *MeshCompPtr;
        if (MeshComp)
        {
            // Дочерние к мешу (на случай если не попали в обход по владельцу)
            TArray<USceneComponent*> Children = MeshComp->GetAttachChildren();
            for (USceneComponent* Child : Children)
            {
                if (Child && IsValid(Child) && Child->GetName().StartsWith(TEXT("ArmorMod_")))
                    Child->DestroyComponent();
            }
            MeshComp->DestroyComponent();
        }
    }

    BoneFollowBySlot.Remove(SlotType);
    if (BoneFollowBySlot.Num() == 0)
    {
        SetComponentTickEnabled(false);
    }
}

