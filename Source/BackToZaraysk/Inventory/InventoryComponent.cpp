#include "InventoryComponent.h"
#include "EquippableItemData.h"
#include "BackToZaraysk/Components/EquipmentComponent.h"
#include "BackToZaraysk/GameData/Items/Test/PickupBase.h"
#include "BackToZaraysk/GameData/Items/EquipmentBase.h"
#include "BackToZaraysk/Inventory/InventoryBlueprintLibrary.h"
#include "BackToZaraysk/Characters/PlayerCharacter.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"

// Вспомогательный хелпер для доступа к массиву кармана по индексу (1..4)
static TArray<TObjectPtr<UInventoryItemData>>* GetPocketArrayByIndex(UInventoryComponent* Comp, int32 PocketIndex);

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UInventoryComponent::AddToBackpack(UInventoryItemData* Item)
{
	if (!Item) return false;

	// Спец-случай: разгрузочный жилет, когда на бронежилете уже стоят моды.
	// В этом случае, если TryPickupItem вернул false (нет рюкзака или места в нём),
	// мы НЕ должны "тихо" класть жилет в BackpackItems (иначе пикап исчезает).
	if (UEquippableItemData* Eq = Cast<UEquippableItemData>(Item))
	{
		if (Eq->EquipmentSlot == Vest)
		{
			if (UEquippableItemData* EquippedArmor = GetEquippedItem(Armor))
			{
				if (EquippedArmor->InstalledMods.Num() > 0)
				{
					// Если есть рюкзак и место, TryPickupItem уже положит жилет в его хранилище.
					// Сюда попадаем только когда TryPickupItem не смог — значит, подбирать нельзя.
					return false;
				}
			}
		}
	}

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

	// Если пытаемся экипировать разгрузку (Vest), а на броне уже стоят моды — запрещаем.
	if (Item->EquipmentSlot == Vest)
	{
		if (UEquippableItemData* EquippedArmor = GetEquippedItem(Armor))
		{
			if (EquippedArmor->InstalledMods.Num() > 0)
			{
				if (GEngine)
				{
					GEngine->AddOnScreenDebugMessage(
						-1, 3.0f, FColor::Red,
						TEXT("❌ Нельзя экипировать разгрузку, пока на бронежилете установлены моды"));
				}
				return false;
			}
		}
	}

	// Аналогично: если уже надет разгрузочный жилет, нельзя надеть бронежилет с установленными модами.
	if (Item->EquipmentSlot == Armor && Item->InstalledMods.Num() > 0)
	{
		if (UEquippableItemData* EquippedVest = GetEquippedItem(Vest))
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					-1, 3.0f, FColor::Red,
					TEXT("❌ Нельзя экипировать бронежилет с модами, пока надета разгрузка"));
			}
			return false;
		}
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
    if (Item->EquipmentSlot == Backpack || Item->EquipmentSlot == Vest || Item->EquipmentSlot == Belt)
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
            // Восстановим и индекс мини‑грида (для жилетов с несколькими гридами)
            if (Item->PersistentGridByItem.Num() > 0)
            {
                for (auto& Kvp : Item->PersistentGridByItem)
                {
                    Item->StoredGridByItem.Add(Kvp.Key, Kvp.Value);
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

    // BackpackBase: при снятии из слота через контекстное меню (bDropToWorld=false) предмет должен вываливаться в мир.
    // Иначе он "исчезает", т.к. положить рюкзак в инвентарь/хранилища обычно некуда по текущим правилам.
    if (SlotType == Backpack && !bDropToWorld)
    {
        bDropToWorld = true;
    }

    // Перед снятием: если это рюкзак или жилет, и мы выбрасываем в мир — переносим содержимое в PersistentStorage
    if ((SlotType == Backpack || SlotType == Vest || SlotType == Belt) && bDropToWorld)
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

    // Если снимаем жилет "в инвентарь" (контекстное меню): пытаемся поместить в хранилище надетого рюкзака.
    // Если рюкзака нет / места нет / есть наложение — выбрасываем в мир.
    bool bVestPlaceInBackpack = false;
    int32 VestGridIdx = 0;
    int32 VestCellX = 0;
    int32 VestCellY = 0;
    bool bVestRot = false;
    if (SlotType == Vest && !bDropToWorld)
    {
        if (UEquippableItemData* EquippedBackpack = GetEquippedItem(Backpack))
        {
            if (EquippedBackpack->bHasAdditionalStorage)
            {
                auto GetGridSizes = [](const UEquippableItemData* Equip) -> TArray<FIntPoint>
                {
                    if (!Equip) return {};
                    if (Equip->AdditionalGrids.Num() > 0) return Equip->AdditionalGrids;
                    return { FIntPoint(FMath::Max(1, Equip->AdditionalGridSize.X), FMath::Max(1, Equip->AdditionalGridSize.Y)) };
                };
                auto GetItemGridIdx = [](const UEquippableItemData* Equip, const UInventoryItemData* It) -> int32
                {
                    if (!Equip || !It) return 0;
                    if (const int32* G = Equip->PersistentGridByItem.Find(It)) return *G;
                    if (const int32* G2 = Equip->StoredGridByItem.Find(It)) return *G2;
                    return 0;
                };
                auto IsRectFree = [&](const UEquippableItemData* Equip, int32 InGridIdx, int32 StartX, int32 StartY, int32 SX, int32 SY, const UInventoryItemData* Ignored) -> bool
                {
                    if (!Equip) return false;
                    const TArray<FIntPoint> Grids = GetGridSizes(Equip);
                    if (!Grids.IsValidIndex(InGridIdx)) return false;
                    const FIntPoint GS(FMath::Max(1, Grids[InGridIdx].X), FMath::Max(1, Grids[InGridIdx].Y));
                    if (StartX < 0 || StartY < 0) return false;
                    if (StartX + SX > GS.X || StartY + SY > GS.Y) return false;
                    for (const TPair<TObjectPtr<UInventoryItemData>, FIntPoint>& Pair : Equip->StoredCellByItem)
                    {
                        UInventoryItemData* Other = Pair.Key;
                        if (!Other || Other == Ignored) continue;
                        if (GetItemGridIdx(Equip, Other) != InGridIdx) continue;
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
                auto FindPlacement = [&](UEquippableItemData* Equip, const UInventoryItemData* It, int32& OutGridIdx, int32& OutX, int32& OutY, bool& bOutRot) -> bool
                {
                    if (!Equip || !It) return false;
                    const TArray<FIntPoint> Grids = GetGridSizes(Equip);
                    for (int32 g = 0; g < Grids.Num(); ++g)
                    {
                        const int32 GX = FMath::Max(1, Grids[g].X);
                        const int32 GY = FMath::Max(1, Grids[g].Y);
                        for (int32 y = 0; y < GY; ++y)
                        {
                            for (int32 x = 0; x < GX; ++x)
                            {
                                int32 SX = FMath::Max(1, It->SizeInCellsX);
                                int32 SY = FMath::Max(1, It->SizeInCellsY);
                                if (IsRectFree(Equip, g, x, y, SX, SY, It))
                                {
                                    OutGridIdx = g; OutX = x; OutY = y; bOutRot = false; return true;
                                }
                                if (It->bRotatable)
                                {
                                    SX = FMath::Max(1, It->SizeInCellsY);
                                    SY = FMath::Max(1, It->SizeInCellsX);
                                    if (IsRectFree(Equip, g, x, y, SX, SY, It))
                                    {
                                        OutGridIdx = g; OutX = x; OutY = y; bOutRot = true; return true;
                                    }
                                }
                            }
                        }
                    }
                    return false;
                };

                if (FindPlacement(EquippedBackpack, Item, VestGridIdx, VestCellX, VestCellY, bVestRot))
                {
                    bVestPlaceInBackpack = true;
                }
                else
                {
                    bDropToWorld = true;
                }
            }
            else
            {
                bDropToWorld = true;
            }
        }
        else
        {
            bDropToWorld = true;
        }
    }

    // Если снимаем бронежилет и рюкзак не экипирован — выбрасываем в мир.
    // ВАЖНО: UI не отображает BackpackItems (SyncBackpack заглушка), поэтому "в инвентарь" для Armor = в хранилище надетого рюкзака.
    // Если рюкзака нет или в нём нет места — дроп в мир.
    bool bArmorPlaceInBackpack = false;
    int32 ArmorGridIdx = 0;
    int32 ArmorCellX = 0;
    int32 ArmorCellY = 0;
    bool bArmorRot = false;
    if (SlotType == Armor && !bDropToWorld)
    {
        if (UEquippableItemData* EquippedBackpack = GetEquippedItem(Backpack))
        {
            auto GetGridSizes = [](const UEquippableItemData* Equip) -> TArray<FIntPoint>
            {
                if (!Equip) return {};
                if (Equip->AdditionalGrids.Num() > 0) return Equip->AdditionalGrids;
                return { FIntPoint(FMath::Max(1, Equip->AdditionalGridSize.X), FMath::Max(1, Equip->AdditionalGridSize.Y)) };
            };
            auto GetItemGridIdx = [](const UEquippableItemData* Equip, const UInventoryItemData* It) -> int32
            {
                if (!Equip || !It) return 0;
                if (const int32* G = Equip->PersistentGridByItem.Find(It)) return *G;
                if (const int32* G2 = Equip->StoredGridByItem.Find(It)) return *G2;
                return 0;
            };
            auto IsRectFree = [&](const UEquippableItemData* Equip, int32 InGridIdx, int32 StartX, int32 StartY, int32 SX, int32 SY, const UInventoryItemData* Ignored) -> bool
            {
                if (!Equip) return false;
                const TArray<FIntPoint> Grids = GetGridSizes(Equip);
                if (!Grids.IsValidIndex(InGridIdx)) return false;
                const FIntPoint GS(FMath::Max(1, Grids[InGridIdx].X), FMath::Max(1, Grids[InGridIdx].Y));
                if (StartX < 0 || StartY < 0) return false;
                if (StartX + SX > GS.X || StartY + SY > GS.Y) return false;
                for (const TPair<TObjectPtr<UInventoryItemData>, FIntPoint>& Pair : Equip->StoredCellByItem)
                {
                    UInventoryItemData* Other = Pair.Key;
                    if (!Other || Other == Ignored) continue;
                    if (GetItemGridIdx(Equip, Other) != InGridIdx) continue;
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
            auto FindPlacement = [&](UEquippableItemData* Equip, const UInventoryItemData* It, int32& OutGridIdx, int32& OutX, int32& OutY, bool& bOutRot) -> bool
            {
                if (!Equip || !It) return false;
                const TArray<FIntPoint> Grids = GetGridSizes(Equip);
                for (int32 g = 0; g < Grids.Num(); ++g)
                {
                    const int32 GX = FMath::Max(1, Grids[g].X);
                    const int32 GY = FMath::Max(1, Grids[g].Y);
                    for (int32 y = 0; y < GY; ++y)
                    {
                        for (int32 x = 0; x < GX; ++x)
                        {
                            int32 SX = FMath::Max(1, It->SizeInCellsX);
                            int32 SY = FMath::Max(1, It->SizeInCellsY);
                            if (IsRectFree(Equip, g, x, y, SX, SY, It))
                            {
                                OutGridIdx = g; OutX = x; OutY = y; bOutRot = false; return true;
                            }
                            SX = FMath::Max(1, It->SizeInCellsY);
                            SY = FMath::Max(1, It->SizeInCellsX);
                            if (IsRectFree(Equip, g, x, y, SX, SY, It))
                            {
                                OutGridIdx = g; OutX = x; OutY = y; bOutRot = true; return true;
                            }
                        }
                    }
                }
                return false;
            };

            if (EquippedBackpack->bHasAdditionalStorage && FindPlacement(EquippedBackpack, Item, ArmorGridIdx, ArmorCellX, ArmorCellY, bArmorRot))
            {
                bArmorPlaceInBackpack = true;
            }
            else
            {
                bDropToWorld = true;
            }
        }
        else
        {
            bDropToWorld = true;
        }
    }

    // Если снимаем шлем: пытаемся положить в хранилища по приоритету:
    // 1) рюкзак (если надет и есть место)
    // 2) разгрузочный жилет (если надет и есть место)
    // 3) карманы 1x1
    // иначе — выбрасываем в мир.
    enum class EHelmetUnequipTarget : uint8 { None, Backpack, Vest, Pocket };
    EHelmetUnequipTarget HelmetTarget = EHelmetUnequipTarget::None;
    int32 HelmetBackpackGridIdx = 0;
    int32 HelmetCellX = 0;
    int32 HelmetCellY = 0;
    bool bHelmetRot = false;
    int32 HelmetVestGridIdx = INDEX_NONE;
    FIntPoint HelmetVestLocalCell(0, 0);
    int32 HelmetPocketIndex = 0;

    if (SlotType == Helmet && !bDropToWorld)
    {
        // === 1) Backpack storage ===
        if (UEquippableItemData* EquippedBackpack = GetEquippedItem(Backpack))
        {
            if (EquippedBackpack->bHasAdditionalStorage)
            {
                auto GetGridSizes = [](const UEquippableItemData* Equip) -> TArray<FIntPoint>
                {
                    if (!Equip) return {};
                    if (Equip->AdditionalGrids.Num() > 0) return Equip->AdditionalGrids;
                    return { FIntPoint(FMath::Max(1, Equip->AdditionalGridSize.X), FMath::Max(1, Equip->AdditionalGridSize.Y)) };
                };
                auto GetItemGridIdx = [](const UEquippableItemData* Equip, const UInventoryItemData* It) -> int32
                {
                    if (!Equip || !It) return 0;
                    if (const int32* G = Equip->PersistentGridByItem.Find(It)) return *G;
                    if (const int32* G2 = Equip->StoredGridByItem.Find(It)) return *G2;
                    return 0;
                };
                auto IsRectFree = [&](const UEquippableItemData* Equip, int32 InGridIdx, int32 StartX, int32 StartY, int32 SX, int32 SY, const UInventoryItemData* Ignored) -> bool
                {
                    if (!Equip) return false;
                    const TArray<FIntPoint> Grids = GetGridSizes(Equip);
                    if (!Grids.IsValidIndex(InGridIdx)) return false;
                    const FIntPoint GS(FMath::Max(1, Grids[InGridIdx].X), FMath::Max(1, Grids[InGridIdx].Y));
                    if (StartX < 0 || StartY < 0) return false;
                    if (StartX + SX > GS.X || StartY + SY > GS.Y) return false;
                    for (const TPair<TObjectPtr<UInventoryItemData>, FIntPoint>& Pair : Equip->StoredCellByItem)
                    {
                        UInventoryItemData* Other = Pair.Key;
                        if (!Other || Other == Ignored) continue;
                        if (GetItemGridIdx(Equip, Other) != InGridIdx) continue;
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
                auto FindPlacement = [&](UEquippableItemData* Equip, const UInventoryItemData* It, int32& OutGridIdx, int32& OutX, int32& OutY, bool& bOutRot) -> bool
                {
                    if (!Equip || !It) return false;
                    const TArray<FIntPoint> Grids = GetGridSizes(Equip);
                    for (int32 g = 0; g < Grids.Num(); ++g)
                    {
                        const int32 GX = FMath::Max(1, Grids[g].X);
                        const int32 GY = FMath::Max(1, Grids[g].Y);
                        for (int32 y = 0; y < GY; ++y)
                        {
                            for (int32 x = 0; x < GX; ++x)
                            {
                                int32 SX = FMath::Max(1, It->SizeInCellsX);
                                int32 SY = FMath::Max(1, It->SizeInCellsY);
                                if (IsRectFree(Equip, g, x, y, SX, SY, It))
                                {
                                    OutGridIdx = g; OutX = x; OutY = y; bOutRot = false; return true;
                                }
                                SX = FMath::Max(1, It->SizeInCellsY);
                                SY = FMath::Max(1, It->SizeInCellsX);
                                if (IsRectFree(Equip, g, x, y, SX, SY, It))
                                {
                                    OutGridIdx = g; OutX = x; OutY = y; bOutRot = true; return true;
                                }
                            }
                        }
                    }
                    return false;
                };

                if (FindPlacement(EquippedBackpack, Item, HelmetBackpackGridIdx, HelmetCellX, HelmetCellY, bHelmetRot))
                {
                    HelmetTarget = EHelmetUnequipTarget::Backpack;
                }
            }
        }

        // === 2) Vest storage (6 mini-grids) ===
        if (HelmetTarget == EHelmetUnequipTarget::None)
        {
            if (UEquippableItemData* EquippedVest = GetEquippedItem(Vest))
            {
                if (EquippedVest->bHasAdditionalStorage)
                {
                    auto GetVestPlacement = [&](const UEquippableItemData* VestData, const UInventoryItemData* It, int32& OutGridIdx, FIntPoint& OutLocalCell) -> bool
                    {
                        if (!VestData || !It) return false;
                        const int32* GridPtr = VestData->PersistentGridByItem.Find(It);
                        if (!GridPtr) GridPtr = VestData->StoredGridByItem.Find(It);
                        const FIntPoint* CellPtr = VestData->PersistentCellByItem.Find(It);
                        if (!CellPtr) CellPtr = VestData->StoredCellByItem.Find(It);
                        if (!CellPtr && !GridPtr) return false;
                        if (GridPtr)
                        {
                            OutGridIdx = *GridPtr;
                            OutLocalCell = CellPtr ? *CellPtr : FIntPoint(0,0);
                            return true;
                        }
                        // миграция старого формата: StoredCellByItem = global (X=grid, Y=row)
                        OutGridIdx = CellPtr->X;
                        OutLocalCell = FIntPoint(0, CellPtr->Y);
                        return true;
                    };
                    auto GetVestRotation = [&](const UEquippableItemData* VestData, const UInventoryItemData* It, bool& bOutRot) -> bool
                    {
                        bOutRot = false;
                        if (!VestData || !It) return false;
                        if (const bool* R = VestData->PersistentRotByItem.Find(It)) { bOutRot = *R; return true; }
                        if (const bool* R2 = VestData->StoredRotByItem.Find(It)) { bOutRot = *R2; return true; }
                        return false;
                    };
                    auto VestMiniGridSizeByIndex = [](int32 GridIdx) -> FIntPoint
                    {
                        if (GridIdx == 2 || GridIdx == 3) return FIntPoint(1, 2);
                        return FIntPoint(1, 1);
                    };
                    auto IsAreaFreeInVestMiniGrid = [&](const UEquippableItemData* VestData, int32 GridIdx, int32 StartX, int32 StartY, int32 SX, int32 SY, const UInventoryItemData* Ignored) -> bool
                    {
                        if (!VestData) return false;
                        if (GridIdx < 0 || GridIdx > 5) return false;
                        const FIntPoint GS = VestMiniGridSizeByIndex(GridIdx);
                        if (StartX < 0 || StartY < 0) return false;
                        if (StartX + SX > GS.X || StartY + SY > GS.Y) return false;

                        for (const TPair<TObjectPtr<UInventoryItemData>, FIntPoint>& Pair : VestData->StoredCellByItem)
                        {
                            UInventoryItemData* Other = Pair.Key;
                            if (!Other || Other == Ignored) continue;
                            int32 OtherGrid = INDEX_NONE;
                            FIntPoint OtherCell(0,0);
                            if (!GetVestPlacement(VestData, Other, OtherGrid, OtherCell)) continue;
                            if (OtherGrid != GridIdx) continue;
                            bool bOtherRot = false;
                            GetVestRotation(VestData, Other, bOtherRot);
                            const int32 OtherSX = bOtherRot ? FMath::Max(1, Other->SizeInCellsY) : FMath::Max(1, Other->SizeInCellsX);
                            const int32 OtherSY = bOtherRot ? FMath::Max(1, Other->SizeInCellsX) : FMath::Max(1, Other->SizeInCellsY);
                            const bool overlapX = !(StartX + SX <= OtherCell.X || OtherCell.X + OtherSX <= StartX);
                            const bool overlapY = !(StartY + SY <= OtherCell.Y || OtherCell.Y + OtherSY <= StartY);
                            if (overlapX && overlapY) return false;
                        }
                        return true;
                    };

                    // Перебираем мини-гриды 0..5, кладём только предметы с шириной 1 (после возможного поворота)
                    const int32 BaseW = FMath::Max(1, Item->SizeInCellsX);
                    const int32 BaseH = FMath::Max(1, Item->SizeInCellsY);
                    for (int32 grid = 0; grid < 6; ++grid)
                    {
                        const FIntPoint GS = VestMiniGridSizeByIndex(grid);
                        struct { int32 W; int32 H; bool bRot; } Candidates[2] = {
                            { BaseW, BaseH, false },
                            { BaseH, BaseW, true }
                        };
                        for (const auto& C : Candidates)
                        {
                            if (C.W != 1) continue;
                            if (C.H > GS.Y) continue;
                            const int32 startYMax = GS.Y - C.H;
                            for (int32 y = 0; y <= startYMax; ++y)
                            {
                                if (IsAreaFreeInVestMiniGrid(EquippedVest, grid, 0, y, C.W, C.H, Item))
                                {
                                    HelmetTarget = EHelmetUnequipTarget::Vest;
                                    HelmetVestGridIdx = grid;
                                    HelmetVestLocalCell = FIntPoint(0, y);
                                    bHelmetRot = C.bRot;
                                    break;
                                }
                            }
                            if (HelmetTarget == EHelmetUnequipTarget::Vest) break;
                        }
                        if (HelmetTarget == EHelmetUnequipTarget::Vest) break;
                    }
                }
            }
        }

        // === 3) Pockets ===
        if (HelmetTarget == EHelmetUnequipTarget::None)
        {
            // Карманы только 1x1
            if (Item->SizeInCellsX == 1 && Item->SizeInCellsY == 1)
            {
                if (Pocket1Items.Num() < 1) { HelmetTarget = EHelmetUnequipTarget::Pocket; HelmetPocketIndex = 1; }
                else if (Pocket2Items.Num() < 1) { HelmetTarget = EHelmetUnequipTarget::Pocket; HelmetPocketIndex = 2; }
                else if (Pocket3Items.Num() < 1) { HelmetTarget = EHelmetUnequipTarget::Pocket; HelmetPocketIndex = 3; }
                else if (Pocket4Items.Num() < 1) { HelmetTarget = EHelmetUnequipTarget::Pocket; HelmetPocketIndex = 4; }
            }
        }

        // Если никуда не помещается — выбрасываем в мир
        if (HelmetTarget == EHelmetUnequipTarget::None)
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
        if ((SlotType == Backpack || SlotType == Vest || SlotType == Belt) && bDropToWorld)
        {
            EquipmentStorage.Remove(Item);
        }

        // Если не выбрасываем в мир
        if (!bDropToWorld)
        {
            // Особый случай: снимаем жилет — кладём в хранилище надетого рюкзака по рассчитанной позиции.
            if (SlotType == Vest)
            {
                if (UEquippableItemData* EquippedBackpack = GetEquippedItem(Backpack))
                {
                    if (EquippedBackpack->bHasAdditionalStorage && bVestPlaceInBackpack)
                    {
                        RemoveFromAnyStorage(Item);
                        if (AddToEquipmentStorage(EquippedBackpack, Item))
                        {
                            EquippedBackpack->StoredGridByItem.Add(Item, VestGridIdx);
                            EquippedBackpack->PersistentGridByItem.Add(Item, VestGridIdx);
                            EquippedBackpack->StoredCellByItem.Add(Item, FIntPoint(VestCellX, VestCellY));
                            EquippedBackpack->PersistentCellByItem.Add(Item, FIntPoint(VestCellX, VestCellY));
                            EquippedBackpack->StoredRotByItem.Add(Item, bVestRot);
                            EquippedBackpack->PersistentRotByItem.Add(Item, bVestRot);
                            return true;
                        }
                    }
                }
                // Если по какой-то причине не удалось добавить — выбрасываем в мир вместо "пропажи"
                if (ACharacter* OwnerChar = Cast<ACharacter>(GetOwner()))
                {
                    if (UWorld* World = OwnerChar->GetWorld())
                    {
                        const FVector SpawnLoc = OwnerChar->GetActorLocation() + OwnerChar->GetActorForwardVector() * 80.f;
                        FVector ViewLoc; FRotator ViewRot; OwnerChar->GetActorEyesViewPoint(ViewLoc, ViewRot);
                        FActorSpawnParameters S; S.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
                        if (TSubclassOf<AActor> DropClass = GetPickupClassForItem_Internal(Item))
                        {
                            if (AActor* SpawnedActor = World->SpawnActor<AActor>(DropClass, SpawnLoc, ViewRot, S))
                            {
                                if (APickupBase* Spawned = Cast<APickupBase>(SpawnedActor))
                                {
                                    Spawned->ItemInstance = Item;
                                    Spawned->ApplyItemInstanceVisuals();
                                }
                                else if (AEquipmentBase* AsEquip = Cast<AEquipmentBase>(SpawnedActor))
                                {
                                    UEquipmentComponent::SetupDroppedEquipmentVisual(AsEquip, Item);
                                }
                            }
                        }
                    }
                }
                return true;
            }

            // Особый случай: снимаем бронежилет — кладём в хранилище надетого рюкзака, иначе он "исчезнет" (BackpackItems не отображаются в UI)
            if (SlotType == Armor)
            {
                if (UEquippableItemData* EquippedBackpack = GetEquippedItem(Backpack))
                {
                    if (EquippedBackpack->bHasAdditionalStorage && bArmorPlaceInBackpack)
                    {
                        RemoveFromAnyStorage(Item);
                        if (AddToEquipmentStorage(EquippedBackpack, Item))
                        {
                            EquippedBackpack->StoredGridByItem.Add(Item, ArmorGridIdx);
                            EquippedBackpack->PersistentGridByItem.Add(Item, ArmorGridIdx);
                            EquippedBackpack->StoredCellByItem.Add(Item, FIntPoint(ArmorCellX, ArmorCellY));
                            EquippedBackpack->PersistentCellByItem.Add(Item, FIntPoint(ArmorCellX, ArmorCellY));
                            EquippedBackpack->StoredRotByItem.Add(Item, bArmorRot);
                            EquippedBackpack->PersistentRotByItem.Add(Item, bArmorRot);
                            return true;
                        }
                    }
                }
                // Если по какой-то причине не удалось добавить — выбрасываем в мир вместо "пропажи"
                if (ACharacter* OwnerChar = Cast<ACharacter>(GetOwner()))
                {
                    if (UWorld* World = OwnerChar->GetWorld())
                    {
                        FVector ViewLoc; FRotator ViewRot; OwnerChar->GetActorEyesViewPoint(ViewLoc, ViewRot);
                        FVector SpawnLoc = ViewLoc + ViewRot.Vector() * 80.f + FVector(0.f, 0.f, 100.f);
                        // Для бронежилета — спавним от позиции груди (spine_03), чтобы не проваливался под землю
                        if (USkeletalMeshComponent* M = OwnerChar->GetMesh())
                        {
                            const FName ChestSocket(TEXT("spine_03"));
                            const FName RootSocket(TEXT("root"));
                            if (M->DoesSocketExist(ChestSocket))
                            {
                                SpawnLoc = M->GetSocketLocation(ChestSocket) + OwnerChar->GetActorForwardVector() * 80.f;
                            }
                            else if (M->DoesSocketExist(RootSocket))
                            {
                                SpawnLoc = M->GetSocketLocation(RootSocket) + OwnerChar->GetActorForwardVector() * 80.f;
                            }
                            else
                            {
                                SpawnLoc = OwnerChar->GetActorLocation() + OwnerChar->GetActorForwardVector() * 80.f;
                            }
                        }
                        FActorSpawnParameters S; S.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
                        if (TSubclassOf<AActor> DropClass = GetPickupClassForItem_Internal(Item))
                        {
                            if (AActor* SpawnedActor = World->SpawnActor<AActor>(DropClass, SpawnLoc, ViewRot, S))
                            {
                                if (AEquipmentBase* AsEquip = Cast<AEquipmentBase>(SpawnedActor))
                                {
                                    UEquipmentComponent::SetupDroppedEquipmentVisual(AsEquip, Item);
                                }
                                else if (APickupBase* Spawned = Cast<APickupBase>(SpawnedActor))
                                {
                                    Spawned->ItemInstance = Item;
                                    Spawned->ApplyItemInstanceVisuals();
                                }
                            }
                        }
                    }
                }
                return true;
            }

            // Особый случай: снимаем шлем — кладём в рюкзак/жилет/карманы по приоритету, иначе (если бDropToWorld=true) уже выбросили.
            if (SlotType == Helmet)
            {
                // 1) В рюкзак
                if (HelmetTarget == EHelmetUnequipTarget::Backpack)
                {
                    if (UEquippableItemData* EquippedBackpack = GetEquippedItem(Backpack))
                    {
                        RemoveFromAnyStorage(Item);
                        if (AddToEquipmentStorage(EquippedBackpack, Item))
                        {
                            EquippedBackpack->StoredGridByItem.Add(Item, HelmetBackpackGridIdx);
                            EquippedBackpack->PersistentGridByItem.Add(Item, HelmetBackpackGridIdx);
                            EquippedBackpack->StoredCellByItem.Add(Item, FIntPoint(HelmetCellX, HelmetCellY));
                            EquippedBackpack->PersistentCellByItem.Add(Item, FIntPoint(HelmetCellX, HelmetCellY));
                            EquippedBackpack->StoredRotByItem.Add(Item, bHelmetRot);
                            EquippedBackpack->PersistentRotByItem.Add(Item, bHelmetRot);
                            return true;
                        }
                    }
                }
                // 2) В жилет (мини-грид)
                if (HelmetTarget == EHelmetUnequipTarget::Vest)
                {
                    if (UEquippableItemData* EquippedVest = GetEquippedItem(Vest))
                    {
                        RemoveFromAnyStorage(Item);
                        if (AddToEquipmentStorage(EquippedVest, Item))
                        {
                            EquippedVest->StoredGridByItem.Add(Item, HelmetVestGridIdx);
                            EquippedVest->PersistentGridByItem.Add(Item, HelmetVestGridIdx);
                            EquippedVest->StoredCellByItem.Add(Item, HelmetVestLocalCell);
                            EquippedVest->PersistentCellByItem.Add(Item, HelmetVestLocalCell);
                            EquippedVest->StoredRotByItem.Add(Item, bHelmetRot);
                            EquippedVest->PersistentRotByItem.Add(Item, bHelmetRot);
                            return true;
                        }
                    }
                }
                // 3) В карман
        if (HelmetTarget == EHelmetUnequipTarget::Pocket && HelmetPocketIndex >= 1 && HelmetPocketIndex <= 4)
        {
            // Удаляем из всех мест и пытаемся положить в выбранный карман с учётом многоклеточной сетки
            RemoveFromAnyStorage(Item);
            int32 CellX = 0, CellY = 0;
            bool bRot = false;
            if (FindPlacementInPocket(HelmetPocketIndex, Item, CellX, CellY, bRot))
            {
                if (TArray<TObjectPtr<UInventoryItemData>>* Target = GetPocketArrayByIndex(this, HelmetPocketIndex))
                {
                    Target->Add(Item);
                    PocketIndexByItem.Add(Item, HelmetPocketIndex);
                    PocketCellByItem.Add(Item, FIntPoint(CellX, CellY));
                    PocketRotByItem.Add(Item, bRot);
                    return true;
                }
            }
        }
                // Если мы дошли сюда — что-то пошло не так (потеряли место после расчёта). Тогда выбрасываем в мир как фолбэк.
                if (ACharacter* OwnerChar = Cast<ACharacter>(GetOwner()))
                {
                    if (UWorld* World = OwnerChar->GetWorld())
                    {
                        FVector ViewLoc; FRotator ViewRot; OwnerChar->GetActorEyesViewPoint(ViewLoc, ViewRot);
                        const FVector SpawnLoc = OwnerChar->GetActorLocation() + OwnerChar->GetActorForwardVector() * 80.f;
                        FActorSpawnParameters S; S.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
                        if (TSubclassOf<AActor> DropClass = GetPickupClassForItem_Internal(Item))
                        {
                            if (AActor* SpawnedActor = World->SpawnActor<AActor>(DropClass, SpawnLoc, ViewRot, S))
                            {
                                if (APickupBase* Spawned = Cast<APickupBase>(SpawnedActor))
                                {
                                    Spawned->ItemInstance = Item;
                                    Spawned->ApplyItemInstanceVisuals();
                                }
                                else if (AEquipmentBase* AsEquip = Cast<AEquipmentBase>(SpawnedActor))
                                {
                                    UEquipmentComponent::SetupDroppedEquipmentVisual(AsEquip, Item);
                                }
                            }
                        }
                    }
                }
                return true;
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
                                    // Гарантируем видимость (ВАЖНО: не включаем одновременно и StaticMesh, и SkeletalMesh,
                                    // иначе у предметов с SkeletalMesh будет дополнительно рисоваться куб Mesh).
                                    SpawnedActor->SetActorHiddenInGame(false);
                                    const bool bHasSk = (Eq->SkeletalMesh && Eq->SkeletalMesh->GetSkeletalMeshAsset() != nullptr);
                                    const bool bHasSt = (Eq->Mesh && Eq->Mesh->GetStaticMesh() != nullptr);
                                    if (bHasSk)
                                    {
                                        Eq->SkeletalMesh->SetVisibility(true, true);
                                        Eq->SkeletalMesh->SetHiddenInGame(false, true);
                                        if (Eq->Mesh)
                                        {
                                            Eq->Mesh->SetVisibility(false, true);
                                            Eq->Mesh->SetHiddenInGame(true, true);
                                        }
                                    }
                                    else if (bHasSt)
                                    {
                                        Eq->Mesh->SetVisibility(true, true);
                                        Eq->Mesh->SetHiddenInGame(false, true);
                                        if (Eq->SkeletalMesh)
                                        {
                                            Eq->SkeletalMesh->SetVisibility(false, true);
                                            Eq->SkeletalMesh->SetHiddenInGame(true, true);
                                        }
                                    }
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
	int32 TotalCells = 0;
    if (Equipment->AdditionalGrids.Num() > 0)
    {
        for (const FIntPoint& G : Equipment->AdditionalGrids)
        {
            TotalCells += FMath::Max(1, G.X) * FMath::Max(1, G.Y);
        }
    }
    else
    {
	    TotalCells = Equipment->AdditionalGridSize.X * Equipment->AdditionalGridSize.Y;
    }
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

    // Карманы
    bRemoved |= Pocket1Items.Remove(Item) > 0;
    bRemoved |= Pocket2Items.Remove(Item) > 0;
    bRemoved |= Pocket3Items.Remove(Item) > 0;
    bRemoved |= Pocket4Items.Remove(Item) > 0;
    PocketIndexByItem.Remove(Item);
    PocketCellByItem.Remove(Item);
    PocketRotByItem.Remove(Item);

    // Хранилища предметов-модулей (EquipModBase) — рекурсивно, чтобы можно было вытаскивать/перемещать предметы
    // из открытого окна, а также корректно удалять их при "Выбросить".
    {
        TSet<const UInventoryItemData*> Visited;

        auto RemoveFromEquipMod = [&](UEquipModBaseItemData* Mod, auto& SelfRef) -> bool
        {
            if (!Mod) return false;
            if (Visited.Contains(Mod)) return false;
            Visited.Add(Mod);

            bool bLocalRemoved = false;
            if (Mod->bHasAdditionalStorage)
            {
                const int32 Before = Mod->PersistentStorage.Num();
                Mod->PersistentStorage.Remove(Item);
                bLocalRemoved |= (Before != Mod->PersistentStorage.Num());

                if (bLocalRemoved)
                {
                    Mod->StoredCellByItem.Remove(Item);
                    Mod->PersistentCellByItem.Remove(Item);
                    Mod->StoredRotByItem.Remove(Item);
                    Mod->PersistentRotByItem.Remove(Item);
                    Mod->StoredGridByItem.Remove(Item);
                    Mod->PersistentGridByItem.Remove(Item);
                }
            }

            // Рекурсивно: если внутри модуля лежат другие модули с хранилищами
            if (Mod->bHasAdditionalStorage)
            {
                for (UInventoryItemData* Inner : Mod->PersistentStorage)
                {
                    if (UEquipModBaseItemData* InnerMod = Cast<UEquipModBaseItemData>(Inner))
                    {
                        bLocalRemoved |= SelfRef(InnerMod, SelfRef);
                    }
                }
            }

            return bLocalRemoved;
        };

        auto ScanContainer = [&](UInventoryItemData* Candidate, auto& SelfScan) -> void
        {
            if (!Candidate) return;
            if (UEquipModBaseItemData* Mod = Cast<UEquipModBaseItemData>(Candidate))
            {
                bRemoved |= RemoveFromEquipMod(Mod, RemoveFromEquipMod);
            }
        };

        // Сканы по всем "верхнеуровневым" спискам, где могут лежать модули
        for (UInventoryItemData* It : BackpackItems) { ScanContainer(It, ScanContainer); }
        for (UInventoryItemData* It : BeltStorageItems) { ScanContainer(It, ScanContainer); }
        for (UInventoryItemData* It : Pocket1Items) { ScanContainer(It, ScanContainer); }
        for (UInventoryItemData* It : Pocket2Items) { ScanContainer(It, ScanContainer); }
        for (UInventoryItemData* It : Pocket3Items) { ScanContainer(It, ScanContainer); }
        for (UInventoryItemData* It : Pocket4Items) { ScanContainer(It, ScanContainer); }

        // Также сканируем содержимое экипируемых контейнеров (рюкзак/разгрузка) — там тоже могут лежать модули
        if (UEquippableItemData* EquippedBackpack2 = GetEquippedItem(Backpack))
        {
            for (UInventoryItemData* It : EquippedBackpack2->PersistentStorage) { ScanContainer(It, ScanContainer); }
        }
        if (UEquippableItemData* EquippedVest2 = GetEquippedItem(Vest))
        {
            for (UInventoryItemData* It : EquippedVest2->PersistentStorage) { ScanContainer(It, ScanContainer); }
        }
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
	for (int32 SlotIndex = 0; SlotIndex < 8; ++SlotIndex) // Все возможные слоты (добавлен Armor)
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

FIntPoint UInventoryComponent::GetPocketGridSize(int32 PocketIndex) const
{
    // Индексы 1..4
    ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
    if (OwnerChar)
    {
        if (const APlayerCharacter* PC = Cast<APlayerCharacter>(OwnerChar))
        {
            if (PC->PocketMiniGridSizes.IsValidIndex(PocketIndex - 1))
            {
                const FIntPoint C = PC->PocketMiniGridSizes[PocketIndex - 1];
                return FIntPoint(FMath::Max(1, C.X), FMath::Max(1, C.Y));
            }
        }
    }
    // По умолчанию 3x2
    return FIntPoint(3, 2);
}

static TArray<TObjectPtr<UInventoryItemData>>* GetPocketArrayByIndex(UInventoryComponent* Comp, int32 PocketIndex)
{
    if (!Comp) return nullptr;
    switch (PocketIndex)
    {
    case 1: return &Comp->Pocket1Items;
    case 2: return &Comp->Pocket2Items;
    case 3: return &Comp->Pocket3Items;
    case 4: return &Comp->Pocket4Items;
    default: return nullptr;
    }
}

bool UInventoryComponent::IsRectFreeInPocket(int32 PocketIndex, int32 StartX, int32 StartY, int32 SX, int32 SY, const UInventoryItemData* Ignored) const
{
    const FIntPoint GS = GetPocketGridSize(PocketIndex);
    if (StartX < 0 || StartY < 0) return false;
    if (StartX + SX > GS.X || StartY + SY > GS.Y) return false;

    const TArray<TObjectPtr<UInventoryItemData>>* PocketItems = GetPocketArrayByIndex(const_cast<UInventoryComponent*>(this), PocketIndex);
    if (!PocketItems) return false;

    for (UInventoryItemData* Other : *PocketItems)
    {
        if (!Other || Other == Ignored) continue;

        const int32* OtherPocketIdx = PocketIndexByItem.Find(Other);
        if (!OtherPocketIdx || *OtherPocketIdx != PocketIndex) continue;

        const FIntPoint* OtherCell = PocketCellByItem.Find(Other);
        if (!OtherCell) continue;

        const bool bOtherRot = PocketRotByItem.Contains(Other) ? PocketRotByItem[Other] : false;
        const int32 OtherSX = bOtherRot ? FMath::Max(1, Other->SizeInCellsY) : FMath::Max(1, Other->SizeInCellsX);
        const int32 OtherSY = bOtherRot ? FMath::Max(1, Other->SizeInCellsX) : FMath::Max(1, Other->SizeInCellsY);

        const int32 Ax0 = StartX;
        const int32 Ay0 = StartY;
        const int32 Ax1 = StartX + SX;
        const int32 Ay1 = StartY + SY;

        const int32 Bx0 = OtherCell->X;
        const int32 By0 = OtherCell->Y;
        const int32 Bx1 = OtherCell->X + OtherSX;
        const int32 By1 = OtherCell->Y + OtherSY;

        const bool overlapX = !(Ax1 <= Bx0 || Bx1 <= Ax0);
        const bool overlapY = !(Ay1 <= By0 || By1 <= Ay0);
        if (overlapX && overlapY)
        {
            return false;
        }
    }
    return true;
}

bool UInventoryComponent::FindPlacementInPocket(int32 PocketIndex, const UInventoryItemData* Item, int32& OutX, int32& OutY, bool& bOutRot) const
{
    if (!Item) return false;
    const FIntPoint GS = GetPocketGridSize(PocketIndex);
    const int32 GX = FMath::Max(1, GS.X);
    const int32 GY = FMath::Max(1, GS.Y);

    for (int32 y = 0; y < GY; ++y)
    {
        for (int32 x = 0; x < GX; ++x)
        {
            int32 SX = FMath::Max(1, Item->SizeInCellsX);
            int32 SY = FMath::Max(1, Item->SizeInCellsY);
            if (IsRectFreeInPocket(PocketIndex, x, y, SX, SY, Item))
            {
                OutX = x;
                OutY = y;
                bOutRot = false;
                return true;
            }
            if (Item->bRotatable)
            {
                SX = FMath::Max(1, Item->SizeInCellsY);
                SY = FMath::Max(1, Item->SizeInCellsX);
                if (IsRectFreeInPocket(PocketIndex, x, y, SX, SY, Item))
                {
                    OutX = x;
                    OutY = y;
                    bOutRot = true;
                    return true;
                }
            }
        }
    }
    return false;
}

bool UInventoryComponent::TryPickupItem(UInventoryItemData* Item)
{
    if (!Item)
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("❌ TryPickupItem: Item is null"));
        return false;
    }

    // Только гарантируем минимум 1x1, но НЕ меняем размер по внутреннему хранилищу.
    Item->SizeInCellsX = FMath::Max(1, Item->SizeInCellsX);
    Item->SizeInCellsY = FMath::Max(1, Item->SizeInCellsY);

    // 1) Если предмет экипируемый и слот свободен — экипируем
    if (UEquippableItemData* Eq = Cast<UEquippableItemData>(Item))
    {
        // Спец-правило для бронежилета:
        // - если слот Armor занят
        // - ИЛИ если это бронежилет с хотя бы одним модом и уже надета разгрузка
        //   кладём ТОЛЬКО в рюкзак (если он надет и есть место), иначе не подбираем
        const bool bArmorSlotOccupied = EquipmentSlots.Contains(Armor);
        const bool bArmorWithModsAndVestEquipped =
            (Eq->EquipmentSlot == Armor && Eq->InstalledMods.Num() > 0 && GetEquippedItem(Vest) != nullptr);

        if (Eq->EquipmentSlot == Armor && (bArmorSlotOccupied || bArmorWithModsAndVestEquipped))
        {
            if (UEquippableItemData* EquippedBackpack = GetEquippedItem(Backpack))
            {
                int32 CellX = 0, CellY = 0; bool bRot = false;
                // Воспользуемся той же раскладкой, что и для обычного помещения в рюкзак
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
                    for (int32 y = 0; y < GS.Y; ++y)
                    {
                        for (int32 x = 0; x < GS.X; ++x)
                        {
                            int32 SX = FMath::Max(1, It->SizeInCellsX);
                            int32 SY = FMath::Max(1, It->SizeInCellsY);
                            if (IsRectFree(Equip, x, y, SX, SY, It))
                            {
                                OutX = x; OutY = y; bOutRot = false; return true;
                            }
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
            if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("⚠️ Armor pickup failed: Armor slot occupied and no backpack space"));
            return false;
        }

        // Спец-правило для рюкзака (BackpackBase):
        // - если слот Backpack свободен: экипируем (ниже по общей логике)
        // - если слот Backpack занят: пытаемся положить рюкзак внутрь хранилища экипированного рюкзака
        //   (и НЕ кладём в карманы/жилет)
        if (Eq->EquipmentSlot == Backpack && EquipmentSlots.Contains(Backpack))
        {
            if (UEquippableItemData* EquippedBackpack = GetEquippedItem(Backpack))
            {
                int32 GridIdx = 0;
                int32 CellX = 0, CellY = 0; bool bRot = false;
                auto GetGridSizes = [](const UEquippableItemData* Equip) -> TArray<FIntPoint>
                {
                    if (!Equip) return {};
                    if (Equip->AdditionalGrids.Num() > 0) return Equip->AdditionalGrids;
                    return { FIntPoint(FMath::Max(1, Equip->AdditionalGridSize.X), FMath::Max(1, Equip->AdditionalGridSize.Y)) };
                };
                auto GetItemGridIdx = [](const UEquippableItemData* Equip, const UInventoryItemData* It) -> int32
                {
                    if (!Equip || !It) return 0;
                    if (const int32* G = Equip->PersistentGridByItem.Find(It)) return *G;
                    if (const int32* G2 = Equip->StoredGridByItem.Find(It)) return *G2;
                    return 0;
                };
                auto IsRectFree = [&](const UEquippableItemData* Equip, int32 InGridIdx, int32 StartX, int32 StartY, int32 SX, int32 SY, const UInventoryItemData* Ignored) -> bool
                {
                    if (!Equip) return false;
                    const TArray<FIntPoint> Grids = GetGridSizes(Equip);
                    if (!Grids.IsValidIndex(InGridIdx)) return false;
                    const FIntPoint GS(FMath::Max(1, Grids[InGridIdx].X), FMath::Max(1, Grids[InGridIdx].Y));
                    if (StartX < 0 || StartY < 0) return false;
                    if (StartX + SX > GS.X || StartY + SY > GS.Y) return false;
                    for (const TPair<TObjectPtr<UInventoryItemData>, FIntPoint>& Pair : Equip->StoredCellByItem)
                    {
                        UInventoryItemData* Other = Pair.Key;
                        if (!Other || Other == Ignored) continue;
                        if (GetItemGridIdx(Equip, Other) != InGridIdx) continue;
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
                auto FindPlacement = [&](UEquippableItemData* Equip, const UInventoryItemData* It, int32& OutGridIdx, int32& OutX, int32& OutY, bool& bOutRot) -> bool
                {
                    if (!Equip || !It || !Equip->bHasAdditionalStorage) return false;
                    const TArray<FIntPoint> Grids = GetGridSizes(Equip);
                    for (int32 g = 0; g < Grids.Num(); ++g)
                    {
                        const int32 GX = FMath::Max(1, Grids[g].X);
                        const int32 GY = FMath::Max(1, Grids[g].Y);
                        for (int32 y = 0; y < GY; ++y)
                        {
                            for (int32 x = 0; x < GX; ++x)
                            {
                                int32 SX = FMath::Max(1, It->SizeInCellsX);
                                int32 SY = FMath::Max(1, It->SizeInCellsY);
                                if (IsRectFree(Equip, g, x, y, SX, SY, It))
                                {
                                    OutGridIdx = g; OutX = x; OutY = y; bOutRot = false; return true;
                                }
                                if (It->bRotatable)
                                {
                                    SX = FMath::Max(1, It->SizeInCellsY);
                                    SY = FMath::Max(1, It->SizeInCellsX);
                                    if (IsRectFree(Equip, g, x, y, SX, SY, It))
                                    {
                                        OutGridIdx = g; OutX = x; OutY = y; bOutRot = true; return true;
                                    }
                                }
                            }
                        }
                    }
                    return false;
                };

                if (FindPlacement(EquippedBackpack, Item, GridIdx, CellX, CellY, bRot))
                {
                    RemoveFromAnyStorage(Item);
                    if (AddToEquipmentStorage(EquippedBackpack, Item))
                    {
                        EquippedBackpack->StoredGridByItem.Add(Item, GridIdx);
                        EquippedBackpack->PersistentGridByItem.Add(Item, GridIdx);
                        EquippedBackpack->StoredCellByItem.Add(Item, FIntPoint(CellX, CellY));
                        EquippedBackpack->PersistentCellByItem.Add(Item, FIntPoint(CellX, CellY));
                        EquippedBackpack->StoredRotByItem.Add(Item, bRot);
                        EquippedBackpack->PersistentRotByItem.Add(Item, bRot);
                        return true;
                    }
                }
            }

            if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("⚠️ Backpack pickup failed: Backpack slot occupied and no backpack storage space"));
            return false;
        }

        // Спец-правило для жилета (VestBase):
        // - если слот Vest занят ИЛИ на бронежилете уже стоят моды:
        //   кладём ТОЛЬКО в хранилище надетого рюкзака (если он надет и есть место), иначе не подбираем
        if (Eq->EquipmentSlot == Vest &&
            (EquipmentSlots.Contains(Vest) ||
             ([this]() -> bool
             {
                 if (UEquippableItemData* EquippedArmor = GetEquippedItem(Armor))
                 {
                     return EquippedArmor->InstalledMods.Num() > 0;
                 }
                 return false;
             })()))
        {
            if (UEquippableItemData* EquippedBackpack = GetEquippedItem(Backpack))
            {
                int32 GridIdx = 0;
                int32 CellX = 0, CellY = 0; bool bRot = false;
                auto GetGridSizes = [](const UEquippableItemData* Equip) -> TArray<FIntPoint>
                {
                    if (!Equip) return {};
                    if (Equip->AdditionalGrids.Num() > 0) return Equip->AdditionalGrids;
                    return { FIntPoint(FMath::Max(1, Equip->AdditionalGridSize.X), FMath::Max(1, Equip->AdditionalGridSize.Y)) };
                };
                auto GetItemGridIdx = [](const UEquippableItemData* Equip, const UInventoryItemData* It) -> int32
                {
                    if (!Equip || !It) return 0;
                    if (const int32* G = Equip->PersistentGridByItem.Find(It)) return *G;
                    if (const int32* G2 = Equip->StoredGridByItem.Find(It)) return *G2;
                    return 0;
                };
                auto IsRectFree = [&](const UEquippableItemData* Equip, int32 StartX, int32 StartY, int32 SX, int32 SY, const UInventoryItemData* Ignored) -> bool
                {
                    if (!Equip) return false;
                    const TArray<FIntPoint> Grids = GetGridSizes(Equip);
                    if (!Grids.IsValidIndex(GridIdx)) return false;
                    const FIntPoint GS(FMath::Max(1, Grids[GridIdx].X), FMath::Max(1, Grids[GridIdx].Y));
                    if (StartX < 0 || StartY < 0) return false;
                    if (StartX + SX > GS.X || StartY + SY > GS.Y) return false;
                    for (const TPair<TObjectPtr<UInventoryItemData>, FIntPoint>& Pair : Equip->StoredCellByItem)
                    {
                        UInventoryItemData* Other = Pair.Key;
                        if (!Other || Other == Ignored) continue;
                        if (GetItemGridIdx(Equip, Other) != GridIdx) continue;
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
                    const TArray<FIntPoint> Grids = GetGridSizes(Equip);
                    for (int32 g = 0; g < Grids.Num(); ++g)
                    {
                        GridIdx = g;
                        const int32 GX = FMath::Max(1, Grids[g].X);
                        const int32 GY = FMath::Max(1, Grids[g].Y);
                        for (int32 y = 0; y < GY; ++y)
                        {
                            for (int32 x = 0; x < GX; ++x)
                            {
                                int32 SX = FMath::Max(1, It->SizeInCellsX);
                                int32 SY = FMath::Max(1, It->SizeInCellsY);
                                if (IsRectFree(Equip, x, y, SX, SY, It))
                                {
                                    OutX = x; OutY = y; bOutRot = false; return true;
                                }
                                if (It->bRotatable)
                                {
                                    SX = FMath::Max(1, It->SizeInCellsY);
                                    SY = FMath::Max(1, It->SizeInCellsX);
                                    if (IsRectFree(Equip, x, y, SX, SY, It))
                                    {
                                        OutX = x; OutY = y; bOutRot = true; return true;
                                    }
                                }
                            }
                        }
                    }
                    return false;
                };

                if (FindPlacement(EquippedBackpack, Item, CellX, CellY, bRot))
                {
                    RemoveFromAnyStorage(Item);
                    if (AddToEquipmentStorage(EquippedBackpack, Item))
                    {
                        EquippedBackpack->StoredGridByItem.Add(Item, GridIdx);
                        EquippedBackpack->PersistentGridByItem.Add(Item, GridIdx);
                        EquippedBackpack->StoredCellByItem.Add(Item, FIntPoint(CellX, CellY));
                        EquippedBackpack->PersistentCellByItem.Add(Item, FIntPoint(CellX, CellY));
                        EquippedBackpack->StoredRotByItem.Add(Item, bRot);
                        EquippedBackpack->PersistentRotByItem.Add(Item, bRot);
                        return true;
                    }
                }
            }

            if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("⚠️ Vest pickup failed: Vest slot occupied and no backpack storage space"));
            return false;
        }

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

    // 2) Пытаемся положить в карманы.
    // Важно: сюда попадают ТОЛЬКО неэкипируемые предметы. Размер (в т.ч. модов с большим хранилищем)
    // учитывается в FindPlacementInPocket: если габариты предмета больше, чем грид кармана, он просто не влезет.
    {
        const bool bIsEquippable = Cast<UEquippableItemData>(Item) != nullptr;
        if (!bIsEquippable)
        {
            for (int32 PocketIdx = 1; PocketIdx <= 4; ++PocketIdx)
            {
                int32 CellX = 0, CellY = 0;
                bool bRot = false;
                if (FindPlacementInPocket(PocketIdx, Item, CellX, CellY, bRot))
                {
                    RemoveFromAnyStorage(Item);
                    if (TArray<TObjectPtr<UInventoryItemData>>* Target = GetPocketArrayByIndex(this, PocketIdx))
                    {
                        Target->Add(Item);
                        PocketIndexByItem.Add(Item, PocketIdx);
                        PocketCellByItem.Add(Item, FIntPoint(CellX, CellY));
                        PocketRotByItem.Add(Item, bRot);
                        return true;
                    }
                }
            }
        }
    }

    // 3) Пытаемся положить в рюкзак с проверкой соседних свободных клеток и автоповоротом
    if (UEquippableItemData* EquippedBackpack = GetEquippedItem(Backpack))
    {
        int32 GridIdx = 0;
        int32 CellX=0, CellY=0; bool bRot=false;

        auto GetGridSizes = [](const UEquippableItemData* Equip) -> TArray<FIntPoint>
        {
            if (!Equip) return {};
            if (Equip->AdditionalGrids.Num() > 0) return Equip->AdditionalGrids;
            return { FIntPoint(FMath::Max(1, Equip->AdditionalGridSize.X), FMath::Max(1, Equip->AdditionalGridSize.Y)) };
        };
        auto GetItemGridIdx = [](const UEquippableItemData* Equip, const UInventoryItemData* It) -> int32
        {
            if (!Equip || !It) return 0;
            if (const int32* G = Equip->PersistentGridByItem.Find(It)) return *G;
            if (const int32* G2 = Equip->StoredGridByItem.Find(It)) return *G2;
            return 0;
        };
        auto IsRectFreeInGrid = [&](const UEquippableItemData* Equip, int32 InGridIdx, int32 StartX, int32 StartY, int32 SX, int32 SY, const UInventoryItemData* Ignored) -> bool
        {
            if (!Equip) return false;
            const TArray<FIntPoint> Grids = GetGridSizes(Equip);
            if (!Grids.IsValidIndex(InGridIdx)) return false;
            const FIntPoint GS(FMath::Max(1, Grids[InGridIdx].X), FMath::Max(1, Grids[InGridIdx].Y));
            if (StartX < 0 || StartY < 0) return false;
            if (StartX + SX > GS.X || StartY + SY > GS.Y) return false;
            for (const TPair<TObjectPtr<UInventoryItemData>, FIntPoint>& Pair : Equip->StoredCellByItem)
            {
                UInventoryItemData* Other = Pair.Key;
                if (!Other || Other == Ignored) continue;
                if (GetItemGridIdx(Equip, Other) != InGridIdx) continue;
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
        auto FindPlacementInBackpack = [&](UEquippableItemData* Equip, const UInventoryItemData* It, int32& OutGridIdx, int32& OutX, int32& OutY, bool& bOutRot) -> bool
        {
            if (!Equip || !It || !Equip->bHasAdditionalStorage) return false;
            const TArray<FIntPoint> Grids = GetGridSizes(Equip);
            for (int32 g = 0; g < Grids.Num(); ++g)
            {
                const int32 GX = FMath::Max(1, Grids[g].X);
                const int32 GY = FMath::Max(1, Grids[g].Y);
                for (int32 y = 0; y < GY; ++y)
                {
                    for (int32 x = 0; x < GX; ++x)
                    {
                        int32 SX = FMath::Max(1, It->SizeInCellsX);
                        int32 SY = FMath::Max(1, It->SizeInCellsY);
                        if (IsRectFreeInGrid(Equip, g, x, y, SX, SY, It))
                        {
                            OutGridIdx = g; OutX = x; OutY = y; bOutRot = false; return true;
                        }
                        if (It->bRotatable)
                        {
                            SX = FMath::Max(1, It->SizeInCellsY);
                            SY = FMath::Max(1, It->SizeInCellsX);
                            if (IsRectFreeInGrid(Equip, g, x, y, SX, SY, It))
                            {
                                OutGridIdx = g; OutX = x; OutY = y; bOutRot = true; return true;
                            }
                        }
                    }
                }
            }
            return false;
        };

        if (FindPlacementInBackpack(EquippedBackpack, Item, GridIdx, CellX, CellY, bRot))
        {
            RemoveFromAnyStorage(Item);
            if (AddToEquipmentStorage(EquippedBackpack, Item))
            {
                EquippedBackpack->StoredGridByItem.Add(Item, GridIdx);
                EquippedBackpack->PersistentGridByItem.Add(Item, GridIdx);
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

bool UInventoryComponent::UninstallArmorMod(UEquippableItemData* ArmorItemData, int32 InstalledModIndex)
{
    if (!ArmorItemData || InstalledModIndex < 0 || InstalledModIndex >= ArmorItemData->InstalledMods.Num())
    {
        return false;
    }
    // Броня должна принадлежать игроку: экипирована или в рюкзаке
    const bool bIsEquipped = (GetEquippedItem(Armor) == ArmorItemData);
    const bool bInBackpack = BackpackItems.Contains(ArmorItemData);
    if (!bIsEquipped && !bInBackpack)
    {
        return false;
    }

    UInventoryItemData* RemovedMod = ArmorItemData->UninstallMod(InstalledModIndex);
    if (!RemovedMod)
    {
        return false;
    }

    if (TryPickupItem(RemovedMod))
    {
        if (bIsEquipped)
        {
            if (ACharacter* OwnerChar = Cast<ACharacter>(GetOwner()))
            {
                if (UEquipmentComponent* EquipComp = OwnerChar->FindComponentByClass<UEquipmentComponent>())
                {
                    EquipComp->RefreshEquippedArmorMods();
                }
            }
        }
        return true;
    }

    // Не хватило места — выбрасываем мод в мир
    ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
    if (!OwnerChar) return true; // мод уже снят с брони, считаем успехом
    UWorld* World = OwnerChar->GetWorld();
    if (!World) return true;

    const FVector SpawnLoc = OwnerChar->GetActorLocation() + OwnerChar->GetActorForwardVector() * 80.f;
    FVector ViewLoc;
    FRotator ViewRot;
    OwnerChar->GetActorEyesViewPoint(ViewLoc, ViewRot);
    FActorSpawnParameters S;
    S.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    if (TSubclassOf<AActor> DropClass = GetPickupClassForItem_Internal(RemovedMod))
    {
        if (AActor* SpawnedActor = World->SpawnActor<AActor>(DropClass, SpawnLoc, ViewRot, S))
        {
            if (APickupBase* Spawned = Cast<APickupBase>(SpawnedActor))
            {
                Spawned->ItemInstance = RemovedMod;
                Spawned->ApplyItemInstanceVisuals();
            }
            else if (AEquipmentBase* AsEquip = Cast<AEquipmentBase>(SpawnedActor))
            {
                AsEquip->ItemInstance = RemovedMod;
                AsEquip->ApplyItemInstanceVisuals();
            }
        }
    }

    if (bIsEquipped)
    {
        if (UEquipmentComponent* EquipComp = OwnerChar->FindComponentByClass<UEquipmentComponent>())
        {
            EquipComp->RefreshEquippedArmorMods();
        }
    }
    return true;
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

    // В карман не кладём экипируемые предметы (броня, оружие и т.п.).
    const bool bIsEquippable = Cast<UEquippableItemData>(Item) != nullptr;
    if (bIsEquippable)
    {
        return false;
    }
    Item->SizeInCellsX = FMath::Max(1, Item->SizeInCellsX);
    Item->SizeInCellsY = FMath::Max(1, Item->SizeInCellsY);

    // ВАЖНО: если предмет был экипирован (лежит в слоте), сперва снимаем его,
    // иначе он останется в слоте и продублируется в кармане.
    // RemoveFromAnyStorage() умеет снимать экипированные предметы без выбрасывания в мир.
    RemoveFromAnyStorage(Item);

    if (PocketIndex < 1 || PocketIndex > 4)
    {
        return false;
    }

    // Ищем первое подходящее место в указанном кармане, как в жилете/рюкзаке.
    int32 CellX = 0, CellY = 0;
    bool bRot = false;
    if (!FindPlacementInPocket(PocketIndex, Item, CellX, CellY, bRot))
    {
        return false;
    }

    if (TArray<TObjectPtr<UInventoryItemData>>* Target = GetPocketArrayByIndex(this, PocketIndex))
    {
        Target->Add(Item);
        PocketIndexByItem.Add(Item, PocketIndex);
        PocketCellByItem.Add(Item, FIntPoint(CellX, CellY));
        PocketRotByItem.Add(Item, bRot);
        return true;
    }

    return false;
}

bool UInventoryComponent::MoveItemToPocketAt(int32 PocketIndex, int32 CellX, int32 CellY, bool bRotated, UInventoryItemData* Item)
{
    if (!Item) return false;

    // В карман не кладём экипируемые предметы.
    const bool bIsEquippable = Cast<UEquippableItemData>(Item) != nullptr;
    if (bIsEquippable)
    {
        return false;
    }
    Item->SizeInCellsX = FMath::Max(1, Item->SizeInCellsX);
    Item->SizeInCellsY = FMath::Max(1, Item->SizeInCellsY);

    if (PocketIndex < 1 || PocketIndex > 4)
    {
        return false;
    }

    // Размер предмета с учётом поворота
    const int32 SX = bRotated ? FMath::Max(1, Item->SizeInCellsY) : FMath::Max(1, Item->SizeInCellsX);
    const int32 SY = bRotated ? FMath::Max(1, Item->SizeInCellsX) : FMath::Max(1, Item->SizeInCellsY);
    // Проверяем, свободен ли прямоугольник в кармане (игнорируя сам предмет, если он уже там лежит)
    if (!IsRectFreeInPocket(PocketIndex, CellX, CellY, SX, SY, Item))
    {
        return false;
    }

    // Теперь можно безопасно удалить предмет из всех хранилищ и положить его в новый карман/ячейку
    RemoveFromAnyStorage(Item);

    if (TArray<TObjectPtr<UInventoryItemData>>* Target = GetPocketArrayByIndex(this, PocketIndex))
    {
        Target->Add(Item);
        PocketIndexByItem.Add(Item, PocketIndex);
        PocketCellByItem.Add(Item, FIntPoint(CellX, CellY));
        PocketRotByItem.Add(Item, bRotated);
        return true;
    }

    return false;
}







