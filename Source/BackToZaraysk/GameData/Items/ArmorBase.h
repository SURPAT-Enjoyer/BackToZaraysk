#pragma once

#include "CoreMinimal.h"
#include "EquipmentBase.h"
#include "ArmorBase.generated.h"

class UTexture2D;

/**
 * Базовый бронежилет (armor_base)
 * Размер в инвентаре: 3x3
 * Экипируется в слот "Бронежилет"
 * НЕ имеет дополнительных хранилищ
 */
UCLASS(Blueprintable)
class BACKTOZARAYSK_API AArmorBase : public AEquipmentBase
{
    GENERATED_BODY()

public:
    AArmorBase();

protected:
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;

public:
    // Иконка в гриде инвентаря (для BP-наследников). Если не выбрана — будет белая заливка (WhiteSquare) нужного размера.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Armor|Inventory", meta=(DisplayName="Inventory Icon Texture"))
    TObjectPtr<UTexture2D> InventoryIconTexture = nullptr;

    // Размер предмета в гриде инвентаря (для BP-наследников). Можно задать произвольный прямоугольник X/Y.
    // По умолчанию 3x3, как у базового бронежилета.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Armor|Inventory", meta=(ClampMin="1", UIMin="1", DisplayName="Inventory Size X"))
    int32 InventorySizeX = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Armor|Inventory", meta=(ClampMin="1", UIMin="1", DisplayName="Inventory Size Y"))
    int32 InventorySizeY = 3;

    // Кость/сокет скелета персонажа, к которой прикрепляется бронежилет при экипировке (по умолчанию spine_02).
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Armor|Attachment", meta=(DisplayName="Attach Bone/Socket"))
    FName AttachBoneName = FName(TEXT("spine_02"));

private:

private:
    void ApplyInventoryIconOverride();

    // Применить настройки из Blueprint (InventorySizeX/Y, модифицируемость и т.п.) в ItemInstance,
    // по аналогии с AVestBase::ApplyEditorOverridesToItemInstance.
    void ApplyEditorOverridesToItemInstance();
};


