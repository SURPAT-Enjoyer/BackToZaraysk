#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryItemData.h"
#include "EquipmentSlotType.h"
#include "InventoryComponent.generated.h"

class UEquippableItemData;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BACKTOZARAYSK_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UInventoryComponent();

	// Простой рюкзак: список предметов
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	TArray<TObjectPtr<UInventoryItemData>> BackpackItems;
	
	// Карта для сохранения позиций предметов при экипировке
	UPROPERTY()
	TMap<TObjectPtr<UInventoryItemData>, int32> ItemPositions;

	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool AddToBackpack(UInventoryItemData* Item);

	UFUNCTION(BlueprintCallable, Category="Inventory")
	UInventoryItemData* RemoveLastFromBackpack();

	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool RemoveSpecificFromBackpack(UInventoryItemData* Item);
	
	// Функция для восстановления предмета на исходной позиции
	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool RestoreItemToPosition(UInventoryItemData* Item);

	// Слоты экипировки
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Equipment Slots")
	TMap<TEnumAsByte<EEquipmentSlotType>, UEquippableItemData*> EquipmentSlots;
	
	// Дополнительные инвентари для экипированных предметов (например, карманы жилета)
	TMap<TObjectPtr<UEquippableItemData>, TArray<TObjectPtr<UInventoryItemData>>> EquipmentStorage;

	// Экипировать предмет из инвентаря
	UFUNCTION(BlueprintCallable, Category="Equipment")
	bool EquipItemFromInventory(UEquippableItemData* Item);

	// Снять предмет в инвентарь
	UFUNCTION(BlueprintCallable, Category="Equipment")
	bool UnequipItemToInventory(EEquipmentSlotType SlotType, bool bDropToWorld = false);
	
	// Функции для работы с дополнительными инвентарями экипировки
	UFUNCTION(BlueprintCallable, Category="Equipment Storage")
	bool AddToEquipmentStorage(UEquippableItemData* Equipment, UInventoryItemData* Item);
	
	UFUNCTION(BlueprintCallable, Category="Equipment Storage")
	bool RemoveFromEquipmentStorage(UEquippableItemData* Equipment, UInventoryItemData* Item);
    // Полное удаление: из оперативного и персистентного хранилища, с очисткой позиций
    UFUNCTION(BlueprintCallable, Category="Equipment Storage")
    bool RemoveCompletelyFromEquipmentStorage(UEquippableItemData* Equipment, UInventoryItemData* Item);
	
	UFUNCTION(BlueprintCallable, Category="Equipment Storage")
	TArray<UInventoryItemData*> GetEquipmentStorageItems(UEquippableItemData* Equipment);

    // Удалить предмет из любого хранения (рюкзак, хранилища экипировки, пояс и карманы)
    UFUNCTION(BlueprintCallable, Category="Inventory")
    bool RemoveFromAnyStorage(UInventoryItemData* Item);

    // Перемещение предметов по DnD
    UFUNCTION(BlueprintCallable, Category="Inventory")
    bool MoveItemToVest(UInventoryItemData* Item);
    UFUNCTION(BlueprintCallable, Category="Inventory")
    bool MoveItemToPocket(int32 PocketIndex, UInventoryItemData* Item);

    // Получить экипированный предмет
	UFUNCTION(BlueprintCallable, Category="Equipment")
	UEquippableItemData* GetEquippedItem(EEquipmentSlotType SlotType) const;

	/** Снять мод с брони по индексу. Мод и его содержимое возвращаются в инвентарь; при нехватке места — выброс в мир. */
	UFUNCTION(BlueprintCallable, Category="Equipment|Mods")
	bool UninstallArmorMod(UEquippableItemData* ArmorItemData, int32 InstalledModIndex);
	
	// Синхронизировать состояние с EquipmentComponent
	UFUNCTION(BlueprintCallable, Category="Equipment")
	bool SyncWithEquipmentComponent();

    // Новая логика подбора: попытаться экипировать или поместить в хранилища по приоритету
    UFUNCTION(BlueprintCallable, Category="Inventory")
    bool TryPickupItem(UInventoryItemData* Item);

    // Пояс и карманы (теперь карманы многоклеточные)
    TArray<TObjectPtr<UInventoryItemData>> BeltStorageItems; // 8x2 как раньше
    TArray<TObjectPtr<UInventoryItemData>> Pocket1Items;
    TArray<TObjectPtr<UInventoryItemData>> Pocket2Items;
    TArray<TObjectPtr<UInventoryItemData>> Pocket3Items;
    TArray<TObjectPtr<UInventoryItemData>> Pocket4Items;

    // Отображение предметов в карманах: какой карман и какая ячейка внутри него занята предметом
    UPROPERTY()
    TMap<TObjectPtr<UInventoryItemData>, int32> PocketIndexByItem;   // 1..4

    UPROPERTY()
    TMap<TObjectPtr<UInventoryItemData>, FIntPoint> PocketCellByItem; // CellX,CellY в пределах мини-грида

    UPROPERTY()
    TMap<TObjectPtr<UInventoryItemData>, bool> PocketRotByItem;

    bool HasSpaceInGridLike(const FIntPoint& GridSize, const TArray<TObjectPtr<UInventoryItemData>>& Items, int32 ItemSizeX, int32 ItemSizeY) const;
    bool AddToGridLike(TArray<TObjectPtr<UInventoryItemData>>& Items, const FIntPoint& GridSize, UInventoryItemData* Item);

    // Прямое размещение предмета в указанную ячейку кармана (используется DnD по ячейкам)
    bool MoveItemToPocketAt(int32 PocketIndex, int32 CellX, int32 CellY, bool bRotated, UInventoryItemData* Item);

private:
    // Вспомогательные функции для многоклеточных карманов
    FIntPoint GetPocketGridSize(int32 PocketIndex) const;
    bool IsRectFreeInPocket(int32 PocketIndex, int32 StartX, int32 StartY, int32 SX, int32 SY, const UInventoryItemData* Ignored) const;
    bool FindPlacementInPocket(int32 PocketIndex, const UInventoryItemData* Item, int32& OutX, int32& OutY, bool& bOutRot) const;
};







