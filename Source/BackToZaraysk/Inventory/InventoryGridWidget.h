#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryGridWidget.generated.h"

USTRUCT(BlueprintType)
struct FInventoryGridCell
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOccupied = false;
};

UCLASS()
class BACKTOZARAYSK_API UInventoryGridWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UInventoryGridWidget(const FObjectInitializer& OI);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid")
	int32 GridWidth = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid")
	int32 GridHeight = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid")
    FLinearColor CellColor = FLinearColor(0.f, 0.f, 0.f, 0.2f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid")
    FLinearColor LineColor = FLinearColor(0.f, 1.f, 0.f, 0.8f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid")
    float LineThickness = 2.0f;

protected:
	virtual void NativeOnInitialized() override;
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

private:
	TArray<FInventoryGridCell> Cells;
};


