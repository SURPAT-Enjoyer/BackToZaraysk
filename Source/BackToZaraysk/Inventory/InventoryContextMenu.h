#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EquipmentSlotType.h"
#include "InventoryContextMenu.generated.h"

class UEquippableItemData;
class UInventoryItemData;

/**
 * Контекстное меню для предметов инвентаря
 * Предоставляет опции: Использовать, Экипировать, Снять, Выбросить и т.д.
 */
UCLASS(BlueprintType, Blueprintable)
class BACKTOZARAYSK_API UInventoryContextMenu : public UUserWidget
{
    GENERATED_BODY()

public:
    // === ПУБЛИЧНЫЕ ФУНКЦИИ ДЛЯ BLUEPRINT ===
    
    // Показать меню для предмета
    UFUNCTION(BlueprintCallable, Category="Context Menu")
    void ShowMenuForItem(UInventoryItemData* Item, FVector2D ScreenPosition);
    
    // Скрыть меню
    UFUNCTION(BlueprintCallable, Category="Context Menu")
    void HideMenu();
    
    // === ДЕЙСТВИЯ (вызываются из Blueprint кнопками) ===
    
    // Экипировать предмет
    UFUNCTION(BlueprintCallable, Category="Context Menu|Actions")
    void EquipItem();
    
    // Снять предмет
    UFUNCTION(BlueprintCallable, Category="Context Menu|Actions")
    void UnequipItem();
    
    // Использовать предмет
    UFUNCTION(BlueprintCallable, Category="Context Menu|Actions")
    void UseItem();
    
    // Выбросить предмет
    UFUNCTION(BlueprintCallable, Category="Context Menu|Actions")
    void DropItem();
    
    // Разделить стак
    UFUNCTION(BlueprintCallable, Category="Context Menu|Actions")
    void SplitStack();
    
    // === ПРОВЕРКИ ДОСТУПНОСТИ ДЕЙСТВИЙ ===
    
    // Можно ли экипировать этот предмет?
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Context Menu|Checks")
    bool CanEquipItem() const;
    
    // Можно ли снять этот предмет?
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Context Menu|Checks")
    bool CanUnequipItem() const;
    
    // Можно ли использовать этот предмет?
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Context Menu|Checks")
    bool CanUseItem() const;

protected:
    // Текущий предмет для контекстного меню
    UPROPERTY(BlueprintReadOnly, Category="Context Menu")
    UInventoryItemData* CurrentItem = nullptr;
    
    // Является ли предмет экипируемым?
    UPROPERTY(BlueprintReadOnly, Category="Context Menu")
    bool bIsEquippableItem = false;
    
    // Экипирован ли предмет в данный момент?
    UPROPERTY(BlueprintReadOnly, Category="Context Menu")
    bool bIsItemEquipped = false;

private:
    // Обновить состояние меню на основе текущего предмета
    void UpdateMenuState();
};




