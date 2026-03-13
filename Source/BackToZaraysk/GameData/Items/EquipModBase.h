#pragma once

#include "CoreMinimal.h"
#include "ItemBase.h"
#include "EquipModBase.generated.h"

class UTexture2D;

/**
 * EquipModBase — базовый модуль снаряжения (НЕ экипировка), наследник ItemBase.
 * Может иметь внутреннее хранилище, открываемое отдельным окном в UI.
 */
UCLASS(Blueprintable)
class BACKTOZARAYSK_API AEquipModBase : public AItemBase
{
	GENERATED_BODY()

public:
	AEquipModBase();

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
	// Есть ли внутренний грид‑контейнер
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EquipMod|Storage", meta=(DisplayName="isHasGrid"))
	bool bIsHasGrid = false;

	// Кол-во мини‑гридов хранилища (как у разгрузки)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EquipMod|Storage", meta=(ClampMin="1", UIMin="1", EditCondition="bIsHasGrid", EditConditionHides))
	int32 StorageGridCount = 1;

	// Размеры мини‑гридов (в клетках), индекс 0..StorageGridCount-1
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EquipMod|Storage", meta=(EditCondition="bIsHasGrid", EditConditionHides))
	TArray<FIntPoint> StorageGridSizes;

	// Смещения мини‑гридов: Offset=(Col,Row) для табличной раскладки в UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EquipMod|Storage", meta=(EditCondition="bIsHasGrid", EditConditionHides))
	TArray<FIntPoint> StorageGridOffsets;

	// На будущее: сколько слотов модификаций занимает этот модуль (X/Y)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EquipMod|Slots", meta=(ClampMin="1", UIMin="1", DisplayName="modSlotSize"))
	FIntPoint ModSlotSize = FIntPoint(1, 1);

	/** Точка отсчёта грида привязки в пространстве меша (X,Y,Z). Этой точкой мод цепляется к сетке на броне. Редактируется кнопкой "Редактор грида привязки..." в Details. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EquipMod|Slots")
	FVector AttachmentGridOrigin = FVector::ZeroVector;

	/** Синхронизировать AttachmentGridOrigin в ItemInstance (вызывается из редактора грида после сохранения). */
	void SyncAttachmentGridOriginToItemInstance();

private:
	void EnsureStorageGrids();
	void ApplyEditorOverridesToItemInstance();
};

