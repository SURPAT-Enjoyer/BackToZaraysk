#pragma once

#include "CoreMinimal.h"
#include "EquipmentBase.h"
#include "TacticalVest.generated.h"

/**
 * Тактический разгрузочный жилет
 * Размер в инвентаре: 3x3
 * Экипируется в слот "Разгрузка"
 */
UCLASS()
class BACKTOZARAYSK_API ATacticalVest : public AEquipmentBase
{
    GENERATED_BODY()
    
public:
    ATacticalVest();
    
protected:
    virtual void BeginPlay() override;
};




