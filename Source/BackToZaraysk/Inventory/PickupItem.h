#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupItem.generated.h"

class UStaticMeshComponent;
class UInventoryItemData;

UCLASS()
class BACKTOZARAYSK_API APickupItem : public AActor
{
	GENERATED_BODY()
public:
	APickupItem();

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item")
	TSubclassOf<UInventoryItemData> ItemClass;
};







