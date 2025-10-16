#include "InventoryComponent.h"
#include "EquippableItemData.h"
#include "BackToZaraysk/Components/EquipmentComponent.h"
#include "BackToZaraysk/GameData/Items/Test/PickupBase.h"
#include "BackToZaraysk/GameData/Items/EquipmentBase.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UInventoryComponent::AddToBackpack(UInventoryItemData* Item)
{
	if (!Item) return false;
	BackpackItems.Add(Item);
	return true;
}

UInventoryItemData* UInventoryComponent::RemoveLastFromBackpack()
{
	if (BackpackItems.Num() == 0) return nullptr;
	UInventoryItemData* Item = BackpackItems.Last();
	BackpackItems.RemoveAt(BackpackItems.Num() - 1);
	return Item;
}

bool UInventoryComponent::RemoveSpecificFromBackpack(UInventoryItemData* Item)
{
    if (!Item) return false;
    const int32 Index = BackpackItems.Find(Item);
    if (Index == INDEX_NONE) return false;
    
    // Очищаем сохраненную позицию при удалении
    ItemPositions.Remove(Item);
    
    BackpackItems.RemoveAt(Index);
    return true;
}

bool UInventoryComponent::EquipItemFromInventory(UEquippableItemData* Item)
{
	if (!Item) 
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("❌ EquipItemFromInventory: Item is null"));
		return false;
	}

	// Дополнительная диагностика предмета
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, 
			FString::Printf(TEXT("🔍 Equipping item: %s, Slot: %d, EquippedMesh: %s"), 
				*Item->DisplayName.ToString(), 
				(int32)Item->EquipmentSlot,
				Item->EquippedMesh ? TEXT("SET") : TEXT("NULL")));
	}

	// Проверяем, есть ли предмет в инвентаре
	if (!BackpackItems.Contains(Item))
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
				FString::Printf(TEXT("⚠️ Предмет '%s' не найден в инвентаре (размер инвентаря: %d)"), 
					*Item->DisplayName.ToString(), BackpackItems.Num()));
		}
		return false;
	}

	// Получаем компонент экипировки
	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	if (!Owner) 
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("❌ EquipItemFromInventory: Owner is not ACharacter"));
		return false;
	}

	UEquipmentComponent* EquipComp = Owner->FindComponentByClass<UEquipmentComponent>();
	if (!EquipComp)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("❌ EquipItemFromInventory: EquipmentComponent not found!"));
		return false;
	}

	// Проверяем, не занят ли уже слот
	if (EquipmentSlots.Contains(Item->EquipmentSlot))
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
				FString::Printf(TEXT("⚠️ Slot %d is already occupied by another item"), (int32)Item->EquipmentSlot));
		}
		return false;
	}

    // Если это рюкзак и в ItemData уже есть PersistenStorage (например, после повторного подбора), 
    // и в оперативном EquipmentStorage пусто — восстановим его перед экипировкой
    if (Item->EquipmentSlot == Backpack)
    {
        TArray<TObjectPtr<UInventoryItemData>>& StorageItems = EquipmentStorage.FindOrAdd(Item);
        if (StorageItems.Num() == 0 && Item->PersistentStorage.Num() > 0)
        {
            StorageItems = Item->PersistentStorage;
            // Восстановим и сохранённые позиции ячеек
            if (Item->PersistentCellByItem.Num() > 0)
            {
                for (auto& Kvp : Item->PersistentCellByItem)
                {
                    Item->StoredCellByItem.Add(Kvp.Key, Kvp.Value);
                }
            }
        }
    }

    // Экипируем предмет
	if (EquipComp->EquipItem(Item))
	{
		// Сохраняем позицию предмета перед удалением
		int32 ItemIndex = BackpackItems.Find(Item);
		if (ItemIndex != INDEX_NONE)
		{
			ItemPositions.Add(Item, ItemIndex);
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, 
					FString::Printf(TEXT("🔍 Saved position %d for item '%s'"), ItemIndex, *Item->DisplayName.ToString()));
			}
		}
		else
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
					FString::Printf(TEXT("❌ Failed to find item '%s' in backpack for position saving"), *Item->DisplayName.ToString()));
			}
		}
		
		// Удаляем из инвентаря и добавляем в слот
		RemoveSpecificFromBackpack(Item);
		EquipmentSlots.Add(Item->EquipmentSlot, Item);
		
		// Проверяем, что предмет действительно экипирован
		UEquippableItemData* EquippedItem = EquipComp->GetEquippedItem(Item->EquipmentSlot);
		if (EquippedItem == Item)
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
					FString::Printf(TEXT("✅ Successfully equipped: %s to slot %d (Total slots: %d)"), 
						*Item->DisplayName.ToString(), (int32)Item->EquipmentSlot, EquipmentSlots.Num()));
			}
			return true;
		}
		else
		{
			// Если экипировка не удалась, возвращаем предмет в инвентарь
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
					TEXT("❌ EquipmentComponent failed to equip item, reverting changes"));
			}
			EquipmentSlots.Remove(Item->EquipmentSlot);
			AddToBackpack(Item);
			return false;
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
				FString::Printf(TEXT("❌ Failed to equip: %s"), *Item->DisplayName.ToString()));
		}
	}

	return false;
}

