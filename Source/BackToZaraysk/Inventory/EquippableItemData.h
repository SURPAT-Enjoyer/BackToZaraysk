#pragma once

#include "CoreMinimal.h"
#include "InventoryItemData.h"
#include "EquipmentSlotType.h"
#include "EquippableItemData.generated.h"

/**
 * Данные экипируемого предмета
 */
UCLASS(BlueprintType, Blueprintable, meta=(DisplayName="Equippable Item Data"))
class BACKTOZARAYSK_API UEquippableItemData : public UInventoryItemData
{
    GENERATED_BODY()
    
public:
    UEquippableItemData();
    
    // Тип слота экипировки
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Equipment")
    TEnumAsByte<EEquipmentSlotType> EquipmentSlot;
    
    // Меш для отображения на персонаже
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Equipment|Visual")
    USkeletalMesh* EquippedMesh = nullptr;
    
    // Имя сокета для прикрепления к персонажу
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Equipment|Visual")
    FName AttachSocketName = NAME_None;
    
    // Относительный трансформ при экипировке
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Equipment|Visual")
    FTransform RelativeTransform;
    
    // Экипирован ли предмет в данный момент
    UPROPERTY(BlueprintReadOnly, Category="Equipment|State")
    bool bIsEquipped = false;
    
    // Размер дополнительного грида для хранения предметов (например, карманы жилета)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Equipment|Storage", meta=(EditCondition="bHasAdditionalStorage"))
    FIntPoint AdditionalGridSize = FIntPoint(0, 0);
    
    // Есть ли дополнительный грид для хранения предметов
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Equipment|Storage")
    bool bHasAdditionalStorage = false;
};

/**
 * Данные тактического жилета 3x3
 */
UCLASS(BlueprintType, Blueprintable, meta=(DisplayName="Tactical Vest Item Data"))
class BACKTOZARAYSK_API UTacticalVestItemData : public UEquippableItemData
{
    GENERATED_BODY()
    
public:
    UTacticalVestItemData();
};

