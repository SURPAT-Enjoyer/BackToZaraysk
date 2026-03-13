#pragma once

#include "CoreMinimal.h"
#include "InventoryItemData.h"
#include "EquipmentSlotType.h"
#include "BackToZaraysk/GameData/Items/ArmorModSideGrid.h"
#include "EquippableItemData.generated.h"

class AActor;

/** Один установленный на броню модуль: ссылка на предмет-мод и позиция на сетке стороны. */
USTRUCT(BlueprintType)
struct BACKTOZARAYSK_API FInstalledArmorMod
{
	GENERATED_BODY()

	/** Данные предмета-мода (должен быть UEquipModBaseItemData или наследник). Содержимое хранилища мода хранится в нём. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Armor|Mods")
	TObjectPtr<UInventoryItemData> ModItemData = nullptr;

	/** Сторона: 0=Front, 1=Back, 2=Left, 3=Right. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Armor|Mods", meta = (ClampMin = "0", ClampMax = "3"))
	int32 SideIndex = 0;

	/** Ячейка сетки по X (левый нижний угол модуля). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Armor|Mods", meta = (ClampMin = "0"))
	int32 CellX = 0;

	/** Ячейка сетки по Y (левый нижний угол модуля). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Armor|Mods", meta = (ClampMin = "0"))
	int32 CellY = 0;
};

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

    // Можно ли модифицировать этот предмет (используется пока только для Armor)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Equipment|Mods")
    bool bIsModdable = false;

    // Сетки для установки модулей по сторонам (заполняется из AArmorBase при pickup/конструкторе)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Equipment|Mods")
    TArray<FArmorModSideGrid> ModSideGrids;

    /** Установленные на эту броню модули (позиция на сетке + ссылка на ModItemData с содержимым хранилища). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Equipment|Mods")
    TArray<FInstalledArmorMod> InstalledMods;

    /** Занятые ячейки на стороне SideIndex (пары CellX,CellY). Используется для проверки при установке мода. */
    UFUNCTION(BlueprintCallable, Category = "Equipment|Mods")
    void GetOccupiedCellsForSide(int32 SideIndex, TSet<FIntPoint>& OutOccupied) const;

    /** Можно ли установить мод с размером Footprint в ячейку (CellX, CellY) на стороне SideIndex. */
    UFUNCTION(BlueprintCallable, Category = "Equipment|Mods")
    bool CanInstallModAt(int32 SideIndex, int32 CellX, int32 CellY, FIntPoint Footprint) const;

    /** Размер мода в ячейках сетки (для UEquipModBaseItemData — ModSlotSize, иначе 1x1). */
    UFUNCTION(BlueprintCallable, Category = "Equipment|Mods")
    static FIntPoint GetModFootprint(UInventoryItemData* ModItemData);

    /** Заполняет список гридов хранилищ от установленных модов (для отображения в UI брони). Содержимое берётся из ModItemData. */
    UFUNCTION(BlueprintCallable, Category = "Equipment|Mods")
    void GetModStorageGrids(TArray<FIntPoint>& OutGridSizes, TArray<UInventoryItemData*>& OutModItemData) const;

    /** Установить мод на бронь в ячейку (SideIndex, CellX, CellY). Возвращает true при успехе. Вызывающий должен убрать мод из инвентаря. */
    UFUNCTION(BlueprintCallable, Category = "Equipment|Mods")
    bool InstallMod(UInventoryItemData* ModItemData, int32 SideIndex, int32 CellX, int32 CellY);

    /** Снять мод по индексу в InstalledMods. Возвращает снятый ModItemData (положить в инвентарь или выбросить в мир). */
    UFUNCTION(BlueprintCallable, Category = "Equipment|Mods")
    UInventoryItemData* UninstallMod(int32 InstalledModIndex);

    /** Найти индекс установленного мода в ячейке (SideIndex, CellX, CellY). Возвращает -1 если пусто. */
    UFUNCTION(BlueprintCallable, Category = "Equipment|Mods")
    int32 GetInstalledModIndexAt(int32 SideIndex, int32 CellX, int32 CellY) const;

    // Меш для отображения на персонаже (может быть SkeletalMesh или StaticMesh)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Equipment|Visual")
    UObject* EquippedMesh = nullptr;

    // Класс pickup-актора, который будет использоваться при выбрасывании предмета в мир.
    // Если не задан, используется класс по умолчанию в зависимости от EquipmentSlot.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Equipment|Visual")
    TSubclassOf<AActor> PickupActorClass;
    
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

    // Мульти-гриды для хранилища (например, разгрузочный жилет как набор независимых мини-гридов слева направо).
    // Если массив непустой — используем его как источник правды для размеров хранилища.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Equipment|Storage", meta=(EditCondition="bHasAdditionalStorage"))
    TArray<FIntPoint> AdditionalGrids;

    // Смещения (в клетках) для каждого мини‑грида AdditionalGrids относительно базовой точки в UI.
    // Индекс соответствует AdditionalGrids[i]. Если пусто или длина не совпадает — UI использует раскладку по умолчанию.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Equipment|Storage", meta=(EditCondition="bHasAdditionalStorage"))
    TArray<FIntPoint> AdditionalGridOffsets;
    
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

    // Чтобы подхватывать новую иконку даже у ранее сохранённых/заспавненных инстансов (где Icon мог остаться старым),
    // после загрузки принудительно корректируем Icon.
    virtual void PostLoad() override;
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
 * Данные базового разгрузочного жилета (VestBase) — размер и конфигурация хранилища переопределяются в pickup-акторе (AVestBase)
 */
UCLASS(BlueprintType, Blueprintable, meta=(DisplayName="Vest Base Item Data"))
class BACKTOZARAYSK_API UVestBaseItemData : public UEquippableItemData
{
    GENERATED_BODY()

public:
    UVestBaseItemData();
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
 * Данные базового рюкзака (BackpackBase) — размер и размер хранилища переопределяются в pickup-акторе (ABackpackBase)
 */
UCLASS(BlueprintType, Blueprintable, meta=(DisplayName="Backpack Base Item Data"))
class BACKTOZARAYSK_API UBackpackBaseItemData : public UEquippableItemData
{
    GENERATED_BODY()

public:
    UBackpackBaseItemData();
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