bool UInventoryComponent::UnequipItemToInventory(EEquipmentSlotType SlotType, bool bDropToWorld)
{
	// Проверяем, занят ли слот
	UEquippableItemData** ItemPtr = EquipmentSlots.Find(SlotType);
	if (!ItemPtr || !(*ItemPtr))
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
				TEXT("⚠️ Слот пуст"));
		}
		return false;
	}

	UEquippableItemData* Item = *ItemPtr;

	// Получаем компонент экипировки
	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	if (!Owner) return false;

	UEquipmentComponent* EquipComp = Owner->FindComponentByClass<UEquipmentComponent>();
	if (!EquipComp)
	{
		UE_LOG(LogTemp, Error, TEXT("InventoryComponent: EquipmentComponent not found!"));
		return false;
	}

    // Перед снятием: если это рюкзак или жилет, и мы выбрасываем в мир — переносим содержимое в PersistentStorage
    if ((SlotType == Backpack || SlotType == Vest) && bDropToWorld)
    {
        if (UEquippableItemData* EquipItem = Item)
        {
            // Прокопируем текущее EquipmentStorage в PersistentStorage
            if (TArray<TObjectPtr<UInventoryItemData>>* StorageItems = EquipmentStorage.Find(EquipItem))
            {
                EquipItem->PersistentStorage = *StorageItems;
                // Копируем позиции ячеек из runtime-структуры в персистентную карту
                for (const TObjectPtr<UInventoryItemData>& It : *StorageItems)
                {
                    if (It && EquipItem->StoredCellByItem.Contains(It))
                    {
                        EquipItem->PersistentCellByItem.Add(It, EquipItem->StoredCellByItem[It]);
                    }
                }
            }
        }
    }

    // Если снимаем жилет и рюкзак не экипирован, то всегда выбрасываем в мир
    if (SlotType == Vest && !bDropToWorld)
    {
        if (GetEquippedItem(Backpack) == nullptr)
        {
            bDropToWorld = true;
        }
    }

    // Снимаем предмет
    if (EquipComp->UnequipItem(SlotType, bDropToWorld))
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, 
				FString::Printf(TEXT("🔍 UnequipItemToInventory: Item '%s' unequipped, bDropToWorld: %s"), 
					*Item->DisplayName.ToString(), bDropToWorld ? TEXT("true") : TEXT("false")));
		}
		
		// Удаляем из слота
		EquipmentSlots.Remove(SlotType);
		
        // Если выбросили рюкзак/жилет — очищаем оперативное хранилище, но оставляем PersistentStorage внутри ItemData
        if ((SlotType == Backpack || SlotType == Vest) && bDropToWorld)
        {
            EquipmentStorage.Remove(Item);
        }

        // Если не выбрасываем в мир
        if (!bDropToWorld)
        {
            // Особый случай: снимаем жилет — если надет рюкзак с хранилищем, кладём жилет туда, чтобы он был виден в UI
            if (SlotType == Vest)
            {
                if (UEquippableItemData* EquippedBackpack = GetEquippedItem(Backpack))
                {
                    if (EquippedBackpack->bHasAdditionalStorage)
                    {
                        if (AddToEquipmentStorage(EquippedBackpack, Item))
                        {
                            // Успешно поместили в хранилище рюкзака — выходим
                            return true;
                        }
                    }
                }
            }
            // Иначе — возвращаем в список инвентаря (общий бэкпак‑лист)
            RestoreItemToPosition(Item);
        }
        else
        {
            // Спавним Pickup для тех слотов, где визуальный компонент не делает этого сам (например, жилет)
            if (SlotType == Vest)
            {
                ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
                if (OwnerChar)
                {
                    if (UWorld* World = OwnerChar->GetWorld())
                    {
                        FVector ViewLoc; FRotator ViewRot; OwnerChar->GetActorEyesViewPoint(ViewLoc, ViewRot);
                        const FVector SpawnLoc = ViewLoc + ViewRot.Vector() * 80.f + FVector(0.f, 0.f, 100.f);
                        if (GEngine)
                        {
                            GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Yellow,
                                FString::Printf(TEXT("🧪 Drop Vest: Preparing spawn at (%.0f, %.0f, %.0f)"), SpawnLoc.X, SpawnLoc.Y, SpawnLoc.Z));
                        }
                        FActorSpawnParameters S; S.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
                        extern TSubclassOf<AActor> GetPickupClassForItem_Internal(const UInventoryItemData* ItemData);
                        TSubclassOf<AActor> DropClass = GetPickupClassForItem_Internal(Item);
                        if (GEngine)
                        {
                            GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Cyan,
                                FString::Printf(TEXT("🧪 Drop Vest: Class to spawn = %s"), *GetNameSafe(DropClass))); 
                        }
                        if (DropClass)
                        {
                            if (AActor* SpawnedActor = World->SpawnActor<AActor>(DropClass, SpawnLoc, ViewRot, S))
                            {
                                if (GEngine)
                                {
                                    GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green,
                                        FString::Printf(TEXT("✅ Drop Vest: Spawned %s at (%.0f, %.0f, %.0f)"), *SpawnedActor->GetName(), SpawnLoc.X, SpawnLoc.Y, SpawnLoc.Z));
                                }
                                if (APickupBase* Spawned = Cast<APickupBase>(SpawnedActor))
                                {
                                    Spawned->ItemInstance = Item;
                                    Spawned->ApplyItemInstanceVisuals();
                                    if (GEngine)
                                    {
                                        GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, TEXT("✅ Drop Vest: ItemInstance applied to spawned actor"));
                                    }
                                }
                                // Диагностика видимости/коллизии
                                const bool bHidden = SpawnedActor->IsHidden();
                                bool bAnyVisible = !bHidden;
                                if (AEquipmentBase* Eq = Cast<AEquipmentBase>(SpawnedActor))
                                {
                                    const bool bSkVisible = (Eq->SkeletalMesh && Eq->SkeletalMesh->IsVisible());
                                    const bool bStVisible = (Eq->Mesh && Eq->Mesh->IsVisible());
                                    bAnyVisible = bAnyVisible || bSkVisible || bStVisible;
                                    // Гарантируем видимость
                                    SpawnedActor->SetActorHiddenInGame(false);
                                    if (Eq->SkeletalMesh) { Eq->SkeletalMesh->SetVisibility(true, true); Eq->SkeletalMesh->SetHiddenInGame(false, true); }
                                    if (Eq->Mesh) { Eq->Mesh->SetVisibility(true, true); Eq->Mesh->SetHiddenInGame(false, true); }
                                    // Выводим фактическую позицию меша
                                    const FVector SkLoc = Eq->SkeletalMesh ? Eq->SkeletalMesh->GetComponentLocation() : FVector::ZeroVector;
                                    const FVector StLoc = Eq->Mesh ? Eq->Mesh->GetComponentLocation() : FVector::ZeroVector;
                                    if (GEngine)
                                    {
                                        GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Cyan,
                                            FString::Printf(TEXT("🧪 Drop Vest: Visible? ActorHidden=%s, SkeletalVisible=%s, StaticVisible=%s"),
                                                bHidden?TEXT("true"):TEXT("false"), bSkVisible?TEXT("true"):TEXT("false"), bStVisible?TEXT("true"):TEXT("false")));
                                        GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Magenta,
                                            FString::Printf(TEXT("🧪 Drop Vest: SkeletalLoc=(%.0f,%.0f,%.0f) StaticLoc=(%.0f,%.0f,%.0f)"),
                                                SkLoc.X, SkLoc.Y, SkLoc.Z, StLoc.X, StLoc.Y, StLoc.Z));
                                    }
                                }
                                else if (APickupBase* PB = Cast<APickupBase>(SpawnedActor))
                                {
                                    const bool bStVisible = (PB->Mesh && PB->Mesh->IsVisible());
                                    bAnyVisible = bAnyVisible || bStVisible;
                                    if (GEngine)
                                    {
                                        GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Cyan,
                                            FString::Printf(TEXT("🧪 Drop Vest: Visible? ActorHidden=%s, StaticVisible=%s"),
                                                bHidden?TEXT("true"):TEXT("false"), bStVisible?TEXT("true"):TEXT("false")));
                                    }
                                }
                                // Маркеры на месте спавна
                                DrawDebugSphere(World, SpawnLoc, 25.f, 16, FColor::Magenta, false, 8.0f);
                                DrawDebugPoint(World, SpawnLoc, 25.f, FColor::Red, false, 8.0f);
                                DrawDebugLine(World, SpawnLoc + FVector(0,0,150.f), SpawnLoc - FVector(0,0,150.f), FColor::Yellow, false, 8.0f, 0, 3.0f);
                                DrawDebugString(World, SpawnLoc + FVector(0,0,180.f), TEXT("VEST SPAWN"), nullptr, FColor::White, 8.0f, true);
                            }
                            else if (GEngine)
                            {
                                GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("❌ Drop Vest: SpawnActor returned nullptr"));
                            }
                        }
                    }
                }
            }
        }
		
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
				FString::Printf(TEXT("✅ UnequipItemToInventory: Item '%s' successfully unequipped"), 
					*Item->DisplayName.ToString()));
		}
		
		return true;
	}

	return false;
}

