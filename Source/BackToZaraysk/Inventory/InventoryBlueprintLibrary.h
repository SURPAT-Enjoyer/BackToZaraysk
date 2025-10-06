#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EquipmentSlotType.h"
#include "InventoryBlueprintLibrary.generated.h"

class UEquippableItemData;
class APlayerCharacter;

/**
 * Blueprint функции для работы с инвентарем и экипировкой
 */
UCLASS()
class BACKTOZARAYSK_API UInventoryBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // === ФУНКЦИИ ДЛЯ ЭКИПИРОВКИ ===
    
    /**
     * Экипировать предмет на игрока
     * @param PlayerCharacter - Персонаж игрока
     * @param Item - Экипируемый предмет
     * @return true если успешно экипировано
     */
    UFUNCTION(BlueprintCallable, Category="Inventory|Equipment", meta=(WorldContext="WorldContextObject"))
    static bool EquipItemOnPlayer(UObject* WorldContextObject, UEquippableItemData* Item);
    
    /**
     * Снять предмет со слота
     * @param PlayerCharacter - Персонаж игрока
     * @param SlotType - Тип слота экипировки
     * @return true если успешно снято
     */
    UFUNCTION(BlueprintCallable, Category="Inventory|Equipment", meta=(WorldContext="WorldContextObject"))
    static bool UnequipItemFromSlot(UObject* WorldContextObject, EEquipmentSlotType SlotType);
    
    /**
     * Получить экипированный предмет из слота
     * @param PlayerCharacter - Персонаж игрока
     * @param SlotType - Тип слота экипировки
     * @return Экипированный предмет или nullptr
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Inventory|Equipment", meta=(WorldContext="WorldContextObject"))
    static UEquippableItemData* GetEquippedItemFromSlot(UObject* WorldContextObject, EEquipmentSlotType SlotType);
    
    /**
     * Проверить, занят ли слот экипировки
     * @param PlayerCharacter - Персонаж игрока
     * @param SlotType - Тип слота экипировки
     * @return true если слот занят
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Inventory|Equipment", meta=(WorldContext="WorldContextObject"))
    static bool IsEquipmentSlotOccupied(UObject* WorldContextObject, EEquipmentSlotType SlotType);
    
    // === ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ===
    
    /**
     * Получить название слота экипировки на русском
     * @param SlotType - Тип слота
     * @return Локализованное название
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Inventory|Equipment")
    static FText GetEquipmentSlotName(EEquipmentSlotType SlotType);
    
    /**
     * Создать тестовый тактический жилет в мире
     * @param WorldContextObject - Контекст мира
     * @param SpawnLocation - Позиция спавна
     * @return Созданный актор жилета
     */
    UFUNCTION(BlueprintCallable, Category="Inventory|Debug", meta=(WorldContext="WorldContextObject"))
    static AActor* SpawnTestTacticalVest(UObject* WorldContextObject, FVector SpawnLocation);

private:
    // Получить персонажа игрока
    static APlayerCharacter* GetPlayerCharacter(UObject* WorldContextObject);
};




