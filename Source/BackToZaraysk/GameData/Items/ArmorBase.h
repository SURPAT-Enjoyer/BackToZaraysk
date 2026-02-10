#pragma once

#include "CoreMinimal.h"
#include "EquipmentBase.h"
#include "ArmorBase.generated.h"

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
};