UEquippableItemData* UInventoryComponent::GetEquippedItem(EEquipmentSlotType SlotType) const
{
	const UEquippableItemData* const* ItemPtr = EquipmentSlots.Find(SlotType);
	if (ItemPtr)
	{
		return const_cast<UEquippableItemData*>(*ItemPtr);
	}
	return nullptr;
}

bool UInventoryComponent::RestoreItemToPosition(UInventoryItemData* Item)
{
	if (!Item) 
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("❌ RestoreItemToPosition: Item is null"));
		return false;
	}
	
	// Проверяем, что предмет не находится уже в инвентаре
	if (BackpackItems.Contains(Item))
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
				FString::Printf(TEXT("⚠️ Item '%s' is already in backpack"), *Item->DisplayName.ToString()));
		}
		return true; // Предмет уже в инвентаре, считаем успехом
	}
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, 
			FString::Printf(TEXT("🔍 Restoring item '%s', current backpack size: %d"), 
				*Item->DisplayName.ToString(), BackpackItems.Num()));
	}
	
	// Ищем сохраненную позицию
	int32* SavedPosition = ItemPositions.Find(Item);
	if (SavedPosition)
	{
		// Вставляем предмет на сохраненную позицию
		int32 Position = *SavedPosition;
		// Ограничиваем позицию размером массива
		Position = FMath::Clamp(Position, 0, BackpackItems.Num());
		BackpackItems.Insert(Item, Position);
		
		// Удаляем из карты позиций
		ItemPositions.Remove(Item);
		
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
				FString::Printf(TEXT("✅ Restored item '%s' to position %d (new backpack size: %d)"), 
					*Item->DisplayName.ToString(), Position, BackpackItems.Num()));
		}
		
		return true;
	}
	else
	{
		// Если позиция не найдена, добавляем в конец
		BackpackItems.Add(Item);
		
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
				FString::Printf(TEXT("⚠️ No saved position for '%s', added to end (new backpack size: %d)"), 
					*Item->DisplayName.ToString(), BackpackItems.Num()));
		}
		
		return true;
	}
}

