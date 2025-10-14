#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupBase.generated.h"

UCLASS()
class BACKTOZARAYSK_API APickupBase : public AActor
{
    GENERATED_BODY()
public:
    APickupBase();

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<class UStaticMeshComponent> Mesh;

    UPROPERTY(EditAnywhere, Category="Pickup")
    TSubclassOf<class UInventoryItemData> ItemClass;

    // Если установлен, этот экземпляр данных предмета будет использован при подборе
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pickup")
    TObjectPtr<class UInventoryItemData> ItemInstance = nullptr;

    UPROPERTY(EditAnywhere, Category="Pickup|Physics")
    bool bEnablePhysics = true;

    // Применение визуала в соответствии с ItemInstance (по умолчанию no-op)
    virtual void ApplyItemInstanceVisuals();
};







