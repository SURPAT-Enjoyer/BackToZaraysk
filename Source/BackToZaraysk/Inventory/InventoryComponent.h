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

	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool AddToBackpack(UInventoryItemData* Item);

	UFUNCTION(BlueprintCallable, Category="Inventory")
	UInventoryItemData* RemoveLastFromBackpack();

	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool RemoveSpecificFromBackpack(UInventoryItemData* Item);

	// Слоты экипировки
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Equipment Slots")
	TMap<EEquipmentSlotType, UEquippableItemData*> EquipmentSlots;

	// Экипировать предмет из инвентаря
	UFUNCTION(BlueprintCallable, Category="Equipment")
	bool EquipItemFromInventory(UEquippableItemData* Item);

	// Снять предмет в инвентарь
	UFUNCTION(BlueprintCallable, Category="Equipment")
	bool UnequipItemToInventory(EEquipmentSlotType SlotType);

	// Получить экипированный предмет
	UFUNCTION(BlueprintCallable, Category="Equipment")
	UEquippableItemData* GetEquippedItem(EEquipmentSlotType SlotType) const;
};







