#pragma once

#include "CoreMinimal.h"
#include "EquipmentBase.h"
#include "HelmetBase.generated.h"

class UTexture2D;

/**
 * Базовый головной убор (helmet_base)
 * - Экипируется в слот "Шлем" (Helmet)
 * - По умолчанию 1x1 в гриде инвентаря (можно изменить в редакторе через параметры актора)
 * - Визуал: небольшой параллелепипед (куб с масштабом)
 */
UCLASS(Blueprintable)
class BACKTOZARAYSK_API AHelmetBase : public AEquipmentBase
{
    GENERATED_BODY()

public:
    AHelmetBase();

protected:
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;

public:
    // Размер в гриде инвентаря (можно менять для этого pickup в редакторе)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Helmet|Inventory", meta=(ClampMin="1"))
    FIntPoint InventoryGridSize = FIntPoint(1, 1);

    // Иконка в гриде инвентаря (для BP-наследников). Если не выбрана — будет белая заливка (WhiteSquare) нужного размера.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Helmet|Inventory", meta=(DisplayName="Inventory Icon Texture"))
    TObjectPtr<UTexture2D> InventoryIconTexture = nullptr;

    // Масштаб визуала в мире
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Helmet|Visual")
    FVector WorldVisualScale = FVector(1.0f, 1.0f, 1.0f);

private:
    void ApplyInventoryIconOverride();
};

