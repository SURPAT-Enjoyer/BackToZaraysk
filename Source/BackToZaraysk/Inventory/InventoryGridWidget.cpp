#include "InventoryGridWidget.h"
#include "Rendering/DrawElements.h"

UInventoryGridWidget::UInventoryGridWidget(const FObjectInitializer& OI)
	: Super(OI)
{
}

void UInventoryGridWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	Cells.SetNum(GridWidth * GridHeight);
}

int32 UInventoryGridWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const FVector2D Size = AllottedGeometry.GetLocalSize();
	const float CellW = Size.X / FMath::Max(1, GridWidth);
	const float CellH = Size.Y / FMath::Max(1, GridHeight);

    // Заливка фона ячеек
	for (int32 y = 0; y < GridHeight; ++y)
	{
		for (int32 x = 0; x < GridWidth; ++x)
		{
			const FVector2D P0(x * CellW, y * CellH);
			const FVector2D P1((x + 1) * CellW, (y + 1) * CellH);
            const FPaintGeometry Geo = AllottedGeometry.ToPaintGeometry(P0, P1 - P0);
            FSlateDrawElement::MakeBox(OutDrawElements, LayerId, Geo, FCoreStyle::Get().GetBrush("WhiteBrush"), ESlateDrawEffect::None, CellColor);
		}
	}

	// Линии сетки
	++LayerId;
	TArray<FVector2D> Points;
	Points.SetNum(2);
    for (int32 x = 0; x <= GridWidth; ++x)
	{
		Points[0] = FVector2D(x * CellW, 0.f);
		Points[1] = FVector2D(x * CellW, Size.Y);
        FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), Points, ESlateDrawEffect::None, LineColor, true, LineThickness);
	}
	for (int32 y = 0; y <= GridHeight; ++y)
	{
		Points[0] = FVector2D(0.f, y * CellH);
		Points[1] = FVector2D(Size.X, y * CellH);
        FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), Points, ESlateDrawEffect::None, LineColor, true, LineThickness);
	}

	return LayerId + 1;
}


