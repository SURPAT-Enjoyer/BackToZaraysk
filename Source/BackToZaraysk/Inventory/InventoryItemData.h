#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InventoryItemData.generated.h"

UENUM(BlueprintType)
enum class EInventoryItemType : uint8
{
	Generic UMETA(DisplayName = "Generic")
};

UCLASS(BlueprintType)
class BACKTOZARAYSK_API UInventoryItemData : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item")
	EInventoryItemType ItemType = EInventoryItemType::Generic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item")
	FText DisplayName = FText::FromString(TEXT("Куб"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item")
	UTexture2D* Icon = nullptr; // по умолчанию установим белый квадрат из движка, если пусто

    // Размер в клетках инвентаря (по умолчанию 1x1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item|Size")
    int32 SizeInCellsX = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item|Size")
    int32 SizeInCellsY = 1;

    // Можно ли вращать предмет (R)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item|Size")
    bool bRotatable = true;
};

UCLASS(BlueprintType)
class BACKTOZARAYSK_API UInventoryItemData_2x1 : public UInventoryItemData
{
    GENERATED_BODY()
public:
    UInventoryItemData_2x1()
    {
        SizeInCellsX = 2;
        SizeInCellsY = 1;
        DisplayName = FText::FromString(TEXT("Параллелепипед"));
    }
};

UCLASS(BlueprintType)
class BACKTOZARAYSK_API UInventoryItemData_2x2 : public UInventoryItemData
{
    GENERATED_BODY()
public:
    UInventoryItemData_2x2()
    {
        SizeInCellsX = 2;
        SizeInCellsY = 2;
        DisplayName = FText::FromString(TEXT("Большой куб"));
    }
};

