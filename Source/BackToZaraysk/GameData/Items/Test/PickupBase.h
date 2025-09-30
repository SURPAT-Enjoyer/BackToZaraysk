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

    UPROPERTY(EditAnywhere, Category="Pickup|Physics")
    bool bEnablePhysics = true;
};







