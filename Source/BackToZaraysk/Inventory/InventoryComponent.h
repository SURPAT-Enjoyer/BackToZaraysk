#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryItemData.h"
#include "InventoryComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BACKTOZARAYSK_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UInventoryComponent();

	// Простой рюкзак: список предметов
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	TArray<TObjectPtr<UInventoryItemData>> BackpackItems;

	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool AddToBackpack(UInventoryItemData* Item);

	UFUNCTION(BlueprintCallable, Category="Inventory")
	UInventoryItemData* RemoveLastFromBackpack();

	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool RemoveSpecificFromBackpack(UInventoryItemData* Item);
};







