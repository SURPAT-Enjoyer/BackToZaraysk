#pragma once

#include "CoreMinimal.h"
#include "Test/PickupBase.h"
#include "ItemBase.generated.h"

class UTexture2D;
class USkeletalMeshComponent;

/**
 * Базовый предмет (НЕ экипировка), который существует в мире как pickup.
 * - Размер в инвентаре задаётся в Details (X/Y), по умолчанию 1x1
 * - Иконка задаётся в Details, иначе UI рисует белую заливку (nullptr)
 * - Вес (кг) задаётся в Details, может быть отрицательным (пока ни на что не влияет)
 * - Визуал по умолчанию: маленький куб 0.1 (если нет мешей)
 * - Масштаб по умолчанию: 1x1x1 (WorldVisualScale = 1)
 */
UCLASS(Blueprintable)
class BACKTOZARAYSK_API AItemBase : public APickupBase
{
	GENERATED_BODY()

public:
	AItemBase();
	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	// Поддержка визуала в зависимости от назначенных мешей/ItemInstance
	virtual void ApplyItemInstanceVisuals() override;

public:
	/** Скелетный меш (опционально). Поддерживается и StaticMesh (Mesh), и SkeletalMesh — визуал берётся из ItemInstance->WorldMesh или из назначенного в Blueprint меша. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USkeletalMeshComponent> SkeletalMesh;

	// Размер предмета в гриде инвентаря (по умолчанию 1x1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item|Inventory", meta=(ClampMin="1", UIMin="1"))
	int32 InventorySizeX = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item|Inventory", meta=(ClampMin="1", UIMin="1"))
	int32 InventorySizeY = 1;

	// Иконка в гриде инвентаря. Если не выбрана — будет белая заливка (WhiteSquare) нужного размера.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item|Inventory", meta=(DisplayName="Inventory Icon Texture"))
	TObjectPtr<UTexture2D> InventoryIconTexture = nullptr;

	// Вес в килограммах. Может быть отрицательным. Пока ни на что не влияет.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item|Stats", meta=(Units="kg", ForceUnits="kg", Delta="0.01"))
	float WeightKg = 1.00f;

	// Плавучесть: -1 тонет, 0 нейтрально, 1 всплывает (пока нигде не применяется).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item|Water", meta=(ClampMin="-1.0", ClampMax="1.0", UIMin="-1.0", UIMax="1.0", Delta="0.1"))
	float Buoyancy = 0.0f;

	// Сила плавучести для физики (множитель, см/с^2 относительно g). 1.0 ~ "как вода".
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item|Water", meta=(ClampMin="0.0", UIMin="0.0"))
	float BuoyancyStrength = 1.0f;

	// Множитель масштаба визуала (для назначенных мешей). По умолчанию 1x1x1.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item|Visual")
	FVector WorldVisualScale = FVector(1.0f, 1.0f, 1.0f);

private:
	void ApplyEditorOverridesToItemInstance();
	void ApplyInventoryIconOverride();
	void ApplyDefaultVisualIfNeeded();
	void SyncVisibleMeshAndScale();
};

