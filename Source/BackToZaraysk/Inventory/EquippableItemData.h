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

    // Флаг поворота предмета внутри дополнительного хранилища (runtime)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Equipment|Storage")
    TMap<TObjectPtr<class UInventoryItemData>, bool> StoredRotByItem;

    // Персистентная копия флага поворота
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Equipment|Storage")
    TMap<TObjectPtr<class UInventoryItemData>, bool> PersistentRotByItem;

    // Для многосекционных гридов (например, жилет из 6 мини‑гридов): в каком гриде лежит предмет
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Equipment|Storage")
    TMap<TObjectPtr<class UInventoryItemData>, int32> StoredGridByItem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Equipment|Storage")
    TMap<TObjectPtr<class UInventoryItemData>, int32> PersistentGridByItem;
};

/**
 * Данные базового шлема (по умолчанию 1x1, слот: Шлем)
 */
UCLASS(BlueprintType, Blueprintable, meta=(DisplayName="Helmet Base Item Data"))
class BACKTOZARAYSK_API UHelmetBaseItemData : public UEquippableItemData
{
    GENERATED_BODY()

public:
    UHelmetBaseItemData();
};

/**
 * Данные кепки Cap01 (на базе HelmetBase)
 */
UCLASS(BlueprintType, Blueprintable, meta=(DisplayName="Cap01 Item Data"))
class BACKTOZARAYSK_API UCap01ItemData : public UHelmetBaseItemData
{
    GENERATED_BODY()

public:
    UCap01ItemData();
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

/**
 * Данные бронежилета 3x3 (без доп. хранилищ)
 */
UCLASS(BlueprintType, Blueprintable, meta=(DisplayName="Armor Base Item Data"))
class BACKTOZARAYSK_API UArmorBaseItemData : public UEquippableItemData
{
    GENERATED_BODY()

public:
    UArmorBaseItemData();
};

/**
 * Бронежилет (bege) — на базе ArmorBase, но с моделью SK_Bulletproof_Bege
 * Размер: 3x3, слот: Бронежилет, без доп. хранилищ
 */
UCLASS(BlueprintType, Blueprintable, meta=(DisplayName="Bulletproof Vest Bege Item Data"))
class BACKTOZARAYSK_API UBulletproofVestBegeItemData : public UEquippableItemData
{
    GENERATED_BODY()

public:
    UBulletproofVestBegeItemData();
};