bool UInventoryComponent::AddToEquipmentStorage(UEquippableItemData* Equipment, UInventoryItemData* Item)
{
	if (!Equipment || !Item)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("❌ AddToEquipmentStorage: Equipment or Item is null"));
		return false;
	}

    // Запрещаем класть предмет самого оборудования внутрь его же хранилища (рюкзак в рюкзак)
    if (Equipment == Item)
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("❌ Нельзя поместить рюкзак в самого себя"));
        return false;
    }
	
	// Проверяем, есть ли у экипировки дополнительное хранилище
	if (!Equipment->bHasAdditionalStorage)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("❌ Equipment has no additional storage"));
		return false;
	}
	
    // Получаем или создаем массив для этого экипированного предмета
    TArray<TObjectPtr<UInventoryItemData>>& StorageItems = EquipmentStorage.FindOrAdd(Equipment);
    // Синхронизируем с персистентным массивом в самом ItemData
    if (StorageItems.Num() == 0 && Equipment->PersistentStorage.Num() > 0)
    {
        StorageItems = Equipment->PersistentStorage;
    }
	
	// Проверяем, есть ли место (простая проверка по количеству ячеек)
	int32 TotalCells = Equipment->AdditionalGridSize.X * Equipment->AdditionalGridSize.Y;
	int32 UsedCells = 0;
	for (const auto& StoredItem : StorageItems)
	{
		if (StoredItem)
		{
			UsedCells += StoredItem->SizeInCellsX * StoredItem->SizeInCellsY;
		}
	}
	
	if (UsedCells + (Item->SizeInCellsX * Item->SizeInCellsY) > TotalCells)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
			FString::Printf(TEXT("❌ Not enough space in equipment storage. Used: %d, Need: %d, Total: %d"), 
				UsedCells, Item->SizeInCellsX * Item->SizeInCellsY, TotalCells));
		return false;
	}
	
    StorageItems.Add(Item);
    // Дублируем в персистентное хранилище, чтобы переживать выброс и повторное поднятие
    if (!Equipment->PersistentStorage.Contains(Item))
    {
        Equipment->PersistentStorage.Add(Item);
    }
    else
    {
        // Обновляем позиционный массив, если его ведём (простое соответствие индексам)
        // Здесь можно сохранять пары X,Y через StoredCellsXY
    }
	
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
		FString::Printf(TEXT("✅ Added '%s' to equipment storage. Items count: %d"), 
			*Item->DisplayName.ToString(), StorageItems.Num()));
	
	return true;
}

