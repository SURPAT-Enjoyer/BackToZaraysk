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
	
	UFUNCTION(BlueprintCallable, Category="Equipment Storage")
	TArray<UInventoryItemData*> GetEquipmentStorageItems(UEquippableItemData* Equipment);

	// Получить экипированный предмет
	UFUNCTION(BlueprintCallable, Category="Equipment")
	UEquippableItemData* GetEquippedItem(EEquipmentSlotType SlotType) const;
	
	// Синхронизировать состояние с EquipmentComponent
	UFUNCTION(BlueprintCallable, Category="Equipment")
	bool SyncWithEquipmentComponent();

    // Новая логика подбора: попытаться экипировать или поместить в хранилища по приоритету
    UFUNCTION(BlueprintCallable, Category="Inventory")
    bool TryPickupItem(UInventoryItemData* Item);

private:
    // Простейшие хранилища пояса и карманов (гриды фиксированного размера)
    FIntPoint BeltGridSize = FIntPoint(8, 2);
    FIntPoint PocketsGridSize = FIntPoint(4, 1);
    TArray<TObjectPtr<UInventoryItemData>> BeltStorageItems;
    TArray<TObjectPtr<UInventoryItemData>> PocketsStorageItems;

    bool HasSpaceInGridLike(const FIntPoint& GridSize, const TArray<TObjectPtr<UInventoryItemData>>& Items, int32 ItemSizeX, int32 ItemSizeY) const;
    bool AddToGridLike(TArray<TObjectPtr<UInventoryItemData>>& Items, const FIntPoint& GridSize, UInventoryItemData* Item);
};







