#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BackToZaraysk/Inventory/EquipmentSlotType.h"
#include "EquipmentComponent.generated.h"

class UEquippableItemData;
class USkeletalMeshComponent;

/**
 * Компонент для управления экипировкой персонажа
 * Отображает экипированные предметы на модели персонажа
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BACKTOZARAYSK_API UEquipmentComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEquipmentComponent();

    // Экипировать предмет
    UFUNCTION(BlueprintCallable, Category="Equipment")
    bool EquipItem(UEquippableItemData* ItemData);
    
    // Снять предмет
    UFUNCTION(BlueprintCallable, Category="Equipment")
    bool UnequipItem(EEquipmentSlotType SlotType);
    
    // Получить экипированный предмет из слота
    UFUNCTION(BlueprintCallable, Category="Equipment")
    UEquippableItemData* GetEquippedItem(EEquipmentSlotType SlotType) const;
    
    // Проверить, занят ли слот
    UFUNCTION(BlueprintCallable, Category="Equipment")
    bool IsSlotOccupied(EEquipmentSlotType SlotType) const;

protected:
    virtual void BeginPlay() override;

private:
    // Карта экипированных предметов: Слот -> ItemData
    UPROPERTY()
    TMap<EEquipmentSlotType, UEquippableItemData*> EquippedItems;
    
    // Карта компонентов мешей: Слот -> SkeletalMeshComponent
    UPROPERTY()
    TMap<EEquipmentSlotType, USkeletalMeshComponent*> EquipmentMeshComponents;
    
    // Ссылка на меш персонажа
    UPROPERTY()
    USkeletalMeshComponent* CharacterMesh;
    
    // Создать и прикрепить компонент меша для экипировки
    USkeletalMeshComponent* CreateEquipmentMeshComponent(EEquipmentSlotType SlotType, UEquippableItemData* ItemData);
    
    // Удалить компонент меша экипировки
    void RemoveEquipmentMeshComponent(EEquipmentSlotType SlotType);
};