bool UInventoryComponent::RemoveFromEquipmentStorage(UEquippableItemData* Equipment, UInventoryItemData* Item)
{
	if (!Equipment || !Item)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("❌ RemoveFromEquipmentStorage: Equipment or Item is null"));
		return false;
	}
	
    TArray<TObjectPtr<UInventoryItemData>>* StorageItems = EquipmentStorage.Find(Equipment);
	if (!StorageItems)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("❌ No storage found for this equipment"));
		return false;
	}
	
	int32 RemovedCount = StorageItems->RemoveAll([Item](const TObjectPtr<UInventoryItemData>& StoredItem)
	{
		return StoredItem == Item;
	});
	
    if (RemovedCount > 0)
	{
        // Не удаляем из PersistentStorage: оно хранит состояние для выброса/подбора
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
			FString::Printf(TEXT("✅ Removed '%s' from equipment storage. Remaining items: %d"), 
				*Item->DisplayName.ToString(), StorageItems->Num()));
		return true;
	}
	else
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("❌ Item not found in equipment storage"));
		return false;
	}
}

bool UInventoryComponent::RemoveCompletelyFromEquipmentStorage(UEquippableItemData* Equipment, UInventoryItemData* Item)
{
    if (!Equipment || !Item) return false;
    bool bRemoved = false;
    TArray<TObjectPtr<UInventoryItemData>>* StorageItems = EquipmentStorage.Find(Equipment);
    if (StorageItems)
    {
        const int32 Before = StorageItems->Num();
        StorageItems->Remove(Item);
        bRemoved |= (Before != StorageItems->Num());
    }
    // Также удаляем из персистентного списка и карт позиций
    Equipment->PersistentStorage.Remove(Item);
    Equipment->StoredCellByItem.Remove(Item);
    Equipment->PersistentCellByItem.Remove(Item);
    return bRemoved;
}

bool UInventoryComponent::RemoveFromAnyStorage(UInventoryItemData* Item)
{
    if (!Item) return false;
    bool bRemoved = false;
    // Рюкзак
    bRemoved |= RemoveSpecificFromBackpack(Item);
    // Хранилища экипировки
    if (UEquippableItemData* EquippedBackpack = GetEquippedItem(Backpack))
    {
        bRemoved |= RemoveCompletelyFromEquipmentStorage(EquippedBackpack, Item);
    }
    if (UEquippableItemData* EquippedVest = GetEquippedItem(Vest))
    {
        bRemoved |= RemoveCompletelyFromEquipmentStorage(EquippedVest, Item);
    }
    // Пояс и карманы
    auto RemoveFromArray = [&](TArray<TObjectPtr<UInventoryItemData>>& Arr)
    {
        const int32 Before = Arr.Num();
        Arr.Remove(Item);
        return Arr.Num() != Before;
    };
    bRemoved |= RemoveFromArray(BeltStorageItems);
    bRemoved |= RemoveFromArray(Pocket1Items);
    bRemoved |= RemoveFromArray(Pocket2Items);
    bRemoved |= RemoveFromArray(Pocket3Items);
    bRemoved |= RemoveFromArray(Pocket4Items);

    return bRemoved;
}

