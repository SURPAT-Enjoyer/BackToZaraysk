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







