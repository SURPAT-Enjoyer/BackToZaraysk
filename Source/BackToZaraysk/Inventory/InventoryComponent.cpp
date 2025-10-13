#include "InventoryComponent.h"
#include "EquippableItemData.h"
#include "BackToZaraysk/Components/EquipmentComponent.h"
#include "GameFramework/Character.h"

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

    // Перед снятием: если это рюкзак, и мы выбрасываем в мир — переносим его содержимое в PersistentStorage
    if (SlotType == Backpack && bDropToWorld)
    {
        if (UEquippableItemData* BackpackItem = Item)
        {
            // Прокопируем текущее EquipmentStorage в PersistentStorage
            if (TArray<TObjectPtr<UInventoryItemData>>* StorageItems = EquipmentStorage.Find(BackpackItem))
            {
                BackpackItem->PersistentStorage = *StorageItems;
            }
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
		
        // Если не выбрасываем в мир, восстанавливаем предмет на исходной позиции
        // Если выбросили рюкзак — очищаем оперативное хранилище, но оставляем PersistentStorage внутри ItemData
        if (SlotType == Backpack && bDropToWorld)
        {
            EquipmentStorage.Remove(Item);
        }

		// Если не выбрасываем в мир, восстанавливаем предмет на исходной позиции
		if (!bDropToWorld)
		{
			RestoreItemToPosition(Item);
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
        // Также удаляем из персистентного массива (если логика требует полного перемещения из рюкзака)
        Equipment->PersistentStorage.Remove(Item);
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

    // 1) Если предмет экипируемый и слот свободен — экипируем
    if (UEquippableItemData* Eq = Cast<UEquippableItemData>(Item))
    {
        if (!EquipmentSlots.Contains(Eq->EquipmentSlot))
        {
            // Добавим во временный инвентарь, чтобы использовать существующую логику
            BackpackItems.Add(Item);
            const bool bEquipped = EquipItemFromInventory(Eq);
            if (!bEquipped)
            {
                RemoveSpecificFromBackpack(Item);
            }
            return bEquipped;
        }
        // Если слот занят — идем по приоритету гридов ниже
    }

    // 2) Если предмет нельзя экипировать или слот занят — кладём в рюкзак, затем жилет, затем пояс, затем карманы
    // Рюкзак как обычный список (здесь без ширины/высоты) — проверим площадь доп.хранилища рюкзака, если он экипирован и имеет хранилище
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

    // Жилет как хранилище (если есть)
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

    // Пояс
    if (AddToGridLike(BeltStorageItems, BeltGridSize, Item))
    {
        return true;
    }

    // Карманы
    if (AddToGridLike(PocketsStorageItems, PocketsGridSize, Item))
    {
        return true;
    }

    // Не хватает места нигде — не подбираем
    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("⚠️ TryPickupItem: No space in any storage"));
    return false;
}