TArray<UInventoryItemData*> UInventoryComponent::GetEquipmentStorageItems(UEquippableItemData* Equipment)
{
	TArray<UInventoryItemData*> Result;
	
	if (!Equipment)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("❌ GetEquipmentStorageItems: Equipment is null"));
		return Result;
	}
	
	TArray<TObjectPtr<UInventoryItemData>>* StorageItems = EquipmentStorage.Find(Equipment);
	if (StorageItems)
	{
		for (const auto& Item : *StorageItems)
		{
			if (Item)
			{
				Result.Add(Item);
			}
		}
	}
	
	return Result;
}

bool UInventoryComponent::SyncWithEquipmentComponent()
{
	// Получаем компонент экипировки
	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	if (!Owner) 
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("❌ SyncWithEquipmentComponent: Owner is not ACharacter"));
		return false;
	}

	UEquipmentComponent* EquipComp = Owner->FindComponentByClass<UEquipmentComponent>();
	if (!EquipComp)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("❌ SyncWithEquipmentComponent: EquipmentComponent not found!"));
		return false;
	}

	bool bSyncSuccessful = true;
	
	// Проверяем все слоты в EquipmentSlots
	for (auto& SlotPair : EquipmentSlots)
	{
		EEquipmentSlotType SlotType = SlotPair.Key;
		UEquippableItemData* Item = SlotPair.Value;
		
		// Проверяем, что предмет действительно экипирован в EquipmentComponent
		UEquippableItemData* EquippedItem = EquipComp->GetEquippedItem(SlotType);
		if (EquippedItem != Item)
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
					FString::Printf(TEXT("❌ Sync error: Slot %d mismatch! Inventory: %s, Equipment: %s"), 
						(int32)SlotType,
						Item ? *Item->DisplayName.ToString() : TEXT("NULL"),
						EquippedItem ? *EquippedItem->DisplayName.ToString() : TEXT("NULL")));
			}
			bSyncSuccessful = false;
		}
	}
	
	// Проверяем все экипированные предметы в EquipmentComponent
	for (int32 SlotIndex = 0; SlotIndex < 7; ++SlotIndex) // Все возможные слоты
	{
		EEquipmentSlotType SlotType = (EEquipmentSlotType)SlotIndex;
		if (EquipComp->IsSlotOccupied(SlotType))
		{
			UEquippableItemData* EquippedItem = EquipComp->GetEquippedItem(SlotType);
			if (!EquipmentSlots.Contains(SlotType))
			{
				if (GEngine)
				{
					GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
						FString::Printf(TEXT("❌ Sync error: Slot %d equipped in EquipmentComponent but not in InventoryComponent!"), 
							(int32)SlotType));
				}
				bSyncSuccessful = false;
			}
		}
	}
	
	if (bSyncSuccessful)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
				TEXT("✅ InventoryComponent and EquipmentComponent are synchronized"));
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
				TEXT("❌ Synchronization errors detected between InventoryComponent and EquipmentComponent"));
		}
	}
	
	return bSyncSuccessful;
}

bool UInventoryComponent::HasSpaceInGridLike(const FIntPoint& GridSize, const TArray<TObjectPtr<UInventoryItemData>>& Items, int32 ItemSizeX, int32 ItemSizeY) const
{
    // Простейшая проверка «по площади»: суммы размеров без раскладки
    int32 UsedCells = 0;
    for (const auto& It : Items)
    {
        if (It)
        {
            UsedCells += FMath::Max(1, It->SizeInCellsX) * FMath::Max(1, It->SizeInCellsY);
        }
    }
    const int32 TotalCells = GridSize.X * GridSize.Y;
    const int32 NeedCells = FMath::Max(1, ItemSizeX) * FMath::Max(1, ItemSizeY);
    return (UsedCells + NeedCells) <= TotalCells;
}

bool UInventoryComponent::AddToGridLike(TArray<TObjectPtr<UInventoryItemData>>& Items, const FIntPoint& GridSize, UInventoryItemData* Item)
{
    if (!Item) return false;
    if (!HasSpaceInGridLike(GridSize, Items, Item->SizeInCellsX, Item->SizeInCellsY)) return false;
    Items.Add(Item);
    return true;
}

