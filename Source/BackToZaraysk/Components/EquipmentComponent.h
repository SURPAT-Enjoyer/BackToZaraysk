#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BackToZaraysk/Inventory/EquipmentSlotType.h"
#include "Components/SceneComponent.h"
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
    bool UnequipItem(EEquipmentSlotType SlotType, bool bDropToWorld = false);
    
    // Получить экипированный предмет из слота
    UFUNCTION(BlueprintCallable, Category="Equipment")
    UEquippableItemData* GetEquippedItem(EEquipmentSlotType SlotType) const;
    
    // Проверить, занят ли слот
    UFUNCTION(BlueprintCallable, Category="Equipment")
    bool IsSlotOccupied(EEquipmentSlotType SlotType) const;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    // Карта экипированных предметов: Слот -> ItemData
    UPROPERTY()
    TMap<TEnumAsByte<EEquipmentSlotType>, UEquippableItemData*> EquippedItems;
    
    // Карта визуальных компонентов экипировки: Слот -> SceneComponent (SkeletalMeshComponent или StaticMeshComponent)
    UPROPERTY()
    TMap<TEnumAsByte<EEquipmentSlotType>, USceneComponent*> EquipmentMeshComponents;
    
    // Ссылка на меш персонажа
    UPROPERTY()
    USkeletalMeshComponent* CharacterMesh;

    // Для некоторых слотов (например, Helmet) можем фоллоуить сокет вручную, если меш персонажа скрыт.
    struct FBoneFollowEntry
    {
        FName SocketName = NAME_None;
        FTransform RelativeToSocket = FTransform::Identity;
    };

    TMap<TEnumAsByte<EEquipmentSlotType>, FBoneFollowEntry> BoneFollowBySlot;
    
    // Создать и прикрепить компонент меша для экипировки (Skeletal или Static)
    USceneComponent* CreateEquipmentMeshComponent(EEquipmentSlotType SlotType, UEquippableItemData* ItemData);
    
    // Удалить компонент меша экипировки
    void RemoveEquipmentMeshComponent(EEquipmentSlotType SlotType);
};




