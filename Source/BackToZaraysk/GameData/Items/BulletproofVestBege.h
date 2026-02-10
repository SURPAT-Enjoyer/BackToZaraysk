#pragma once

#include "CoreMinimal.h"
#include "ArmorBase.h"
#include "BulletproofVestBege.generated.h"

/**
 * Бронежилет на базе ArmorBase, но с моделью SK_Bulletproof_Bege
 * Все остальные свойства идентичны ArmorBase (3x3, слот Armor, без хранилищ).
 */
UCLASS(Blueprintable)
class BACKTOZARAYSK_API ABulletproofVestBege : public AArmorBase
{
    GENERATED_BODY()

public:
    ABulletproofVestBege();

protected:
    virtual void BeginPlay() override;
};


