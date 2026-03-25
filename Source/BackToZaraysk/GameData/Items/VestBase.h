#pragma once

#include "CoreMinimal.h"
#include "EquipmentBase.h"
#include "VestBase.generated.h"

class UTexture2D;

/**
 * Базовая разгрузка (VestBase)
 * - Экипируется в слот "Разгрузка" (Vest)
 * - Размер в инвентаре задаётся в BP-наследнике (X/Y)
 * - Хранилище задаётся как набор независимых мини‑гридов слева направо:
 *   Count + список размеров каждого грида (X/Y)
 * - Иконка в инвентаре может быть задана в BP, иначе будет белая заливка
 * - Кость/сокет аттача задаётся в BP (как в BackpackBase)
 */
UCLASS(Blueprintable)
class BACKTOZARAYSK_API AVestBase : public AEquipmentBase
{
	GENERATED_BODY()

public:
	AVestBase();

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
	// Размер предмета в гриде инвентаря (по умолчанию 1x1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vest|Inventory", meta=(ClampMin="1", UIMin="1"))
	int32 InventorySizeX = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vest|Inventory", meta=(ClampMin="1", UIMin="1"))
	int32 InventorySizeY = 1;

	// Количество мини‑гридов хранилища (по умолчанию 1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vest|Storage", meta=(ClampMin="1", UIMin="1"))
	int32 StorageGridCount = 1;

	// Размеры мини‑гридов (слева направо). Кол-во элементов поддерживается равным StorageGridCount.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vest|Storage")
	TArray<FIntPoint> StorageGridSizes;

	// Смещения мини‑гридов (в клетках 60x60) относительно базовой точки отрисовки жилета.
	// Кол-во элементов поддерживается равным StorageGridCount.
	// Пример: (0,0), (2,0), (4,0) — три грида в ряд с шагом 2 клетки.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vest|Storage")
	TArray<FIntPoint> StorageGridOffsets;

	// Кость/сокет, к которой разгрузка будет приаттачена при экипировке (по умолчанию spine_03)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vest|Attachment", meta=(DisplayName="Attach Bone/Socket"))
	FName AttachBoneName = FName(TEXT("spine_03"));

	// Иконка в гриде инвентаря (для BP-наследников). Если не выбрана — будет белая заливка нужного размера.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vest|Inventory", meta=(DisplayName="Inventory Icon Texture"))
	TObjectPtr<UTexture2D> InventoryIconTexture = nullptr;

	// Масштаб визуала в мире (по умолчанию 1x1x1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vest|Visual")
	FVector WorldVisualScale = FVector(1.0f, 1.0f, 1.0f);

private:
	void EnsureStorageGridSizes();
	void ApplyEditorOverridesToItemInstance();
	void ApplyInventoryIconOverride();
};

