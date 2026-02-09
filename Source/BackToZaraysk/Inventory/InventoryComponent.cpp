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

    // Если это рюкзак или жилет и в ItemData уже есть PersistenStorage (например, после повторного подбора), 
    // и в оперативном EquipmentStorage пусто — восстановим его перед экипировкой
    if (Item->EquipmentSlot == Backpack || Item->EquipmentSlot == Vest)
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
            // Восстановим повороты
            for (auto& RotKvp : Item->PersistentRotByItem)
            {
                Item->StoredRotByItem.Add(RotKvp.Key, RotKvp.Value);
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
                    if (It && EquipItem->StoredRotByItem.Contains(It))
                    {
                        EquipItem->PersistentRotByItem.Add(It, EquipItem->StoredRotByItem[It]);
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
	
    // Избегаем дублей: если предмет уже в оперативном списке хранилища — не добавляем повторно
    if (!StorageItems.Contains(Item))
    {
        StorageItems.Add(Item);
    }
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
    Equipment->StoredRotByItem.Remove(Item);
    Equipment->PersistentRotByItem.Remove(Item);
    return bRemoved;
}

bool UInventoryComponent::RemoveFromAnyStorage(UInventoryItemData* Item)
{
    if (!Item) return false;
    bool bRemoved = false;
    
    // Проверяем, не является ли предмет экипированным
    if (UEquippableItemData* EquippedItem = Cast<UEquippableItemData>(Item))
    {
        if (EquippedItem->bIsEquipped)
        {
            // Снимаем экипированный предмет без выбрасывания в мир
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange, 
                    FString::Printf(TEXT("🔧 Removing equipped item from slot: %d"), (int32)EquippedItem->EquipmentSlot));
            }
            
            // Снимаем с экипировки и убираем визуал с персонажа
            if (UEquippableItemData** ItemPtr = EquipmentSlots.Find(EquippedItem->EquipmentSlot))
            {
                if (*ItemPtr == EquippedItem)
                {
                    // Проверяем состояние EquippedMesh перед снятием
                    if (GEngine)
                    {
                        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
                            FString::Printf(TEXT("🔍 Before unequip: EquippedMesh=%s"), 
                                EquippedItem->EquippedMesh ? *EquippedItem->EquippedMesh->GetName() : TEXT("null")));
                    }
                    
                    EquipmentSlots.Remove(EquippedItem->EquipmentSlot);
                    EquippedItem->bIsEquipped = false;
                    
                    // Убираем визуал с персонажа
                    if (UEquipmentComponent* EquipComp = GetOwner()->FindComponentByClass<UEquipmentComponent>())
                    {
                        EquipComp->UnequipItem(EquippedItem->EquipmentSlot);
                    }
                    
                    // Проверяем состояние EquippedMesh после снятия
                    if (GEngine)
                    {
                        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
                            FString::Printf(TEXT("🔍 After unequip: EquippedMesh=%s"), 
                                EquippedItem->EquippedMesh ? *EquippedItem->EquippedMesh->GetName() : TEXT("null")));
                        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                            FString::Printf(TEXT("✅ Item unequipped from slot: %d"), (int32)EquippedItem->EquipmentSlot));
                    }
                }
            }
            return true;
        }
    }
    
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

    // Вспомогательные лямбды для проверки размещения в гриде экипировки
    auto GetGridSize = [](const UEquippableItemData* Equip) -> FIntPoint
    {
        if (!Equip) return FIntPoint(0,0);
        const int32 GX = FMath::Max(1, Equip->AdditionalGridSize.X);
        const int32 GY = FMath::Max(1, Equip->AdditionalGridSize.Y);
        return FIntPoint(GX, GY);
    };

    auto IsRectFree = [&](const UEquippableItemData* Equip, int32 StartX, int32 StartY, int32 SX, int32 SY, const UInventoryItemData* Ignored) -> bool
    {
        if (!Equip) return false;
        const FIntPoint GS = GetGridSize(Equip);
        if (StartX < 0 || StartY < 0) return false;
        if (StartX + SX > GS.X || StartY + SY > GS.Y) return false;
        for (const TPair<TObjectPtr<UInventoryItemData>, FIntPoint>& Pair : Equip->StoredCellByItem)
        {
            UInventoryItemData* Other = Pair.Key;
            if (!Other || Other == Ignored) continue;
            const FIntPoint OtherCell = Pair.Value;
            const bool bOtherRot = Equip->PersistentRotByItem.Contains(Other) ? Equip->PersistentRotByItem[Other] : (Equip->StoredRotByItem.Contains(Other) ? Equip->StoredRotByItem[Other] : false);
            const int32 OtherSX = bOtherRot ? FMath::Max(1, Other->SizeInCellsY) : FMath::Max(1, Other->SizeInCellsX);
            const int32 OtherSY = bOtherRot ? FMath::Max(1, Other->SizeInCellsX) : FMath::Max(1, Other->SizeInCellsY);
            const bool overlapX = !(StartX + SX <= OtherCell.X || OtherCell.X + OtherSX <= StartX);
            const bool overlapY = !(StartY + SY <= OtherCell.Y || OtherCell.Y + OtherSY <= StartY);
            if (overlapX && overlapY) return false;
        }
        return true;
    };

    auto FindPlacement = [&](UEquippableItemData* Equip, const UInventoryItemData* It, int32& OutX, int32& OutY, bool& bOutRot) -> bool
    {
        if (!Equip || !It || !Equip->bHasAdditionalStorage) return false;
        const FIntPoint GS = GetGridSize(Equip);
        // Перебираем клетки слева-вверх → справа-вниз
        for (int32 y = 0; y < GS.Y; ++y)
        {
            for (int32 x = 0; x < GS.X; ++x)
            {
                // Сначала без поворота
                int32 SX = FMath::Max(1, It->SizeInCellsX);
                int32 SY = FMath::Max(1, It->SizeInCellsY);
                if (IsRectFree(Equip, x, y, SX, SY, It))
                {
                    OutX = x; OutY = y; bOutRot = false; return true;
                }
                // Затем с поворотом
                SX = FMath::Max(1, It->SizeInCellsY);
                SY = FMath::Max(1, It->SizeInCellsX);
                if (IsRectFree(Equip, x, y, SX, SY, It))
                {
                    OutX = x; OutY = y; bOutRot = true; return true;
                }
            }
        }
        return false;
    };

    // 2) Пытаемся положить в карманы (одна 1x1 ячейка на карман)
    if (Item->SizeInCellsX == 1 && Item->SizeInCellsY == 1)
    {
        const FIntPoint OneCell(1,1);
        if (Pocket1Items.Num() < 1 && AddToGridLike(Pocket1Items, OneCell, Item)) return true;
        if (Pocket2Items.Num() < 1 && AddToGridLike(Pocket2Items, OneCell, Item)) return true;
        if (Pocket3Items.Num() < 1 && AddToGridLike(Pocket3Items, OneCell, Item)) return true;
        if (Pocket4Items.Num() < 1 && AddToGridLike(Pocket4Items, OneCell, Item)) return true;
    }

    // 3) Пытаемся положить в рюкзак с проверкой соседних свободных клеток и автоповоротом
    if (UEquippableItemData* EquippedBackpack = GetEquippedItem(Backpack))
    {
        int32 CellX=0, CellY=0; bool bRot=false;
        if (FindPlacement(EquippedBackpack, Item, CellX, CellY, bRot))
        {
            RemoveFromAnyStorage(Item);
            if (AddToEquipmentStorage(EquippedBackpack, Item))
            {
                EquippedBackpack->StoredCellByItem.Add(Item, FIntPoint(CellX, CellY));
                EquippedBackpack->PersistentCellByItem.Add(Item, FIntPoint(CellX, CellY));
                EquippedBackpack->StoredRotByItem.Add(Item, bRot);
                EquippedBackpack->PersistentRotByItem.Add(Item, bRot);
                return true;
            }
        }
    }

    // 4) Пытаемся положить в жилет: ширина секции всегда 1, колонки 2 и 3 (0-based) имеют высоту 2, остальные высота 1
    if (UEquippableItemData* EquippedVest = GetEquippedItem(Vest))
    {
        auto IsTallColumn = [](int32 Col) { return Col == 2 || Col == 3; };
        const int32 ItemW = FMath::Max(1, Item->SizeInCellsX);
        const int32 ItemH = FMath::Max(1, Item->SizeInCellsY);
        // 2x2 никогда не влезет в секции жилета (ширина секции = 1)
        if (!(ItemW == 2 && ItemH == 2))
        {
            // Перебираем колонки 0..5
            for (int32 col = 0; col < 6; ++col)
            {
                const int32 colHeight = IsTallColumn(col) ? 2 : 1;
                // Кандидаты ориентаций: только ширина 1 допустима
                struct { int32 W; int32 H; bool bRot; } Candidates[2] = {
                    { ItemW, ItemH, false },
                    { ItemH, ItemW, true }
                };
                for (const auto& C : Candidates)
                {
                    if (C.W != 1) continue; // ширина секции строго 1
                    if (C.H > colHeight) continue; // не помещается по высоте в эту колонку
                    const int32 startYMax = colHeight - C.H;
                    for (int32 y = 0; y <= startYMax; ++y)
                    {
                        if (IsRectFree(EquippedVest, col, y, C.W, C.H, Item))
                        {
                            // Нашли место
                            RemoveFromAnyStorage(Item);
                            if (AddToEquipmentStorage(EquippedVest, Item))
                            {
                                EquippedVest->StoredCellByItem.Add(Item, FIntPoint(col, y));
                                EquippedVest->PersistentCellByItem.Add(Item, FIntPoint(col, y));
                                EquippedVest->StoredRotByItem.Add(Item, C.bRot);
                                EquippedVest->PersistentRotByItem.Add(Item, C.bRot);
                                return true;
                            }
                            else
                            {
                                // Если добавить в хранилище не удалось — прекращаем поиск
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    // Иначе — не подбираем (нет континуального места ни в одном хранилище)
    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("⚠️ TryPickupItem: No contiguous free cells in pockets/backpack/vest"));
    return false;
}

bool UInventoryComponent::MoveItemToVest(UInventoryItemData* Item)
{
    if (!Item) return false;
    if (UEquippableItemData* EquippedVest = GetEquippedItem(Vest))
    {
        // Удаляем из ВСЕХ возможных мест хранения (включая карманы/пояс),
        // иначе предмет может дублироваться (например, остаться в кармане и одновременно оказаться в жилете).
        RemoveFromAnyStorage(Item);
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







