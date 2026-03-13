#pragma once

#include "CoreMinimal.h"
#include "HelmetBase.h"
#include "Cap01.generated.h"

/**
 * Головной убор Cap01 (кепка) на базе HelmetBase
 * Визуал: SM_Cap_Bege
 */
UCLASS(Blueprintable)
class BACKTOZARAYSK_API ACap01 : public AHelmetBase
{
    GENERATED_BODY()

public:
    ACap01();

protected:
    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void BeginPlay() override;
};

