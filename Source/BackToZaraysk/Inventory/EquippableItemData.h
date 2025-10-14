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
    
    // Меш для отображения на персонаже (может быть SkeletalMesh или StaticMesh)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Equipment|Visual")
    UObject* EquippedMesh = nullptr;
    
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

    // Персистентное содержимое хранилища предмета (живёт вместе с самим ItemData)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Equipment|Storage")
    TArray<TObjectPtr<class UInventoryItemData>> PersistentStorage;

    // Позиции предметов в гриде (CellX, CellY) по конкретному ItemData
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Equipment|Storage")
    TMap<TObjectPtr<class UInventoryItemData>, FIntPoint> StoredCellByItem;

    // Персистентная копия позиций для переживания выброса/подбора
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Equipment|Storage")
    TMap<TObjectPtr<class UInventoryItemData>, FIntPoint> PersistentCellByItem;
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

/**
 * Данные рюкзака 8x6
 */
UCLASS(BlueprintType, Blueprintable, meta=(DisplayName="Backpack Item Data"))
class BACKTOZARAYSK_API UBackpackItemData : public UEquippableItemData
{
    GENERATED_BODY()
    
public:
    UBackpackItemData();
};

