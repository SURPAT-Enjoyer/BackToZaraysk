#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InventoryItemData.generated.h"

UENUM(BlueprintType)
enum class EInventoryItemType : uint8
{
	Generic UMETA(DisplayName = "Generic")
};

UCLASS(BlueprintType, Blueprintable)
class BACKTOZARAYSK_API UInventoryItemData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UInventoryItemData();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item")
	EInventoryItemType ItemType = EInventoryItemType::Generic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item")
	FText DisplayName = FText::FromString(TEXT("Куб"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item")
	UTexture2D* Icon = nullptr; // по умолчанию установим белый квадрат из движка, если пусто

	// Визуал, который должен использоваться при спавне/выбросе предмета в мир
	// (например, чтобы у BP-наследников ItemBase/EquipModBase при выбросе сохранялся их меш).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item|Visual")
	TObjectPtr<UObject> WorldMesh = nullptr;

	// Вес предмета в килограммах (может быть отрицательным). Пока ни на что не влияет.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item|Stats", meta=(Units="kg", ForceUnits="kg", Delta="0.01"))
	float WeightKg = 1.00f;

    // Размер в клетках инвентаря (по умолчанию 1x1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item|Size")
    int32 SizeInCellsX = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item|Size")
    int32 SizeInCellsY = 1;

    // Можно ли вращать предмет (R)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item|Size")
    bool bRotatable = true;
};

// Базовые данные для предметов "в мире", которые не являются экипировкой.
// Нужен отдельный класс, чтобы корректно маппить на AItemBase при выбрасывании.
UCLASS(BlueprintType)
class BACKTOZARAYSK_API UItemBaseItemData : public UInventoryItemData
{
	GENERATED_BODY()
public:
	UItemBaseItemData()
	{
		DisplayName = FText::FromString(TEXT("Предмет"));
		// Остальные значения по умолчанию берутся из UInventoryItemData
	}
};

// Данные модуля снаряжения (НЕ экипировка), который может иметь собственное хранилище.
UCLASS(BlueprintType, Blueprintable)
class BACKTOZARAYSK_API UEquipModBaseItemData : public UItemBaseItemData
{
	GENERATED_BODY()
public:
	UEquipModBaseItemData()
	{
		DisplayName = FText::FromString(TEXT("Модуль"));
		bHasAdditionalStorage = false;
		AdditionalGridSize = FIntPoint(1, 1);
		AdditionalGrids = { FIntPoint(1, 1) };
		AdditionalGridOffsets = { FIntPoint(0, 0) };
		ModSlotSize = FIntPoint(1, 1);
	}

	// Есть ли внутреннее хранилище (если false — UI/контекстное меню не показывает "Открыть")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mod|Storage")
	bool bHasAdditionalStorage = false;

	// Legacy/общий размер (bounding) — может использоваться как фолбэк
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mod|Storage", meta=(EditCondition="bHasAdditionalStorage"))
	FIntPoint AdditionalGridSize = FIntPoint(0, 0);

	// Мульти‑гриды (как у разгрузки): размеры мини‑гридов в клетках
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mod|Storage", meta=(EditCondition="bHasAdditionalStorage"))
	TArray<FIntPoint> AdditionalGrids;

	// Смещения мини‑гридов: Offset=(Col,Row) для табличной раскладки в UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mod|Storage", meta=(EditCondition="bHasAdditionalStorage"))
	TArray<FIntPoint> AdditionalGridOffsets;

	// Персистентное содержимое (живёт вместе с самим ItemData)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mod|Storage", meta=(EditCondition="bHasAdditionalStorage"))
	TArray<TObjectPtr<class UInventoryItemData>> PersistentStorage;

	// Позиции в мини‑гридах (локальные CellX/CellY)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mod|Storage", meta=(EditCondition="bHasAdditionalStorage"))
	TMap<TObjectPtr<class UInventoryItemData>, FIntPoint> StoredCellByItem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mod|Storage", meta=(EditCondition="bHasAdditionalStorage"))
	TMap<TObjectPtr<class UInventoryItemData>, FIntPoint> PersistentCellByItem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mod|Storage", meta=(EditCondition="bHasAdditionalStorage"))
	TMap<TObjectPtr<class UInventoryItemData>, bool> StoredRotByItem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mod|Storage", meta=(EditCondition="bHasAdditionalStorage"))
	TMap<TObjectPtr<class UInventoryItemData>, bool> PersistentRotByItem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mod|Storage", meta=(EditCondition="bHasAdditionalStorage"))
	TMap<TObjectPtr<class UInventoryItemData>, int32> StoredGridByItem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mod|Storage", meta=(EditCondition="bHasAdditionalStorage"))
	TMap<TObjectPtr<class UInventoryItemData>, int32> PersistentGridByItem;

	// На будущее: сколько слотов под модификации занимает этот модуль (прямоугольник X/Y)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mod|Slots", meta=(ClampMin="1", UIMin="1"))
	FIntPoint ModSlotSize = FIntPoint(1, 1);

	/** Точка отсчёта грида привязки в пространстве меша (X,Y,Z). Этой точкой мод цепляется к сетке на броне. Редактируется в редакторе грида мода. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mod|Slots")
	FVector AttachmentGridOrigin = FVector::ZeroVector;
};

UCLASS(BlueprintType)
class BACKTOZARAYSK_API UInventoryItemData_2x1 : public UInventoryItemData
{
    GENERATED_BODY()
public:
    UInventoryItemData_2x1()
    {
        SizeInCellsX = 2;
        SizeInCellsY = 1;
        DisplayName = FText::FromString(TEXT("Параллелепипед"));
    }
};

UCLASS(BlueprintType)
class BACKTOZARAYSK_API UInventoryItemData_2x2 : public UInventoryItemData
{
    GENERATED_BODY()
public:
    UInventoryItemData_2x2()
    {
        SizeInCellsX = 2;
        SizeInCellsY = 2;
        DisplayName = FText::FromString(TEXT("Большой куб"));
    }
};

