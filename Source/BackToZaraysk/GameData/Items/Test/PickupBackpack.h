#pragma once

#include "CoreMinimal.h"
#include "PickupBase.h"
#include "PickupBackpack.generated.h"

UCLASS()
class BACKTOZARAYSK_API APickupBackpack : public APickupBase
{
	GENERATED_BODY()

public:
	APickupBackpack();
	
	// Override the pickup behavior to auto-equip if no backpack is currently worn
	virtual bool CanBePickedUp(class UInventoryComponent* InventoryComponent) const;
	virtual void OnPickedUp(class UInventoryComponent* InventoryComponent);
};