bool UInventoryComponent::TryPickupItem(UInventoryItemData* Item)
{
    if (!Item)
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("❌ TryPickupItem: Item is null"));
        return false;
    }

    // Стандартизируем размеры минимум 1x1
    Item->SizeInCellsX = FMath::Max(1, Item->SizeInCellsX);
    Item->SizeInCellsY = FMath::Max(1, Item->SizeInCellsY);

    // 1) Если предмет экипируемый и слот свободен — экипируем
    if (UEquippableItemData* Eq = Cast<UEquippableItemData>(Item))
    {
        if (!EquipmentSlots.Contains(Eq->EquipmentSlot))
        {
            BackpackItems.Add(Item);
            const bool bEquipped = EquipItemFromInventory(Eq);
            if (!bEquipped)
            {
                RemoveSpecificFromBackpack(Item);
            }
            return bEquipped;
        }
    }

    // 2) Если экипирован рюкзак — пробуем доп. хранилище экипированного рюкзака
    if (UEquippableItemData* EquippedBackpack = GetEquippedItem(Backpack))
    {
        if (EquippedBackpack->bHasAdditionalStorage)
        {
            if (AddToEquipmentStorage(EquippedBackpack, Item))
            {
                return true;
            }
        }
    }

    // 3) Если в рюкзаке нет места или он не экипирован — пробуем карманы (по одному 1x1)
    const FIntPoint OneCell(1,1);
    if (Item->SizeInCellsX == 1 && Item->SizeInCellsY == 1)
    {
        if (AddToGridLike(Pocket1Items, OneCell, Item)) return true;
        if (AddToGridLike(Pocket2Items, OneCell, Item)) return true;
        if (AddToGridLike(Pocket3Items, OneCell, Item)) return true;
        if (AddToGridLike(Pocket4Items, OneCell, Item)) return true;
    }

    // 4) Если в карманах нет места — пробуем жилет
    if (UEquippableItemData* EquippedVest = GetEquippedItem(Vest))
    {
        if (EquippedVest->bHasAdditionalStorage)
        {
            if (AddToEquipmentStorage(EquippedVest, Item))
            {
                return true;
            }
        }
    }

    // Иначе — не подбираем
    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("⚠️ TryPickupItem: No space in pockets/backpack/vest"));
    return false;
}

bool UInventoryComponent::MoveItemToVest(UInventoryItemData* Item)
{
    if (!Item) return false;
    if (UEquippableItemData* EquippedVest = GetEquippedItem(Vest))
    {
        // Удаляем из всех возможных мест хранения
        RemoveSpecificFromBackpack(Item);
        if (UEquippableItemData* EquippedBackpack = GetEquippedItem(Backpack)) { RemoveFromEquipmentStorage(EquippedBackpack, Item); }
        RemoveFromEquipmentStorage(EquippedVest, Item);
        // Пытаемся положить в жилет
        if (EquippedVest->bHasAdditionalStorage)
        {
            return AddToEquipmentStorage(EquippedVest, Item);
        }
    }
    return false;
}

bool UInventoryComponent::MoveItemToPocket(int32 PocketIndex, UInventoryItemData* Item)
{
    if (!Item) return false;
    TArray<TObjectPtr<UInventoryItemData>>* Target = nullptr;
    switch (PocketIndex)
    {
        case 1: Target = &Pocket1Items; break;
        case 2: Target = &Pocket2Items; break;
        case 3: Target = &Pocket3Items; break;
        case 4: Target = &Pocket4Items; break;
        default: return false;
    }
    
    // Удаляем из всех других карманов
    Pocket1Items.Remove(Item);
    Pocket2Items.Remove(Item);
    Pocket3Items.Remove(Item);
    Pocket4Items.Remove(Item);
    
    // Удаляем из других хранилищ
    RemoveSpecificFromBackpack(Item);
    if (UEquippableItemData* EquippedVest = GetEquippedItem(Vest)) { RemoveCompletelyFromEquipmentStorage(EquippedVest, Item); }
    if (UEquippableItemData* EquippedBackpack = GetEquippedItem(Backpack)) { RemoveCompletelyFromEquipmentStorage(EquippedBackpack, Item); }
    
    return AddToGridLike(*Target, FIntPoint(1,1), Item);
}







