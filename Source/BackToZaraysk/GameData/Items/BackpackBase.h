#pragma once

#include "CoreMinimal.h"
#include "EquipmentBase.h"
#include "BackpackBase.generated.h"

class UTexture2D;

/**
 * Базовый рюкзак (BackpackBase)
 * - Экипируется в слот "Рюкзак" (Backpack)
 * - Размер в инвентаре задаётся в BP-наследнике (X/Y)
 * - Размер грида-хранилища задаётся в BP-наследнике (X/Y)
 * - Иконка в инвентаре может быть задана в BP, иначе будет белая заливка
 */
UCLASS(Blueprintable)
class BACKTOZARAYSK_API ABackpackBase : public AEquipmentBase
{
	GENERATED_BODY()

public:
	ABackpackBase();

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
	// Размер предмета в гриде инвентаря (по умолчанию 1x1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Backpack|Inventory", meta=(ClampMin="1", UIMin="1"))
	int32 InventorySizeX = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Backpack|Inventory", meta=(ClampMin="1", UIMin="1"))
	int32 InventorySizeY = 1;

	// Размер внутреннего хранилища рюкзака (legacy single-grid, kept for compatibility)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Backpack|Storage", meta=(ClampMin="1", UIMin="1"))
	int32 StorageGridSizeX = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Backpack|Storage", meta=(ClampMin="1", UIMin="1"))
	int32 StorageGridSizeY = 1;

	// Новый формат: произвольное количество мини‑гридов хранилища (как у VestBase)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Backpack|Storage", meta=(ClampMin="1", UIMin="1"))
	int32 StorageGridCount = 1;

	// Размеры мини‑гридов (в клетках), индекс 0..StorageGridCount-1
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Backpack|Storage")
	TArray<FIntPoint> StorageGridSizes;

	// Смещения мини‑гридов (в клетках 60x60) относительно базовой точки отрисовки рюкзака
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Backpack|Storage")
	TArray<FIntPoint> StorageGridOffsets;

	// Кость/сокет, к которой рюкзак будет приаттачен при экипировке (по умолчанию spine_03)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Backpack|Attachment", meta=(DisplayName="Attach Bone/Socket"))
	FName AttachBoneName = FName(TEXT("spine_03"));

	// Иконка в гриде инвентаря (для BP-наследников). Если не выбрана — будет белая заливка (WhiteSquare) нужного размера.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Backpack|Inventory", meta=(DisplayName="Inventory Icon Texture"))
	TObjectPtr<UTexture2D> InventoryIconTexture = nullptr;

	// Масштаб визуала в мире (по умолчанию 1x1x1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Backpack|Visual")
	FVector WorldVisualScale = FVector(1.0f, 1.0f, 1.0f);

private:
	void EnsureStorageGrids();
	void ApplyEditorOverridesToItemInstance();
	void ApplyInventoryIconOverride();
};

