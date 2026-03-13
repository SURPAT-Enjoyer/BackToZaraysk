#include "InventoryWidget.h"
#include "InventoryItemData.h"
#include "EquippableItemData.h"
#include "EquipmentSlotType.h"
#include "BackToZaraysk/Characters/PlayerCharacter.h"
#include "BackToZaraysk/Inventory/InventoryComponent.h"
#include "BackToZaraysk/Inventory/EquippableItemData.h"
#include "BackToZaraysk/Characters/PlayerCharacter.h"
#include "BackToZaraysk/Inventory/InventoryItemWidget.h"
#include "BackToZaraysk/Components/EquipmentComponent.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/PanelWidget.h"
#include "Components/Image.h"
#include "Components/VerticalBox.h"
#include "InventoryGridWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Slate/SlateBrushAsset.h"
#include "Styling/SlateColor.h"
#include "Engine/Texture2D.h"
#include "Components/Border.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "BackToZaraysk/Characters/BTZBaseCharacter.h"
#include "BackToZaraysk/Characters/PlayerCharacter.h"
#include "BackToZaraysk/Inventory/InventoryItemWidget.h"
#include "BackToZaraysk/Inventory/InventoryComponent.h"
#include "BackToZaraysk/Inventory/EquippableItemData.h"
#include "BackToZaraysk/GameData/Items/ArmorModPreviewActor.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"

namespace
{
    inline uint32 HashIntPoint(const FIntPoint& P)
    {
        // Не полагаемся на GetTypeHash(FIntPoint) (в некоторых конфигурациях его может не быть)
        return HashCombineFast(::GetTypeHash(P.X), ::GetTypeHash(P.Y));
    }

    inline uint32 HashGridLayout(const TArray<FIntPoint>& Sizes, const TArray<FIntPoint>& Offsets)
    {
        uint32 H = ::GetTypeHash(Sizes.Num());
        for (int32 i = 0; i < Sizes.Num(); ++i)
        {
            H = HashCombineFast(H, HashIntPoint(Sizes[i]));
            const FIntPoint Off = Offsets.IsValidIndex(i) ? Offsets[i] : FIntPoint(0, 0);
            H = HashCombineFast(H, HashIntPoint(Off));
        }
        return H;
    }

    // Табличная раскладка: Offset=(Col,Row) задаёт номер столбца/строки.
    // Позиции считаются по max ширинам столбцов и max высотам строк + фиксированный GapPx.
    inline void BuildTableLayoutPx(const TArray<FIntPoint>& SizesCells,
        const TArray<FIntPoint>& OffsetsCR,
        int32 CellSizePx,
        int32 GapPx,
        TMap<int32, float>& OutColStartPx,
        TMap<int32, float>& OutRowStartPx)
    {
        OutColStartPx.Reset();
        OutRowStartPx.Reset();

        TMap<int32, int32> ColMaxW;
        TMap<int32, int32> RowMaxH;
        for (int32 i = 0; i < SizesCells.Num(); ++i)
        {
            const FIntPoint Sz = SizesCells[i];
            const FIntPoint Off = OffsetsCR.IsValidIndex(i) ? OffsetsCR[i] : FIntPoint(i, 0);
            const int32 Col = Off.X;
            const int32 Row = Off.Y;
            ColMaxW.FindOrAdd(Col) = FMath::Max(ColMaxW.FindOrAdd(Col), FMath::Max(1, Sz.X));
            RowMaxH.FindOrAdd(Row) = FMath::Max(RowMaxH.FindOrAdd(Row), FMath::Max(1, Sz.Y));
        }

        TArray<int32> Cols;
        ColMaxW.GetKeys(Cols);
        Cols.Sort();
        float XCursor = 0.f;
        for (const int32 Col : Cols)
        {
            OutColStartPx.Add(Col, XCursor);
            XCursor += (float)(ColMaxW[Col] * CellSizePx + GapPx);
        }

        TArray<int32> Rows;
        RowMaxH.GetKeys(Rows);
        Rows.Sort();
        float YCursor = 0.f;
        for (const int32 Row : Rows)
        {
            OutRowStartPx.Add(Row, YCursor);
            YCursor += (float)(RowMaxH[Row] * CellSizePx + GapPx);
        }
    }
	// Размер мини‑грида жилета по индексу. Если VestData->AdditionalGrids заполнен — используем его,
	// иначе фолбэк на старую конфигурацию 6 мини‑гридов: 1x1,1x1,1x2,1x2,1x1,1x1
	inline FIntPoint VestMiniGridSizeByIndex(const UEquippableItemData* VestData, int32 GridIdx)
	{
		if (VestData && VestData->AdditionalGrids.Num() > 0 && VestData->AdditionalGrids.IsValidIndex(GridIdx))
		{
			const FIntPoint P = VestData->AdditionalGrids[GridIdx];
			return FIntPoint(FMath::Max(1, P.X), FMath::Max(1, P.Y));
		}
		if (GridIdx == 2 || GridIdx == 3) return FIntPoint(1, 2);
		return FIntPoint(1, 1);
	}

	inline int32 VestGridIndexFromVestGridWidgetName(const FString& WidgetName)
	{
		// "VestGrid_жилет3" -> 2
		FString Name = WidgetName;
		Name.RemoveFromStart(TEXT("VestGrid_"));
		if (Name.RemoveFromStart(TEXT("жилет")))
		{
			const int32 Parsed = FCString::Atoi(*Name);
			if (Parsed > 0) return Parsed - 1;
		}
		return INDEX_NONE;
	}

	// Новый формат жилета: StoredGridByItem/PersistentGridByItem = индекс мини‑грида 0..5,
	// StoredCellByItem/PersistentCellByItem = локальная клетка внутри мини‑грида.
	// Поддерживаем миграцию со старого формата, где StoredCellByItem было "глобальным" (X=секция 0..5, Y=ряд 0..1).
	inline bool GetVestPlacement(const UEquippableItemData* VestData, const UInventoryItemData* Item, int32& OutGridIdx, FIntPoint& OutLocalCell)
	{
		if (!VestData || !Item) return false;

		const int32* GridPtr = VestData->PersistentGridByItem.Find(Item);
		if (!GridPtr) GridPtr = VestData->StoredGridByItem.Find(Item);

		const FIntPoint* CellPtr = VestData->PersistentCellByItem.Find(Item);
		if (!CellPtr) CellPtr = VestData->StoredCellByItem.Find(Item);

		if (!CellPtr && !GridPtr) return false;

		if (GridPtr)
		{
			OutGridIdx = *GridPtr;
			OutLocalCell = CellPtr ? *CellPtr : FIntPoint(0, 0);
			return true;
		}

		// Миграция: X трактуем как GridIdx, Y — как local Y, local X всегда 0
		OutGridIdx = CellPtr->X;
		OutLocalCell = FIntPoint(0, CellPtr->Y);
		return true;
	}

	inline bool GetVestRotation(const UEquippableItemData* VestData, const UInventoryItemData* Item, bool& bOutRotated)
	{
		bOutRotated = false;
		if (!VestData || !Item) return false;
		if (const bool* R = VestData->PersistentRotByItem.Find(Item)) { bOutRotated = *R; return true; }
		if (const bool* R2 = VestData->StoredRotByItem.Find(Item)) { bOutRotated = *R2; return true; }
		return false;
	}

	inline bool IsAreaFreeInVestMiniGrid(const UEquippableItemData* VestData,
		int32 GridIdx,
		int32 StartCellX,
		int32 StartCellY,
		int32 SizeX,
		int32 SizeY,
		const UInventoryItemData* IgnoredItem)
	{
		if (!VestData) return false;
		const int32 MaxGrids = (VestData->AdditionalGrids.Num() > 0) ? VestData->AdditionalGrids.Num() : 6;
		if (GridIdx < 0 || GridIdx >= MaxGrids) return false;

		const FIntPoint GridSize = VestMiniGridSizeByIndex(VestData, GridIdx);
		if (StartCellX < 0 || StartCellY < 0) return false;
		if (StartCellX + SizeX > GridSize.X || StartCellY + SizeY > GridSize.Y) return false;

		for (const TPair<TObjectPtr<UInventoryItemData>, FIntPoint>& Pair : VestData->StoredCellByItem)
		{
			UInventoryItemData* Other = Pair.Key;
			if (!Other || Other == IgnoredItem) continue;

			int32 OtherGridIdx = INDEX_NONE;
			FIntPoint OtherCell(0, 0);
			if (!GetVestPlacement(VestData, Other, OtherGridIdx, OtherCell)) continue;
			if (OtherGridIdx != GridIdx) continue;

			bool bOtherRot = false;
			GetVestRotation(VestData, Other, bOtherRot);
			const int32 OtherSizeX = bOtherRot ? FMath::Max(1, Other->SizeInCellsY) : FMath::Max(1, Other->SizeInCellsX);
			const int32 OtherSizeY = bOtherRot ? FMath::Max(1, Other->SizeInCellsX) : FMath::Max(1, Other->SizeInCellsY);

			const bool bOverlapX = !(StartCellX + SizeX <= OtherCell.X || OtherCell.X + OtherSizeX <= StartCellX);
			const bool bOverlapY = !(StartCellY + SizeY <= OtherCell.Y || OtherCell.Y + OtherSizeY <= StartCellY);
			if (bOverlapX && bOverlapY)
			{
				return false;
			}
		}
		return true;
	}

    inline int32 BackpackGridIndexFromAreaName(const FString& AreaName)
    {
        // "рюкзак3" -> 2
        FString Name = AreaName;
        if (Name.RemoveFromStart(TEXT("рюкзак")))
        {
            const int32 Parsed = FCString::Atoi(*Name);
            if (Parsed > 0) return Parsed - 1;
        }
        return 0;
    }

    inline int32 EquipModGridIndexFromAreaName(const FString& AreaName)
    {
        // "модуль3" -> 2
        FString Name = AreaName;
        if (Name.RemoveFromStart(TEXT("модуль")))
        {
            const int32 Parsed = FCString::Atoi(*Name);
            if (Parsed > 0) return Parsed - 1;
        }
        return 0;
    }

    inline FIntPoint EquipModMiniGridSizeByIndex(const UEquipModBaseItemData* ModData, int32 GridIdx)
    {
        if (ModData && ModData->AdditionalGrids.Num() > 0 && ModData->AdditionalGrids.IsValidIndex(GridIdx))
        {
            const FIntPoint P = ModData->AdditionalGrids[GridIdx];
            return FIntPoint(FMath::Max(1, P.X), FMath::Max(1, P.Y));
        }
        if (ModData)
        {
            return FIntPoint(FMath::Max(1, ModData->AdditionalGridSize.X), FMath::Max(1, ModData->AdditionalGridSize.Y));
        }
        return FIntPoint(1, 1);
    }

    inline bool GetEquipModPlacement(const UEquipModBaseItemData* ModData, const UInventoryItemData* Item, int32& OutGridIdx, FIntPoint& OutLocalCell)
    {
        if (!ModData || !Item) return false;
        const int32* GridPtr = ModData->PersistentGridByItem.Find(Item);
        if (!GridPtr) GridPtr = ModData->StoredGridByItem.Find(Item);

        const FIntPoint* CellPtr = ModData->PersistentCellByItem.Find(Item);
        if (!CellPtr) CellPtr = ModData->StoredCellByItem.Find(Item);

        if (!CellPtr && !GridPtr) return false;
        OutGridIdx = GridPtr ? *GridPtr : 0;
        OutLocalCell = CellPtr ? *CellPtr : FIntPoint(0, 0);
        return true;
    }

    inline bool GetEquipModRotation(const UEquipModBaseItemData* ModData, const UInventoryItemData* Item, bool& bOutRotated)
    {
        bOutRotated = false;
        if (!ModData || !Item) return false;
        if (const bool* R = ModData->PersistentRotByItem.Find(Item)) { bOutRotated = *R; return true; }
        if (const bool* R2 = ModData->StoredRotByItem.Find(Item)) { bOutRotated = *R2; return true; }
        return false;
    }

    inline bool IsAreaFreeInEquipModMiniGrid(const UEquipModBaseItemData* ModData,
        int32 GridIdx,
        int32 StartCellX,
        int32 StartCellY,
        int32 SizeX,
        int32 SizeY,
        const UInventoryItemData* IgnoredItem)
    {
        if (!ModData || !ModData->bHasAdditionalStorage) return false;
        const int32 MaxGrids = (ModData->AdditionalGrids.Num() > 0) ? ModData->AdditionalGrids.Num() : 1;
        if (GridIdx < 0 || GridIdx >= MaxGrids) return false;

        const FIntPoint GridSize = EquipModMiniGridSizeByIndex(ModData, GridIdx);
        if (StartCellX < 0 || StartCellY < 0) return false;
        if (StartCellX + SizeX > GridSize.X || StartCellY + SizeY > GridSize.Y) return false;

        // Проверяем пересечения только с теми предметами, которые реально лежат в хранилище мода
        // (PersistentStorage) — так мы не зависим от возможных "висячих" записей в StoredCellByItem.
        for (UInventoryItemData* Other : ModData->PersistentStorage)
        {
            if (!Other || Other == IgnoredItem) continue;

            int32 OtherGridIdx = 0;
            FIntPoint OtherCell(0, 0);
            if (!GetEquipModPlacement(ModData, Other, OtherGridIdx, OtherCell)) continue;
            if (OtherGridIdx != GridIdx) continue;

            bool bOtherRot = false;
            GetEquipModRotation(ModData, Other, bOtherRot);
            const int32 OtherSizeX = bOtherRot ? FMath::Max(1, Other->SizeInCellsY) : FMath::Max(1, Other->SizeInCellsX);
            const int32 OtherSizeY = bOtherRot ? FMath::Max(1, Other->SizeInCellsX) : FMath::Max(1, Other->SizeInCellsY);

            const bool bOverlapX = !(StartCellX + SizeX <= OtherCell.X || OtherCell.X + OtherSizeX <= StartCellX);
            const bool bOverlapY = !(StartCellY + SizeY <= OtherCell.Y || OtherCell.Y + OtherSizeY <= StartCellY);
            if (bOverlapX && bOverlapY) return false;
        }
        return true;
    }

    inline FIntPoint BackpackMiniGridSizeByIndex(const UEquippableItemData* BackpackData, int32 GridIdx)
    {
        if (BackpackData && BackpackData->AdditionalGrids.Num() > 0 && BackpackData->AdditionalGrids.IsValidIndex(GridIdx))
        {
            const FIntPoint P = BackpackData->AdditionalGrids[GridIdx];
            return FIntPoint(FMath::Max(1, P.X), FMath::Max(1, P.Y));
        }
        if (BackpackData)
        {
            return FIntPoint(FMath::Max(1, BackpackData->AdditionalGridSize.X), FMath::Max(1, BackpackData->AdditionalGridSize.Y));
        }
        return FIntPoint(1, 1);
    }

    // Новый формат рюкзака: StoredGridByItem/PersistentGridByItem = индекс мини‑грида,
    // StoredCellByItem/PersistentCellByItem = локальная клетка внутри мини‑грида.
    // Миграция со старого формата: если StoredGridByItem отсутствует, считаем что GridIdx=0, а StoredCellByItem — локальная клетка.
    inline bool GetBackpackPlacement(const UEquippableItemData* BackpackData, const UInventoryItemData* Item, int32& OutGridIdx, FIntPoint& OutLocalCell)
    {
        if (!BackpackData || !Item) return false;
        const int32* GridPtr = BackpackData->PersistentGridByItem.Find(Item);
        if (!GridPtr) GridPtr = BackpackData->StoredGridByItem.Find(Item);

        const FIntPoint* CellPtr = BackpackData->PersistentCellByItem.Find(Item);
        if (!CellPtr) CellPtr = BackpackData->StoredCellByItem.Find(Item);

        if (!CellPtr && !GridPtr) return false;
        OutGridIdx = GridPtr ? *GridPtr : 0;
        OutLocalCell = CellPtr ? *CellPtr : FIntPoint(0, 0);
        return true;
    }

    inline bool GetBackpackRotation(const UEquippableItemData* BackpackData, const UInventoryItemData* Item, bool& bOutRotated)
    {
        bOutRotated = false;
        if (!BackpackData || !Item) return false;
        if (const bool* R = BackpackData->PersistentRotByItem.Find(Item)) { bOutRotated = *R; return true; }
        if (const bool* R2 = BackpackData->StoredRotByItem.Find(Item)) { bOutRotated = *R2; return true; }
        return false;
    }

    inline bool IsAreaFreeInBackpackMiniGrid(const UEquippableItemData* BackpackData,
        int32 GridIdx,
        int32 StartCellX,
        int32 StartCellY,
        int32 SizeX,
        int32 SizeY,
        const UInventoryItemData* IgnoredItem)
    {
        if (!BackpackData) return false;
        const int32 MaxGrids = (BackpackData->AdditionalGrids.Num() > 0) ? BackpackData->AdditionalGrids.Num() : 1;
        if (GridIdx < 0 || GridIdx >= MaxGrids) return false;

        const FIntPoint GridSize = BackpackMiniGridSizeByIndex(BackpackData, GridIdx);
        if (StartCellX < 0 || StartCellY < 0) return false;
        if (StartCellX + SizeX > GridSize.X || StartCellY + SizeY > GridSize.Y) return false;

        for (const TPair<TObjectPtr<UInventoryItemData>, FIntPoint>& Pair : BackpackData->StoredCellByItem)
        {
            UInventoryItemData* Other = Pair.Key;
            if (!Other || Other == IgnoredItem) continue;

            int32 OtherGridIdx = 0;
            FIntPoint OtherCell(0, 0);
            if (!GetBackpackPlacement(BackpackData, Other, OtherGridIdx, OtherCell)) continue;
            if (OtherGridIdx != GridIdx) continue;

            bool bOtherRot = false;
            GetBackpackRotation(BackpackData, Other, bOtherRot);
            const int32 OtherSizeX = bOtherRot ? FMath::Max(1, Other->SizeInCellsY) : FMath::Max(1, Other->SizeInCellsX);
            const int32 OtherSizeY = bOtherRot ? FMath::Max(1, Other->SizeInCellsX) : FMath::Max(1, Other->SizeInCellsY);

            const bool bOverlapX = !(StartCellX + SizeX <= OtherCell.X || OtherCell.X + OtherSizeX <= StartCellX);
            const bool bOverlapY = !(StartCellY + SizeY <= OtherCell.Y || OtherCell.Y + OtherSizeY <= StartCellY);
            if (bOverlapX && bOverlapY) return false;
        }
        return true;
    }
}

void UInventoryWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // Строим UI один раз на экземпляр (важно для нового PIE сеанса)
    if (bUIBuilt) return;
    bUIBuilt = true;
    
    // Инициализируем ссылки на виджеты
    VestItemWidgetRef = nullptr;

    // Построим простой UI прямо в C++, даже если класс — Blueprint с уже существующим корневым виджетом
    if (WidgetTree)
    {
        UPanelWidget* RootPanel = Cast<UPanelWidget>(WidgetTree->RootWidget);
        if (!RootPanel)
        {
            UCanvasPanel* NewRoot = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
            WidgetTree->RootWidget = NewRoot;
            RootPanel = NewRoot;
        }

            if (UCanvasPanel* Canvas = Cast<UCanvasPanel>(RootPanel))
        {
            // Полноэкранная подложка через UImage с цветом и альфой
            UImage* Backdrop = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("Backdrop"));
            Backdrop->SetColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.7f));
            if (UCanvasPanelSlot* BackdropSlot = Canvas->AddChildToCanvas(Backdrop))
            {
                BackdropSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
                BackdropSlot->SetOffsets(FMargin(0.f));
                BackdropSlot->SetZOrder(-100); // Фон всегда ниже остальных элементов
            }

            UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Title"));
            Title->SetText(FText::FromString(TEXT("ИНВЕНТАРЬ (I — закрыть)")));
            Title->SetJustification(ETextJustify::Center);
            Title->SetColorAndOpacity(FSlateColor(FLinearColor::White));
            Title->SetShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.8f));
            Title->SetShadowOffset(FVector2D(1.f, 1.f));
            if (UCanvasPanelSlot* TitleSlot = Canvas->AddChildToCanvas(Title))
            {
                TitleSlot->SetAnchors(FAnchors(0.5f, 0.f, 0.5f, 0.f));
                TitleSlot->SetAlignment(FVector2D(0.5f, 0.f));
                TitleSlot->SetPosition(FVector2D(0.f, 40.f));
            }

            // Контейнер справа под 4 грид-области (разгрузка/пояс/карманы/рюкзак)
            UCanvasPanel* RightPanel = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RightPanel"));
            if (UCanvasPanelSlot* RightSlot = Canvas->AddChildToCanvas(RightPanel))
            {
                RightSlot->SetAnchors(FAnchors(1.f, 0.5f, 1.f, 0.5f));
                RightSlot->SetAlignment(FVector2D(1.f, 0.5f));
                RightSlot->SetPosition(FVector2D(-60.f, 0.f));
                RightSlot->SetSize(FVector2D(560.f, 760.f));
            }
            RightPanelRef = RightPanel;
            
            // Создаем панель для слотов экипировки
            UCanvasPanel* EquipmentPanel = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("EquipmentPanel"));
            if (UCanvasPanelSlot* EquipmentSlot = Canvas->AddChildToCanvas(EquipmentPanel))
            {
                EquipmentSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
                EquipmentSlot->SetAlignment(FVector2D(0.5f, 0.5f));
                EquipmentSlot->SetPosition(FVector2D(0.f, 0.f));
                EquipmentSlot->SetSize(FVector2D(400.f, 600.f));
                EquipmentSlot->SetZOrder(10);
            }
            EquipmentPanelRef = EquipmentPanel;

            // Силуэт человека слева. Добавляем под слоты (низкий Z), чтобы не менять их позиционирование
            {
                UImage* Silhouette = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("Silhouette"));
                // Попытаться загрузить текстуру, если художник импортировал её как /Game/UI/T_Silhouette
                if (UTexture2D* SilTex = LoadObject<UTexture2D>(nullptr, TEXT("/Game/UI/T_Silhouette.T_Silhouette")))
                {
                    FSlateBrush Brush;
                    Brush.SetResourceObject(SilTex);
                    Brush.ImageSize = FVector2D(512.f, 1024.f);
                    Silhouette->SetBrush(Brush);
                    Silhouette->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 0.25f));
                }
                else
                {
                    // Фолбэк: белая квадратная текстура из движка, тонированная в полупрозрачный чёрный
                    if (UTexture2D* WhiteTex = LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture")))
                    {
                        FSlateBrush Brush;
                        Brush.SetResourceObject(WhiteTex);
                        Brush.ImageSize = FVector2D(512.f, 1024.f);
                        Silhouette->SetBrush(Brush);
                        Silhouette->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 0.35f));
                    }
                }
                if (UCanvasPanelSlot* SilSlot = Canvas->AddChildToCanvas(Silhouette))
                {
                    SilSlot->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
                    SilSlot->SetAlignment(FVector2D(0.f, 0.f));
                    SilSlot->SetPosition(FVector2D(200.f, 120.f));
                    SilSlot->SetSize(FVector2D(240.f, 700.f));
                    SilSlot->SetZOrder(-1); // Выше фона, ниже слотов и гридов
                }
            }

            auto AddLabeledGrid = [&](const TCHAR* Label, float TopOffset, FVector2D GridSize, int32 GridWidth, int32 GridHeight, const TCHAR* SlotLabel = nullptr, bool bShowLabel = true)
            {
                // Слот слева от надписи
                if (SlotLabel)
                {
                    UBorder* Slot = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
                    Slot->SetBrushColor(FLinearColor(1.f,1.f,1.f,0.05f));
                    if (UCanvasPanelSlot* SS = RightPanel->AddChildToCanvas(Slot))
                    {
                        SS->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
                        SS->SetAlignment(FVector2D(0.f, 0.f));
                        SS->SetPosition(FVector2D(0.f, TopOffset));
                        SS->SetSize(FVector2D(60.f, 60.f));
                    }
                    
                    UTextBlock* SlotText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
                    SlotText->SetText(FText::FromString(SlotLabel));
                    SlotText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
                    SlotText->SetJustification(ETextJustify::Center);
                    if (UCanvasPanelSlot* ST = RightPanel->AddChildToCanvas(SlotText))
                    {
                        ST->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
                        ST->SetAlignment(FVector2D(0.f, 0.f));
                        ST->SetPosition(FVector2D(0.f, TopOffset + 60.f));
                        ST->SetSize(FVector2D(60.f, 20.f));
                    }
                }
                
                // Надпись - показываем только если bShowLabel = true
                if (bShowLabel)
                {
                    UTextBlock* L = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
                    L->SetText(FText::FromString(Label));
                    L->SetColorAndOpacity(FSlateColor(FLinearColor::White));
                    if (UCanvasPanelSlot* S = RightPanel->AddChildToCanvas(L))
                    {
                        S->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
                        S->SetAlignment(FVector2D(0.f, 0.f));
                        S->SetPosition(FVector2D(SlotLabel ? 70.f : 0.f, TopOffset - 30.f));
                        S->SetSize(FVector2D(160.f, 24.f));
                    }
                }
                
                // Грид
                UInventoryGridWidget* GW = WidgetTree->ConstructWidget<UInventoryGridWidget>(UInventoryGridWidget::StaticClass());
                GW->GridWidth = GridWidth; 
                GW->GridHeight = GridHeight;
                if (UCanvasPanelSlot* GS = RightPanel->AddChildToCanvas(GW))
                {
                    GS->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
                    GS->SetAlignment(FVector2D(0.f, 0.f));
                    const FVector2D Pos(SlotLabel ? 70.f : 0.f, TopOffset);
                    GS->SetPosition(Pos);
                    GS->SetSize(GridSize);
                    GS->SetAutoSize(false);
                    // Регистрируем в координатах рут-канвы
                    if (UCanvasPanelSlot* RightBase = Cast<UCanvasPanelSlot>(RightPanel->Slot))
                    {
                        const FVector2D RootPos = RightBase->GetPosition() + Pos;
                        RegisterGrid(Label, RootPos, GridSize, GridWidth, GridHeight);
                    }
                    else
                    {
                    RegisterGrid(Label, Pos, GridSize, GridWidth, GridHeight);
                    }
                }
            };
            // Убираем грид разгрузки из центральной части
            // AddLabeledGrid(TEXT("разгрузка"), 0.f, FVector2D(480.f, 240.f), 8, 4, TEXT("разгрузка"), false);
            // Пояс: стиль задаёт ориентир — полупрозрачный тёмный слот и подпись
            AddLabeledGrid(TEXT("пояс"), 280.f, FVector2D(480.f, 120.f), 8, 2, TEXT("пояс"), false);
            // Карманы: создаём до четырёх мини-гридов по линии слева направо.
            // Размер мини-гридов (в клетках) берём из настроек персонажа (PocketMiniGridSizes).
            {
                const float PocketsTop = 430.f;
                const float CellSize = 60.f;
                // Базовый размер по умолчанию, если в персонаже не задано
                const FIntPoint DefaultPocketCells(1, 1);
                const float StartX = 70.f;
                // Отступ между мини‑гридами карманов берём такой же, как у мини‑гридов жилета.
                const int32 GridGapPx = 6;

                APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(GetOwningPlayerPawn());
                const int32 MaxPockets = 4;
                const int32 PocketCount = PlayerChar ? FMath::Clamp(PlayerChar->PocketMiniGridCount, 0, MaxPockets) : MaxPockets;

                auto GetPocketCells = [&](int32 Index) -> FIntPoint
                {
                    if (PlayerChar && PlayerChar->PocketMiniGridSizes.IsValidIndex(Index))
                    {
                        const FIntPoint C = PlayerChar->PocketMiniGridSizes[Index];
                        return FIntPoint(FMath::Max(1, C.X), FMath::Max(1, C.Y));
                    }
                    return DefaultPocketCells;
                };

                // AddPocket принимает X‑позицию ЛЕВОГО края, как для мини‑гридов жилета.
                // Между карманами используем фиксированный GridGapPx.
                auto AddPocket = [&](const TCHAR* Name, float XPos, int32 CellsX, int32 CellsY)
                {
                    const FVector2D PocketSize(CellSize * CellsX, CellSize * CellsY);

                    UCanvasPanel* Grid = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), FName(Name));
                    // Контейнер кармана не должен перехватывать хиты, только его дети (иконки предметов)
                    Grid->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
                    if (UCanvasPanelSlot* GS = RightPanel->AddChildToCanvas(Grid))
                    {
                        GS->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
                        GS->SetAlignment(FVector2D(0.f, 0.f));
                        GS->SetPosition(FVector2D(XPos, PocketsTop));
                        GS->SetSize(PocketSize);
                        GS->SetAutoSize(false);
                    }
                    // фон как у пояса (единообразный стиль)
                    if (UBorder* Bg = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), FName(*FString::Printf(TEXT("%s_bg"), Name))))
                    {
                        // Стиль фона как у пояса: лёгкий серый
                        Bg->SetBrushColor(FLinearColor(1.f,1.f,1.f,0.05f));
                        // Фон полностью прозрачно-хиттест невидим, чтобы не блокировать взаимодействие с предметами
                        Bg->SetVisibility(ESlateVisibility::HitTestInvisible);
                        if (UCanvasPanelSlot* BGS = Grid->AddChildToCanvas(Bg))
                        {
                            BGS->SetAnchors(FAnchors(0.0f,0.0f,1.f,1.f));
                            BGS->SetOffsets(FMargin(0.0f));
                        }
                    }
                    // Регистрируем в координатах корневого канваса (смещение от RightPanel)
                    if (UCanvasPanelSlot* RightBase = Cast<UCanvasPanelSlot>(RightPanel->Slot))
                    {
                        const FVector2D RootPos = RightBase->GetPosition() + FVector2D(XPos, PocketsTop);
                        RegisterGrid(Name, RootPos, PocketSize, CellsX, CellsY);
                    }
                };
                const int32 ActualPockets = FMath::Min(PocketCount, MaxPockets);
                float XPos = StartX;
                if (ActualPockets >= 1)
                {
                    const FIntPoint Cells = GetPocketCells(0);
                    AddPocket(TEXT("карман1"), XPos, Cells.X, Cells.Y);
                    XPos += CellSize * Cells.X + GridGapPx;
                }
                if (ActualPockets >= 2)
                {
                    const FIntPoint Cells = GetPocketCells(1);
                    AddPocket(TEXT("карман2"), XPos, Cells.X, Cells.Y);
                    XPos += CellSize * Cells.X + GridGapPx;
                }
                if (ActualPockets >= 3)
                {
                    const FIntPoint Cells = GetPocketCells(2);
                    AddPocket(TEXT("карман3"), XPos, Cells.X, Cells.Y);
                    XPos += CellSize * Cells.X + GridGapPx;
                }
                if (ActualPockets >= 4)
                {
                    const FIntPoint Cells = GetPocketCells(3);
                    AddPocket(TEXT("карман4"), XPos, Cells.X, Cells.Y);
                }
            }
            
            // Инициализируем систему отслеживания занятых ячеек
            InitializeOccupiedCells();
            
            // VestOccupiedCells инициализируем динамически при создании гридов жилета
            VestOccupiedCells.Empty();
            
            // Backpack grid removed - no longer needed

            // Левая колонка слотов экипировки
            auto AddEquipSlot = [&](const TCHAR* Label, FVector2D Pos)
            {
                UTextBlock* L = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
                L->SetText(FText::FromString(Label));
                L->SetColorAndOpacity(FSlateColor(FLinearColor::White));
                UBorder* Slot = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
                Slot->SetBrushColor(FLinearColor(1.f,1.f,1.f,0.05f));
                if (UCanvasPanelSlot* LS = Canvas->AddChildToCanvas(L))
                {
                    LS->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
                    LS->SetAlignment(FVector2D(0.f, 0.f));
                    LS->SetPosition(Pos);
                    LS->SetSize(FVector2D(120.f, 20.f));
                }
                if (UCanvasPanelSlot* SS = Canvas->AddChildToCanvas(Slot))
                {
                    SS->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
                    SS->SetAlignment(FVector2D(0.f, 0.f));
                    SS->SetPosition(Pos + FVector2D(0.f, 20.f));
                    SS->SetSize(FVector2D(120.f, 80.f));
                }
            };
            // Базовые слоты в левой колонке
            float y=40.f;
            // Сначала лицо, затем глаза ниже него
            AddEquipSlot(TEXT("лицо"), FVector2D(60.f,y)); y+=110.f;
            AddEquipSlot(TEXT("глаза"), FVector2D(60.f,y)); y+=110.f;
            
            // Координаты по силуэту
            const float SilLeft = 200.f;
            const float SilTop = 120.f;
            const float SilWidth = 240.f;
            const float SilHeight = 700.f;
            const float SilCenterX = SilLeft + SilWidth * 0.5f; // 320
            const float SlotHalfWidth = 60.f; // ширина 120
            const float LabelHeight = 20.f;
            const float BoxHeight = 80.f;
            const float Step = 120.f; // равные интервалы по вертикали

            // Голова (интерактивный слот для шлема): на уровне "лицо", по центру силуэта
            const float HeadY = 150.f;
            {
                UTextBlock* HelmetLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
                HelmetLabel->SetText(FText::FromString(TEXT("голова")));
                HelmetLabel->SetColorAndOpacity(FSlateColor(FLinearColor::White));

                HelmetSlotRef = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("HelmetSlot"));
                HelmetSlotRef->SetBrushColor(FLinearColor(1.f,1.f,1.f,0.05f));

                const FVector2D HelmetPos = FVector2D(SilCenterX - SlotHalfWidth, HeadY);
                if (UCanvasPanelSlot* HelmetLabelSlot = Canvas->AddChildToCanvas(HelmetLabel))
                {
                    HelmetLabelSlot->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
                    HelmetLabelSlot->SetAlignment(FVector2D(0.f, 0.f));
                    HelmetLabelSlot->SetPosition(HelmetPos);
                    HelmetLabelSlot->SetSize(FVector2D(120.f, 20.f));
                }
                if (UCanvasPanelSlot* HelmetSlotCanvas = Canvas->AddChildToCanvas(HelmetSlotRef))
                {
                    HelmetSlotCanvas->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
                    HelmetSlotCanvas->SetAlignment(FVector2D(0.f, 0.f));
                    HelmetSlotCanvas->SetPosition(HelmetPos + FVector2D(0.f, 20.f));
                    HelmetSlotCanvas->SetSize(FVector2D(120.f, 80.f));
                }
            }

            // Обувь остаётся внизу силуэта (фиксированная позиция)
            const float ShoeY = SilTop + SilHeight - (LabelHeight + BoxHeight); // 720

            // Тело и штаны делят отрезок [голова..обувь] на три равные части
            const float Segment = (ShoeY - HeadY) / 3.f;
            AddEquipSlot(TEXT("тело"), FVector2D(SilCenterX - SlotHalfWidth, HeadY + Segment * 1.f));
            // Перчатки справа от слота "тело" на одном уровне
            AddEquipSlot(TEXT("перчатки"), FVector2D(SilCenterX - SlotHalfWidth + 120.f + 20.f, HeadY + Segment * 1.f));
            AddEquipSlot(TEXT("штаны"), FVector2D(SilCenterX - SlotHalfWidth, HeadY + Segment * 2.f));
            AddEquipSlot(TEXT("обувь"), FVector2D(SilCenterX - SlotHalfWidth, ShoeY));

            // Бронежилет на уровне слота "тело" (интерактивный слот)
            {
                UTextBlock* ArmorLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
                ArmorLabel->SetText(FText::FromString(TEXT("бронежилет")));
                ArmorLabel->SetColorAndOpacity(FSlateColor(FLinearColor::White));
                ArmorSlotRef = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("ArmorSlot"));
                ArmorSlotRef->SetBrushColor(FLinearColor(1.f,1.f,1.f,0.05f));

                const FVector2D ArmorPos = FVector2D(60.f, HeadY + Segment * 1.f);
                if (UCanvasPanelSlot* ArmorLabelSlot = Canvas->AddChildToCanvas(ArmorLabel))
                {
                    ArmorLabelSlot->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
                    ArmorLabelSlot->SetAlignment(FVector2D(0.f, 0.f));
                    ArmorLabelSlot->SetPosition(ArmorPos);
                    ArmorLabelSlot->SetSize(FVector2D(120.f, 20.f));
                }
                if (UCanvasPanelSlot* ArmorSlotCanvas = Canvas->AddChildToCanvas(ArmorSlotRef))
                {
                    ArmorSlotCanvas->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
                    ArmorSlotCanvas->SetAlignment(FVector2D(0.f, 0.f));
                    ArmorSlotCanvas->SetPosition(ArmorPos + FVector2D(0.f, 20.f));
                    ArmorSlotCanvas->SetSize(FVector2D(120.f, 80.f));
                }
            }
            // Разгрузка рядом с бронежилетом - сохраняем ссылку
            UTextBlock* VestLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
            VestLabel->SetText(FText::FromString(TEXT("разгрузка")));
            VestLabel->SetColorAndOpacity(FSlateColor(FLinearColor::White));
            VestSlotRef = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
            VestSlotRef->SetBrushColor(FLinearColor(1.f,1.f,1.f,0.05f));
            
            const FVector2D VestPos = FVector2D(60.f, HeadY + Segment * 1.f + 110.f);
            if (UCanvasPanelSlot* VestLabelSlot = Canvas->AddChildToCanvas(VestLabel))
            {
                VestLabelSlot->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
                VestLabelSlot->SetAlignment(FVector2D(0.f, 0.f));
                VestLabelSlot->SetPosition(VestPos);
                VestLabelSlot->SetSize(FVector2D(120.f, 20.f));
            }
            if (UCanvasPanelSlot* VestSlotCanvas = Canvas->AddChildToCanvas(VestSlotRef))
            {
                VestSlotCanvas->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
                VestSlotCanvas->SetAlignment(FVector2D(0.f, 0.f));
                VestSlotCanvas->SetPosition(VestPos + FVector2D(0.f, 20.f));
                VestSlotCanvas->SetSize(FVector2D(120.f, 80.f));
            }
            // Эти два слота перенесём вправо от длинных слотов оружия ниже по коду
            
            // Длинные слоты под силуэтом: основное оружие и оружие на спине
            auto AddWideSlot = [&](const TCHAR* Label, FVector2D Pos, FVector2D Size)
            {
                UTextBlock* WL = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
                WL->SetText(FText::FromString(Label));
                WL->SetColorAndOpacity(FSlateColor(FLinearColor::White));
                UBorder* WSlot = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
                WSlot->SetBrushColor(FLinearColor(1.f,1.f,1.f,0.05f));
                if (UCanvasPanelSlot* WLS = Canvas->AddChildToCanvas(WL))
                {
                    WLS->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
                    WLS->SetAlignment(FVector2D(0.f, 0.f));
                    WLS->SetPosition(Pos);
                    WLS->SetSize(FVector2D(Size.X, 20.f));
                }
                if (UCanvasPanelSlot* WSS = Canvas->AddChildToCanvas(WSlot))
                {
                    WSS->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
                    WSS->SetAlignment(FVector2D(0.f, 0.f));
                    WSS->SetPosition(Pos + FVector2D(0.f, 20.f));
                    WSS->SetSize(FVector2D(Size.X, Size.Y));
                }
            };
            {
                const float wideX = 40.f; // под силуэтом, левее его
                const float wideYStart = SilTop + SilHeight + 20.f; // сразу под силуэтом
                const FVector2D wideSize(400.f, 80.f); // «длинный» слот (по ширине ~4 обычных ячейки)
                AddWideSlot(TEXT("основное оружие"), FVector2D(wideX, wideYStart), wideSize);
                AddWideSlot(TEXT("оружие на спине"), FVector2D(wideX, wideYStart + 120.f), wideSize);

                // Справа от длинных слотов — кобура и холодное оружие, один над другим
                const float smallX = wideX + wideSize.X + 20.f;
                const FVector2D smallSize(120.f, 80.f);
                AddEquipSlot(TEXT("кобура"), FVector2D(smallX, wideYStart));
                AddEquipSlot(TEXT("холодное оружие"), FVector2D(smallX, wideYStart + 120.f));
            }
            
        }
        else
        {
            // Если корень — не Canvas, добавим затемнение как Image
            UImage* BackdropSimple = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("BackdropSimple"));
            BackdropSimple->SetColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.7f));
            RootPanel->AddChild(BackdropSimple);
        }
    }
}

void UInventoryWidget::NativeConstruct()
{
    Super::NativeConstruct();
    bIsFocusable = true; // принимать клавиатурные события
    SetKeyboardFocus();
    SetVisible(false);
    // При конструкте не пересоздаём виджеты, а если уже есть отображаемые — рееставрируем позиции из Placed
    if (RightPanelRef)
    {
        for (const FPlacedItem& P : Placed)
        {
            if (!P.Widget) continue;
            if (UCanvasPanelSlot* S = Cast<UCanvasPanelSlot>(P.Widget->Slot))
            {
                if (GridAreas.IsValidIndex(P.GridIndex))
                {
                    const FGridArea& A = GridAreas[P.GridIndex];
                    const FVector2D CellSize(A.Size.X / FMath::Max(1, A.CellsX), A.Size.Y / FMath::Max(1, A.CellsY));
                    S->SetPosition(A.Position + FVector2D(CellSize.X * P.CellX, CellSize.Y * P.CellY));
                    S->SetSize(FVector2D(60.f * P.SizeX, 60.f * P.SizeY));
                }
            }
        }
    }
}

void UInventoryWidget::SetVisible(bool bIsVisible)
{
	bShown = bIsVisible;
    SetVisibility(bIsVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    // При скрытии инвентаря сохраняем позиции всех предметов жилета и рюкзака в их данные
    if (!bIsVisible)
    {
        if (APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(GetOwningPlayerPawn()))
        {
            if (UInventoryComponent* Inv = PlayerChar->InventoryComponent)
            {
                // Жилет: сохраняем позиции как независимые гриды (GridIndex + LocalCell)
                if (UEquippableItemData* VestData = Inv->GetEquippedItem(Vest))
                {
                    for (UCanvasPanel* Grid : VestGrids)
                    {
                        if (!Grid) continue;
                        const int32 VestIdx = VestGridIndexFromVestGridWidgetName(Grid->GetName());
                        if (VestIdx == INDEX_NONE) continue;
                        TArray<UWidget*> Children = Grid->GetAllChildren();
            for (UWidget* W : Children)
            {
                            if (UInventoryItemWidget* IW = Cast<UInventoryItemWidget>(W))
                            {
                                if (UCanvasPanelSlot* S = Cast<UCanvasPanelSlot>(IW->Slot))
                                {
                                    const int32 CellX = FMath::RoundToInt(S->GetPosition().X / 60.f);
                                    const int32 CellY = FMath::RoundToInt(S->GetPosition().Y / 60.f);
                                    const FIntPoint GridSize = VestMiniGridSizeByIndex(VestData, VestIdx);
                                    const FIntPoint LocalCell(
                                        FMath::Clamp(CellX, 0, FMath::Max(0, GridSize.X - 1)),
                                        FMath::Clamp(CellY, 0, FMath::Max(0, GridSize.Y - 1))
                                    );
                                    VestData->StoredGridByItem.Add(IW->ItemData, VestIdx);
                                    VestData->PersistentGridByItem.Add(IW->ItemData, VestIdx);
                                    VestData->StoredCellByItem.Add(IW->ItemData, LocalCell);
                                    VestData->PersistentCellByItem.Add(IW->ItemData, LocalCell);
                                    VestData->StoredRotByItem.Add(IW->ItemData, IW->bRotated);
                                    VestData->PersistentRotByItem.Add(IW->ItemData, IW->bRotated);
                                }
                            }
                        }
                    }
                }
                // Рюкзак: сохраняем позиции как независимые мини‑гриды (GridIndex + LocalCell)
                if (UEquippableItemData* BackpackData = Inv->GetEquippedItem(Backpack))
                {
                    for (int32 GridIdx = 0; GridIdx < BackpackGrids.Num(); ++GridIdx)
                    {
                        UCanvasPanel* Grid = BackpackGrids[GridIdx];
                        if (!Grid) continue;
                        TArray<UWidget*> Children = Grid->GetAllChildren();
                        for (UWidget* W : Children)
                        {
                            if (UInventoryItemWidget* IW = Cast<UInventoryItemWidget>(W))
                            {
                                if (UCanvasPanelSlot* S = Cast<UCanvasPanelSlot>(IW->Slot))
                                {
                                    const int32 CellX = FMath::RoundToInt(S->GetPosition().X / 60.f);
                                    const int32 CellY = FMath::RoundToInt(S->GetPosition().Y / 60.f);
                                    BackpackData->StoredGridByItem.Add(IW->ItemData, GridIdx);
                                    BackpackData->PersistentGridByItem.Add(IW->ItemData, GridIdx);
                                    BackpackData->StoredCellByItem.Add(IW->ItemData, FIntPoint(CellX, CellY));
                                    BackpackData->PersistentCellByItem.Add(IW->ItemData, FIntPoint(CellX, CellY));
                                    BackpackData->StoredRotByItem.Add(IW->ItemData, IW->bRotated);
                                    BackpackData->PersistentRotByItem.Add(IW->ItemData, IW->bRotated);
                                }
                            }
                        }
                    }
                }

                // Открытое окно модуля: сохраняем позиции и закрываем окно
                if (OpenedEquipMod && OpenedEquipMod->bHasAdditionalStorage)
                {
                    for (int32 GridIdx = 0; GridIdx < EquipModGrids.Num(); ++GridIdx)
                    {
                        UCanvasPanel* Grid = EquipModGrids[GridIdx];
                        if (!Grid) continue;
                        TArray<UWidget*> Children = Grid->GetAllChildren();
                        for (UWidget* W : Children)
                        {
                            if (UInventoryItemWidget* IW = Cast<UInventoryItemWidget>(W))
                            {
                                if (UCanvasPanelSlot* S = Cast<UCanvasPanelSlot>(IW->Slot))
                                {
                                    const int32 CellX = FMath::RoundToInt(S->GetPosition().X / 60.f);
                                    const int32 CellY = FMath::RoundToInt(S->GetPosition().Y / 60.f);
                                    OpenedEquipMod->StoredGridByItem.Add(IW->ItemData, GridIdx);
                                    OpenedEquipMod->PersistentGridByItem.Add(IW->ItemData, GridIdx);
                                    OpenedEquipMod->StoredCellByItem.Add(IW->ItemData, FIntPoint(CellX, CellY));
                                    OpenedEquipMod->PersistentCellByItem.Add(IW->ItemData, FIntPoint(CellX, CellY));
                                    OpenedEquipMod->StoredRotByItem.Add(IW->ItemData, IW->bRotated);
                                    OpenedEquipMod->PersistentRotByItem.Add(IW->ItemData, IW->bRotated);
                                }
                            }
                        }
                    }
                    CloseEquipModStorage();
                }

                // Гриды модов брони: позиции предметов уже хранятся в Stored*/Persistent* при dnd,
                // здесь дополнительной синхронизации не требуется.
            }
        }
    }
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, bIsVisible ? FColor::Green : FColor::Silver, bIsVisible ? TEXT("Inventory visibility: VISIBLE") : TEXT("Inventory visibility: HIDDEN"));
    }
}

void UInventoryWidget::SyncBackpack(const TArray<UInventoryItemData*>& Items)
{
    // Legacy function - no longer needed since backpack grid is removed
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("⚠️ SyncBackpack called - backpack grid removed"));
    }
}

void UInventoryWidget::UpdateStaticEquipmentSlots()
{
    if (!EquipmentPanelRef)
    {
        return;
    }

    // Создаём слот рюкзака, если он ещё не создан
    if (!BackpackSlotRef)
    {
        // Подпись слота "рюкзак"
        if (UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("BackpackSlotLabel")))
        {
            Label->SetText(FText::FromString(TEXT("рюкзак")));
            Label->SetColorAndOpacity(FSlateColor(FLinearColor::White));
            if (UCanvasPanelSlot* LS = Cast<UCanvasPanelSlot>(EquipmentPanelRef->AddChildToCanvas(Label)))
            {
                LS->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
                LS->SetAlignment(FVector2D(0.f, 0.f));
                // Размещаем под блоком карманов
                LS->SetPosition(FVector2D(10.f, 190.f));
                LS->SetZOrder(19);
            }
        }

        UBorder* BackpackSlotBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("BackpackSlot"));
        BackpackSlotBorder->SetBrushColor(FLinearColor(1.f,1.f,1.f,0.05f));
        if (UCanvasPanelSlot* S = Cast<UCanvasPanelSlot>(EquipmentPanelRef->AddChildToCanvas(BackpackSlotBorder)))
    {
        S->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
        S->SetAlignment(FVector2D(0.f, 0.f));
            // Размещаем под карманами, такого же размера, как остальные слоты
            S->SetPosition(FVector2D(10.f, 210.f));
            S->SetSize(EquipmentSlotSize);
            S->SetZOrder(20);
        }
        BackpackSlotRef = BackpackSlotBorder;
    }

    // Слот жилета (разгрузка)
    if (!VestSlotRef)
    {
        if (UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("VestSlotLabel")))
        {
            Label->SetText(FText::FromString(TEXT("разгрузка")));
            Label->SetColorAndOpacity(FSlateColor(FLinearColor::White));
            if (UCanvasPanelSlot* LS = Cast<UCanvasPanelSlot>(EquipmentPanelRef->AddChildToCanvas(Label)))
            {
                LS->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
                LS->SetAlignment(FVector2D(0.f, 0.f));
                LS->SetPosition(FVector2D(10.f, 90.f));
                LS->SetZOrder(19);
            }
        }
        UBorder* VestSlotBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("VestSlot"));
        VestSlotBorder->SetBrushColor(FLinearColor(1.f,1.f,1.f,0.05f));
        if (UCanvasPanelSlot* S = Cast<UCanvasPanelSlot>(EquipmentPanelRef->AddChildToCanvas(VestSlotBorder)))
    {
        S->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
        S->SetAlignment(FVector2D(0.f, 0.f));
            S->SetPosition(FVector2D(10.f, 110.f));
            S->SetSize(EquipmentSlotSize);
            S->SetZOrder(20);
        }
        VestSlotRef = VestSlotBorder;
    }

    // Слот бронежилета
    if (!ArmorSlotRef)
    {
        if (UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ArmorSlotLabel")))
        {
            Label->SetText(FText::FromString(TEXT("бронежилет")));
            Label->SetColorAndOpacity(FSlateColor(FLinearColor::White));
            if (UCanvasPanelSlot* LS = Cast<UCanvasPanelSlot>(EquipmentPanelRef->AddChildToCanvas(Label)))
            {
                LS->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
                LS->SetAlignment(FVector2D(0.f, 0.f));
                LS->SetPosition(FVector2D(10.f, 10.f));
                LS->SetZOrder(19);
            }
        }

        UBorder* ArmorSlotBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("ArmorSlotFallback"));
        ArmorSlotBorder->SetBrushColor(FLinearColor(1.f,1.f,1.f,0.05f));
        if (UCanvasPanelSlot* S = Cast<UCanvasPanelSlot>(EquipmentPanelRef->AddChildToCanvas(ArmorSlotBorder)))
        {
            S->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
            S->SetAlignment(FVector2D(0.f, 0.f));
            S->SetPosition(FVector2D(10.f, 30.f));
            S->SetSize(EquipmentSlotSize);
            S->SetZOrder(20);
        }
        ArmorSlotRef = ArmorSlotBorder;
    }
}

void UInventoryWidget::UpdateEquipmentSlots()
{
    UpdateStaticEquipmentSlots();

    // Очищаем виджеты в слотах, если они уже есть
    if (BackpackSlotRef && BackpackItemWidgetRef)
    {
        BackpackSlotRef->ClearChildren();
        BackpackItemWidgetRef = nullptr;
    }
    if (VestSlotRef && VestItemWidgetRef)
    {
        VestSlotRef->ClearChildren();
        VestItemWidgetRef = nullptr;
    }
    if (ArmorSlotRef && ArmorItemWidgetRef)
    {
        ArmorSlotRef->ClearChildren();
        ArmorItemWidgetRef = nullptr;
    }
    if (HelmetSlotRef && HelmetItemWidgetRef)
    {
        HelmetSlotRef->ClearChildren();
        HelmetItemWidgetRef = nullptr;
    }

    // Получаем экипированный рюкзак и отображаем его в слоте
    if (APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(GetOwningPlayerPawn()))
    {
        if (UInventoryComponent* InvComp = PlayerChar->InventoryComponent)
        {
            // Шлем
            if (UEquippableItemData* EquippedHelmet = InvComp->GetEquippedItem(Helmet))
            {
                if (HelmetSlotRef)
                {
                    UInventoryItemWidget* ItemW = WidgetTree->ConstructWidget<UInventoryItemWidget>(UInventoryItemWidget::StaticClass());
                    ItemW->bIsStaticEquipmentSlot = true;
                    ItemW->Init(EquippedHelmet, EquippedHelmet->Icon, EquipmentSlotSize);
                    HelmetSlotRef->SetContent(ItemW);
                    HelmetItemWidgetRef = ItemW;
                }
            }

            // Бронежилет
            if (UEquippableItemData* EquippedArmor = InvComp->GetEquippedItem(Armor))
            {
                if (ArmorSlotRef)
                {
                    UInventoryItemWidget* ItemW = WidgetTree->ConstructWidget<UInventoryItemWidget>(UInventoryItemWidget::StaticClass());
                    ItemW->bIsStaticEquipmentSlot = true;
                    ItemW->Init(EquippedArmor, EquippedArmor->Icon, EquipmentSlotSize);
                    ArmorSlotRef->SetContent(ItemW);
                    ArmorItemWidgetRef = ItemW;
                }
            }

            // Жилет
            if (UEquippableItemData* EquippedVest = InvComp->GetEquippedItem(Vest))
            {
                if (VestSlotRef)
                {
                    UInventoryItemWidget* ItemW = WidgetTree->ConstructWidget<UInventoryItemWidget>(UInventoryItemWidget::StaticClass());
                    ItemW->bIsStaticEquipmentSlot = true;
                    ItemW->Init(EquippedVest, EquippedVest->Icon, EquipmentSlotSize);
                    VestSlotRef->SetContent(ItemW);
                    VestItemWidgetRef = ItemW;
                }
            }

            if (UEquippableItemData* EquippedBackpack = InvComp->GetEquippedItem(Backpack))
            {
                if (BackpackSlotRef)
                {
                    UInventoryItemWidget* ItemW = WidgetTree->ConstructWidget<UInventoryItemWidget>(UInventoryItemWidget::StaticClass());
                    ItemW->bIsStaticEquipmentSlot = true;
                    // Инициализируем виджет предмета (иконка может быть nullptr — в виджете должен быть дефолт)
                    ItemW->Init(EquippedBackpack, EquippedBackpack->Icon, EquipmentSlotSize);
                    BackpackSlotRef->SetContent(ItemW);
                    BackpackItemWidgetRef = ItemW;
                }

                // Показать/обновить грид хранилища рюкзака под слотом
                UpdateBackpackStorageGrid();
            }
        }
    }
}

void UInventoryWidget::UpdatePocketsGrid()
{
    if (!RightPanelRef) return;

    // Получаем компонент инвентаря
    APlayerController* PC = GetOwningPlayer();
    if (!PC) return;
    
    APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(PC->GetPawn());
    if (!PlayerChar || !PlayerChar->InventoryComponent) return;
    
    UInventoryComponent* InvComp = PlayerChar->InventoryComponent;
    
    // Обновляем каждый карман
    const TArray<TArray<TObjectPtr<UInventoryItemData>>*> PocketArrays = {
        &InvComp->Pocket1Items,
        &InvComp->Pocket2Items, 
        &InvComp->Pocket3Items,
        &InvComp->Pocket4Items
    };
    
    for (int32 PocketIndex = 0; PocketIndex < 4; ++PocketIndex)
    {
        FString PocketName = FString::Printf(TEXT("карман%d"), PocketIndex + 1);
        
        // Находим грид кармана
        UCanvasPanel* PocketGrid = nullptr;
        TArray<UWidget*> Children = RightPanelRef->GetAllChildren();
        for (UWidget* Child : Children)
        {
            if (Child && Child->GetFName() == FName(*PocketName))
            {
                PocketGrid = Cast<UCanvasPanel>(Child);
                break;
            }
        }
        
        if (!PocketGrid) continue;

        // Полностью очищаем грид и рисуем сетку маленьких клеток,
        // размеры которых берём из настроек персонажа (PocketMiniGridSizes).
        PocketGrid->ClearChildren();

        const int32 CellSize = 60;
        const FIntPoint DefaultCells(3, 2);
        FIntPoint Cells(DefaultCells);
        if (PlayerChar && PlayerChar->PocketMiniGridSizes.IsValidIndex(PocketIndex))
        {
            const FIntPoint C = PlayerChar->PocketMiniGridSizes[PocketIndex];
            Cells = FIntPoint(FMath::Max(1, C.X), FMath::Max(1, C.Y));
        }
        const int32 CellsX = Cells.X;
        const int32 CellsY = Cells.Y;

        for (int32 Row = 0; Row < CellsY; ++Row)
        {
            for (int32 Col = 0; Col < CellsX; ++Col)
            {
                UCanvasPanel* CellContainer = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass());
                if (CellContainer)
                {
                    UBorder* CellBg = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
                    if (CellBg)
                    {
                        CellBg->SetBrushColor(FLinearColor(1.f, 1.f, 1.f, 0.05f));
                        if (UCanvasPanelSlot* BgSlot = CellContainer->AddChildToCanvas(CellBg))
                        {
                            BgSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
                            BgSlot->SetOffsets(FMargin(1.f));
                        }
                    }
                    if (UCanvasPanelSlot* CellSlot = PocketGrid->AddChildToCanvas(CellContainer))
                    {
                        CellSlot->SetPosition(FVector2D(Col * CellSize, Row * CellSize));
                        CellSlot->SetSize(FVector2D((float)CellSize, (float)CellSize));
                    }
                }
            }
        }

        // Добавляем предметы из кармана. Теперь каждый карман — полноценный мини‑грид,
        // и каждый предмет имеет свои координаты ячейки внутри него.
        const TArray<TObjectPtr<UInventoryItemData>>& PocketItems = *PocketArrays[PocketIndex];
        for (UInventoryItemData* Item : PocketItems)
        {
            if (!Item) continue;

            const int32* PocketIdxPtr = InvComp->PocketIndexByItem.Find(Item);
            const FIntPoint* CellPtr = InvComp->PocketCellByItem.Find(Item);
            const bool* bRotPtr = InvComp->PocketRotByItem.Find(Item);
            if (!PocketIdxPtr || *PocketIdxPtr != PocketIndex + 1 || !CellPtr)
            {
                continue;
            }

            const bool bRotated = bRotPtr ? *bRotPtr : false;
            const int32 ItemCellsX = bRotated ? FMath::Max(1, Item->SizeInCellsY) : FMath::Max(1, Item->SizeInCellsX);
            const int32 ItemCellsY = bRotated ? FMath::Max(1, Item->SizeInCellsX) : FMath::Max(1, Item->SizeInCellsY);

            UInventoryItemWidget* ItemWidget = WidgetTree->ConstructWidget<UInventoryItemWidget>(UInventoryItemWidget::StaticClass());
            ItemWidget->bRotated = bRotated;
            ItemWidget->Init(Item, Item->Icon, FVector2D(ItemCellsX * CellSize, ItemCellsY * CellSize));
            ItemWidget->SetVisibility(ESlateVisibility::Visible);
            ItemWidget->SetIsEnabled(true);

            if (UCanvasPanelSlot* ItemSlot = Cast<UCanvasPanelSlot>(PocketGrid->AddChildToCanvas(ItemWidget)))
            {
                ItemSlot->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
                ItemSlot->SetAlignment(FVector2D(0.f, 0.f));
                ItemSlot->SetPosition(FVector2D(CellPtr->X * CellSize, CellPtr->Y * CellSize));
                ItemSlot->SetSize(FVector2D(ItemCellsX * CellSize, ItemCellsY * CellSize));
            }
        }
    }
}

void UInventoryWidget::UpdateBackpackStorageGrid()
{
    // Работать будем относительно корневого Canvas, чтобы координаты совпадали с hit-test
    UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetTree ? WidgetTree->RootWidget : nullptr);
    if (!RootCanvas) return;

    // Дальше: определим лэйаут; если он не поменялся — НЕ пересоздаём гриды (это и вызывало лаги),
    // а просто переотрисуем предметы.

    // Проверяем, экипирован ли рюкзак и есть ли у него хранилище
    if (APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(GetOwningPlayerPawn()))
    {
        if (UInventoryComponent* InvComp = PlayerChar->InventoryComponent)
        {
            if (UEquippableItemData* EquippedBackpack = InvComp->GetEquippedItem(Backpack))
            {
                const int32 CellSizePx = 60;
                const int32 GridGapPx = 6; // фиксированный зазор между соседними мини‑гридами
                const FVector2D BaseTopLeft = FVector2D(821.f, 689.f);

                TArray<FIntPoint> GridSizes = EquippedBackpack->AdditionalGrids;
                TArray<FIntPoint> GridOffsets = EquippedBackpack->AdditionalGridOffsets;
                if (GridSizes.Num() == 0)
                {
                    const int32 GX = (EquippedBackpack->AdditionalGridSize.X > 0 ? EquippedBackpack->AdditionalGridSize.X : 8);
                    const int32 GY = (EquippedBackpack->AdditionalGridSize.Y > 0 ? EquippedBackpack->AdditionalGridSize.Y : 6);
                    GridSizes = { FIntPoint(GX, GY) };
                    GridOffsets = { FIntPoint(0, 0) };
                }
                if (GridOffsets.Num() != GridSizes.Num())
                {
                    GridOffsets.SetNum(GridSizes.Num());
                    for (int32 i = 0; i < GridOffsets.Num(); ++i) GridOffsets[i] = FIntPoint(i, 0);
                }

                const uint32 LayoutHash = HashGridLayout(GridSizes, GridOffsets);
                const bool bNeedRebuild = (BackpackGrids.Num() != GridSizes.Num()) || (CachedBackpackLayoutHash != LayoutHash);

                if (bNeedRebuild)
                {
                    // Удаляем старые канвы мини‑гридов
                    TArray<UWidget*> ToRemove;
                    const int32 NumChildren = RootCanvas->GetChildrenCount();
                    for (int32 i = 0; i < NumChildren; ++i)
                    {
                        if (UWidget* C = RootCanvas->GetChildAt(i))
                        {
                            if (C->GetName().StartsWith(TEXT("BackpackGrid_")))
                            {
                                ToRemove.Add(C);
                            }
                            if (C->GetFName() == TEXT("BackpackStorageCanvas"))
                            {
                                ToRemove.Add(C);
                            }
                        }
                    }
                    for (UWidget* R : ToRemove) { RootCanvas->RemoveChild(R); }
                    BackpackGrids.Empty();
                    BackpackGridSizes.Empty();

                    // Очищаем старые grid-areas "рюкзак*", чтобы FindGridAtPoint не перехватывал старые области
                    GridAreas.RemoveAll([](const FGridArea& A) { return A.Name.StartsWith(TEXT("рюкзак")); });
                }
                else
                {
                    // Лэйаут тот же — чистим только предметы (не фон/ячейки)
                    for (UCanvasPanel* G : BackpackGrids)
                    {
                        if (!G) continue;
                        TArray<UWidget*> Children = G->GetAllChildren();
                        for (UWidget* W : Children)
                        {
                            if (Cast<UInventoryItemWidget>(W))
                            {
                                W->RemoveFromParent();
                            }
                        }
                    }
                }

                auto AddBackpackMiniGrid = [&](int32 GridIdx, const FVector2D& Pos, const FIntPoint& Cells)
                {
                    const FString Label = FString::Printf(TEXT("рюкзак%d"), GridIdx + 1);
                    const FName GridName(*FString::Printf(TEXT("BackpackGrid_%s"), *Label));
                    UCanvasPanel* Grid = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), GridName);
                    if (!Grid) return;

                    const FVector2D SizePx((float)(Cells.X * CellSizePx), (float)(Cells.Y * CellSizePx));
                    BackpackGrids.Add(Grid);
                    BackpackGridSizes.Add(SizePx);

                    if (UCanvasPanelSlot* GS = Cast<UCanvasPanelSlot>(RootCanvas->AddChildToCanvas(Grid)))
                    {
                        GS->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
                        GS->SetAlignment(FVector2D(0.f, 0.f));
                        GS->SetPosition(Pos);
                        GS->SetSize(SizePx);
                        GS->SetZOrder(15);
                    }

                    if (UBorder* GridBackground = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass()))
                    {
                        GridBackground->SetBrushColor(FLinearColor(1.f,1.f,1.f,0.05f));
                        GridBackground->SetVisibility(ESlateVisibility::HitTestInvisible);
                        if (UCanvasPanelSlot* BgSlot = Grid->AddChildToCanvas(GridBackground))
                        {
                            BgSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
                            BgSlot->SetOffsets(FMargin(0.0f));
                            BgSlot->SetZOrder(0);
                        }
                    }

                    // Регистрируем область мини‑грида для dnd/снаппинга в координатах корня
                    RegisterGrid(Label, Pos, SizePx, Cells.X, Cells.Y);

                    // Рисуем видимую сетку (1px зазоры)
                    for (int32 yCell = 0; yCell < Cells.Y; ++yCell)
                    {
                        for (int32 xCell = 0; xCell < Cells.X; ++xCell)
                        {
                            UBorder* Cell = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
                            if (!Cell) continue;
                            Cell->SetBrushColor(FLinearColor(1.f, 1.f, 1.f, 0.06f));
                            Cell->SetVisibility(ESlateVisibility::HitTestInvisible);
                            if (UCanvasPanelSlot* CellSlot = Grid->AddChildToCanvas(Cell))
                            {
                                CellSlot->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
                                CellSlot->SetAlignment(FVector2D(0.f, 0.f));
                                CellSlot->SetPosition(FVector2D(xCell * 60.f + 1.f, yCell * 60.f + 1.f));
                                CellSlot->SetSize(FVector2D(58.f, 58.f));
                                CellSlot->SetZOrder(1);
                            }
                        }
                    }
                };

                if (bNeedRebuild)
                {
                    TMap<int32, float> ColStartPx;
                    TMap<int32, float> RowStartPx;
                    BuildTableLayoutPx(GridSizes, GridOffsets, CellSizePx, GridGapPx, ColStartPx, RowStartPx);

                    for (int32 i = 0; i < GridSizes.Num(); ++i)
                    {
                        const FIntPoint Sz(FMath::Max(1, GridSizes[i].X), FMath::Max(1, GridSizes[i].Y));
                        const FIntPoint Off = GridOffsets.IsValidIndex(i) ? GridOffsets[i] : FIntPoint(0, 0);
                        const float X0 = ColStartPx.Contains(Off.X) ? ColStartPx[Off.X] : 0.f;
                        const float Y0 = RowStartPx.Contains(Off.Y) ? RowStartPx[Off.Y] : 0.f;
                        const FVector2D Pos = BaseTopLeft + FVector2D(X0, Y0);
                        AddBackpackMiniGrid(i, Pos, Sz);
                    }
                    CachedBackpackLayoutHash = LayoutHash;
                }

                // Отрисовываем предметы из хранилища рюкзака (с учётом GridIdx + LocalCell)
                TArray<UInventoryItemData*> Stored = InvComp->GetEquipmentStorageItems(EquippedBackpack);
                const FVector2D CellSize(60.f, 60.f);
                for (UInventoryItemData* It : Stored)
                {
                    if (!It) continue;
                    UInventoryItemWidget* W = WidgetTree->ConstructWidget<UInventoryItemWidget>(UInventoryItemWidget::StaticClass());
                    W->Init(It, It->Icon, CellSize);
                    W->SetVisibility(ESlateVisibility::Visible);
                    W->SetIsEnabled(true);
                    int32 GridIdx = 0;
                    FIntPoint LocalCell(0, 0);
                    if (!GetBackpackPlacement(EquippedBackpack, It, GridIdx, LocalCell))
                    {
                        GridIdx = 0;
                        if (const FIntPoint* C = EquippedBackpack->StoredCellByItem.Find(It)) LocalCell = *C;
                        else if (const FIntPoint* C2 = EquippedBackpack->PersistentCellByItem.Find(It)) LocalCell = *C2;
                    }
                    UCanvasPanel* TargetGrid = BackpackGrids.IsValidIndex(GridIdx) ? BackpackGrids[GridIdx] : (BackpackGrids.Num() > 0 ? BackpackGrids[0] : nullptr);
                    if (TargetGrid)
                    {
                        if (UCanvasPanelSlot* S = Cast<UCanvasPanelSlot>(TargetGrid->AddChildToCanvas(W)))
                        {
                            S->SetAnchors(FAnchors(0.f,0.f,0.f,0.f));
                            S->SetAlignment(FVector2D(0.f,0.f));
                            S->SetPosition(FVector2D(LocalCell.X * CellSize.X, LocalCell.Y * CellSize.Y));
                            bool bRot = false;
                            GetBackpackRotation(EquippedBackpack, It, bRot);
                            const int32 SX = bRot ? FMath::Max(1, It->SizeInCellsY) : FMath::Max(1, It->SizeInCellsX);
                            const int32 SY = bRot ? FMath::Max(1, It->SizeInCellsX) : FMath::Max(1, It->SizeInCellsY);
                            W->bRotated = bRot;
                            S->SetSize(FVector2D(CellSize.X * SX, CellSize.Y * SY));
                            S->SetZOrder(2);
                        }
                    }
                    // Нормализуем в новый формат
                    EquippedBackpack->StoredGridByItem.Add(It, GridIdx);
                    EquippedBackpack->PersistentGridByItem.Add(It, GridIdx);
                    EquippedBackpack->StoredCellByItem.Add(It, LocalCell);
                    EquippedBackpack->PersistentCellByItem.Add(It, LocalCell);
                }
            }
        }
    }
}

bool UInventoryWidget::IsAreaFreeInBackpack(const UEquippableItemData* BackpackData,
                              int32 StartCellX,
                              int32 StartCellY,
                              int32 SizeX,
                              int32 SizeY,
                              const UInventoryItemData* IgnoredItem) const
{
    // Legacy wrapper: считаем, что это grid 0 и координаты локальные.
    return IsAreaFreeInBackpackMiniGrid(BackpackData, 0, StartCellX, StartCellY, SizeX, SizeY, IgnoredItem);
}

// Универсальная проверка для любого Equipment-хранилища (в т.ч. жилета)
bool UInventoryWidget::IsAreaFreeInEquipment(const UEquippableItemData* EquipmentData,
                              int32 StartCellX,
                              int32 StartCellY,
                              int32 SizeX,
                              int32 SizeY,
                              const UInventoryItemData* IgnoredItem) const
{
    if (!EquipmentData) return false;
    const int32 GridX = FMath::Max(1, EquipmentData->AdditionalGridSize.X);
    const int32 GridY = FMath::Max(1, EquipmentData->AdditionalGridSize.Y);
    if (StartCellX < 0 || StartCellY < 0) return false;
    if (StartCellX + SizeX > GridX || StartCellY + SizeY > GridY) return false;

    for (const TPair<TObjectPtr<UInventoryItemData>, FIntPoint>& Pair : EquipmentData->StoredCellByItem)
    {
        UInventoryItemData* Other = Pair.Key;
        if (!Other || Other == IgnoredItem) continue;
        const FIntPoint OtherCell = Pair.Value;
        const bool bOtherRot =
            (EquipmentData->PersistentRotByItem.Contains(Other) ? EquipmentData->PersistentRotByItem[Other] :
             (EquipmentData->StoredRotByItem.Contains(Other) ? EquipmentData->StoredRotByItem[Other] : false));
        const int32 OtherSizeX = bOtherRot ? FMath::Max(1, Other->SizeInCellsY) : FMath::Max(1, Other->SizeInCellsX);
        const int32 OtherSizeY = bOtherRot ? FMath::Max(1, Other->SizeInCellsX) : FMath::Max(1, Other->SizeInCellsY);
        const bool bOverlapX = !(StartCellX + SizeX <= OtherCell.X || OtherCell.X + OtherSizeX <= StartCellX);
        const bool bOverlapY = !(StartCellY + SizeY <= OtherCell.Y || OtherCell.Y + OtherSizeY <= StartCellY);
        if (bOverlapX && bOverlapY)
        {
            return false;
        }
    }
    return true;
}

// Поиск ближайшей свободной ячейки вокруг стартовой точки (по манхэттену), учитывая размер предмета
bool UInventoryWidget::FindNearestFreeCellInEquipment(const UEquippableItemData* EquipmentData,
                                        int32 SizeX,
                                        int32 SizeY,
                                        int32 StartX,
                                        int32 StartY,
                                        int32& OutX,
                                        int32& OutY,
                                        const UInventoryItemData* IgnoredItem) const
{
    if (!EquipmentData) return false;
    const int32 GridX = FMath::Max(1, EquipmentData->AdditionalGridSize.X);
    const int32 GridY = FMath::Max(1, EquipmentData->AdditionalGridSize.Y);

    auto InBounds = [&](int32 X, int32 Y){ return X >= 0 && Y >= 0 && X + SizeX <= GridX && Y + SizeY <= GridY; };

    // Спиральный поиск по возрастанию расстояния Манхэттена
    const int32 MaxR = GridX + GridY; // верхняя граница
    for (int32 r = 0; r <= MaxR; ++r)
    {
        for (int32 dy = -r; dy <= r; ++dy)
        {
            const int32 dx = r - FMath::Abs(dy);
            const int32 candX1 = StartX + dx;
            const int32 candY1 = StartY + dy;
            if (InBounds(candX1, candY1) && IsAreaFreeInEquipment(EquipmentData, candX1, candY1, SizeX, SizeY, IgnoredItem))
            {
                OutX = candX1; OutY = candY1; return true;
            }
            const int32 candX2 = StartX - dx;
            const int32 candY2 = StartY + dy;
            if (dx != 0 && InBounds(candX2, candY2) && IsAreaFreeInEquipment(EquipmentData, candX2, candY2, SizeX, SizeY, IgnoredItem))
            {
                OutX = candX2; OutY = candY2; return true;
            }
        }
    }
    return false;
}

void UInventoryWidget::RemoveExistingItemWidget(UInventoryItemData* ItemData)
{
    if (!ItemData || !RightPanelRef) return;
    // Удаляем из правой панели (карманы/жилетные гриды живут там)
    const int32 Count = RightPanelRef->GetChildrenCount();
    for (int32 i = 0; i < Count; ++i)
    {
        if (UWidget* Child = RightPanelRef->GetChildAt(i))
        {
            if (UCanvasPanel* Panel = Cast<UCanvasPanel>(Child))
            {
                TArray<UWidget*> Children = Panel->GetAllChildren();
                for (UWidget* W : Children)
                {
                    if (UInventoryItemWidget* I = Cast<UInventoryItemWidget>(W))
                    {
                        if (I->ItemData == ItemData)
                        {
                            Panel->RemoveChild(I);
                            break;
                        }
                    }
                }
            }
        }
    }
    // Также удаляем из рут-канвы (BackpackStorageCanvas)
    if (UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetTree ? WidgetTree->RootWidget : nullptr))
    {
        const int32 RCount = RootCanvas->GetChildrenCount();
        for (int32 i = 0; i < RCount; ++i)
        {
            if (UWidget* Child = RootCanvas->GetChildAt(i))
            {
                if (Child->GetFName() == TEXT("BackpackStorageCanvas"))
                {
                    if (UCanvasPanel* Grid = Cast<UCanvasPanel>(Child))
                    {
                        TArray<UWidget*> GChildren = Grid->GetAllChildren();
                        for (UWidget* W : GChildren)
                        {
                            if (UInventoryItemWidget* I = Cast<UInventoryItemWidget>(W))
                            {
                                if (I->ItemData == ItemData)
                                {
                                    Grid->RemoveChild(I);
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void UInventoryWidget::RefreshInventoryUI()
{
    if (APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(GetOwningPlayerPawn()))
    {
        if (UInventoryComponent* InvComp = PlayerChar->InventoryComponent)
        {
            UpdateEquipmentSlots();
            // Обновляем грид жилета, если он экипирован
            UpdateVestGrid();
            // Обновляем карманы
            UpdatePocketsGrid();
            UpdateBackpackStorageGrid();
            SyncBackpack(InvComp->BackpackItems);
        }
    }
}

void UInventoryWidget::AddBackpackItemIcon(UInventoryItemData* ItemData)
{
    // Legacy function - no longer needed since backpack grid is removed
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("⚠️ AddBackpackItemIcon called - backpack grid removed"));
    }
}

void UInventoryWidget::RemoveItemWidget(UInventoryItemWidget* Widget)
{
    if (Widget && RightPanelRef)
    {
        RightPanelRef->RemoveChild(Widget);
    }
}

void UInventoryWidget::RemoveItemMapping(UInventoryItemData* ItemData)
{
    if (ItemData)
    {
        ItemToWidget.Remove(ItemData);
    }
}

void UInventoryWidget::ClearItemPosition(UInventoryItemData* ItemData)
{
    // Remove from Placed array by finding the widget that corresponds to this ItemData
    for (int32 i = Placed.Num() - 1; i >= 0; --i)
    {
        if (Placed[i].Widget && Placed[i].Widget->ItemData == ItemData)
        {
            Placed.RemoveAt(i);
            break;
        }
    }
}

FReply UInventoryWidget::HandleItemRotation(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    if (InKeyEvent.GetKey() == EKeys::R)
    {
        // 1) Если есть активный dnd — вращаем перетаскиваемый
        if (UDragDropOperation* Op = UWidgetBlueprintLibrary::GetDragDroppingContent())
        {
            if (UInventoryItemWidget* Dragged = Cast<UInventoryItemWidget>(Op->Payload))
            {
                Dragged->bRotated = !Dragged->bRotated;
                const int32 SX = Dragged->bRotated ? Dragged->SizeY : Dragged->SizeX;
                const int32 SY = Dragged->bRotated ? Dragged->SizeX : Dragged->SizeY;
                if (UCanvasPanelSlot* S = Cast<UCanvasPanelSlot>(Dragged->Slot))
                {
                    S->SetSize(FVector2D(60.f * SX, 60.f * SY));
                }
                // Обновим визуал драга, если он есть
                if (UInventoryItemWidget* DragVisual = Cast<UInventoryItemWidget>(Op->DefaultDragVisual))
                {
                    DragVisual->bRotated = Dragged->bRotated;
                    DragVisual->UpdateVisualSize(FVector2D(60.f, 60.f));
                }
                return FReply::Handled();
            }
        }
        // 2) Иначе, если есть наведённый предмет — вращаем его
        if (HoverItem && HoverItem->ItemData && HoverItem->ItemData->bRotatable)
        {
            HoverItem->bRotated = !HoverItem->bRotated;
            const int32 SX = HoverItem->bRotated ? HoverItem->SizeY : HoverItem->SizeX;
            const int32 SY = HoverItem->bRotated ? HoverItem->SizeX : HoverItem->SizeY;
            if (UCanvasPanelSlot* S = Cast<UCanvasPanelSlot>(HoverItem->Slot))
            {
                S->SetSize(FVector2D(60.f * SX, 60.f * SY));
            }
            else
            {
                HoverItem->UpdateVisualSize(FVector2D(60.f, 60.f));
            }
            return FReply::Handled();
        }
    }
    return FReply::Unhandled();
}

void UInventoryWidget::InitializeOccupiedCells()
{
    // Clear the occupied cells array since backpack grid is removed
    OccupiedCells.Empty();
}

bool UInventoryWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    if (!InOperation) return false;
    UInventoryItemWidget* DraggedWidget = Cast<UInventoryItemWidget>(InOperation->Payload);
    if (!DraggedWidget || !DraggedWidget->ItemData) return false;

    const FVector2D ScreenPos = InDragDropEvent.GetScreenSpacePosition();
    UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetTree ? WidgetTree->RootWidget : nullptr);
    if (!RootCanvas) return false;

    // Дроп мода на превью брони в окне модификации
    if (ArmorModWindowBorder && ArmorModWindowBorder->GetVisibility() == ESlateVisibility::Visible &&
        ArmorModImage && ArmorModImage->GetCachedGeometry().IsUnderLocation(ScreenPos) &&
        OpenedArmorForMods && ArmorModPreviewActor)
    {
        if (UEquipModBaseItemData* ModData = Cast<UEquipModBaseItemData>(DraggedWidget->ItemData))
        {
            const FGeometry& ImgGeo = ArmorModImage->GetCachedGeometry();
            FVector2D LocalPos = ImgGeo.AbsoluteToLocal(ScreenPos);
            FVector2D WidgetSize = ImgGeo.GetLocalSize();
            int32 Side = 0, CellX = 0, CellY = 0;
            if (ArmorModPreviewActor->HitTestGridFromWidgetPosition(LocalPos, WidgetSize, Side, CellX, CellY))
            {
                FIntPoint Foot = UEquippableItemData::GetModFootprint(ModData);
                if (OpenedArmorForMods->CanInstallModAt(Side, CellX, CellY, Foot))
                {
                    TryInstallModOnArmor(ModData, Side, CellX, CellY);
                    return true;
                }
            }
            ClearArmorModHighlight();
        }
    }

    // Находим зарегистрированную область грида в корневых координатах (для размера/кол-ва ячеек)
    const FVector2D RootLocal = RootCanvas->GetCachedGeometry().AbsoluteToLocal(ScreenPos);

    // Дроп на ИКОНКУ EquipModBase: перемещаем предмет во внутреннее хранилище модуля (если оно есть),
    // без наслоения виджетов и без необходимости открывать окно.
    {
        UInventoryItemWidget* TargetItem = nullptr;
        if (HoverItem && HoverItem != DraggedWidget && HoverItem->GetCachedGeometry().IsUnderLocation(ScreenPos))
        {
            TargetItem = HoverItem;
        }
        if (!TargetItem)
        {
            for (const auto& Pair : ItemToWidget)
            {
                UInventoryItemWidget* W = Pair.Value.Get();
                if (!W || W == DraggedWidget) continue;
                if (W->GetCachedGeometry().IsUnderLocation(ScreenPos))
                {
                    TargetItem = W;
                    break;
                }
            }
        }

        if (TargetItem)
        {
            UEquipModBaseItemData* TargetMod = Cast<UEquipModBaseItemData>(TargetItem->ItemData);
            if (TargetMod && TargetMod->bHasAdditionalStorage)
            {
                // Нельзя класть модуль в самого себя
                if (DraggedWidget->ItemData == TargetMod)
                {
                    return false;
                }

                auto FindPlacementInMod = [&](bool bRot, int32& OutGrid, int32& OutX, int32& OutY) -> bool
                {
                    const int32 SX = bRot ? DraggedWidget->SizeY : DraggedWidget->SizeX;
                    const int32 SY = bRot ? DraggedWidget->SizeX : DraggedWidget->SizeY;

                    const int32 MaxGrids = (TargetMod->AdditionalGrids.Num() > 0) ? TargetMod->AdditionalGrids.Num() : 1;
                    for (int32 GridIdx = 0; GridIdx < MaxGrids; ++GridIdx)
                    {
                        const FIntPoint GridSize = EquipModMiniGridSizeByIndex(TargetMod, GridIdx);
                        for (int32 Y = 0; Y <= GridSize.Y - SY; ++Y)
                        {
                            for (int32 X = 0; X <= GridSize.X - SX; ++X)
                            {
                                if (IsAreaFreeInEquipModMiniGrid(TargetMod, GridIdx, X, Y, SX, SY, DraggedWidget->ItemData))
                                {
                                    OutGrid = GridIdx;
                                    OutX = X;
                                    OutY = Y;
                                    return true;
                                }
                            }
                        }
                    }
                    return false;
                };

                int32 PlaceGrid = 0;
                int32 PlaceX = 0;
                int32 PlaceY = 0;
                bool bPlaceRot = DraggedWidget->bRotated;

                if (!FindPlacementInMod(bPlaceRot, PlaceGrid, PlaceX, PlaceY))
                {
                    bPlaceRot = !bPlaceRot;
                    if (!FindPlacementInMod(bPlaceRot, PlaceGrid, PlaceX, PlaceY))
                    {
                        return false;
                    }
                }

                // Логика хранения
                if (APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(GetOwningPlayerPawn()))
                {
                    if (UInventoryComponent* InvComp = PlayerChar->InventoryComponent)
                    {
                        InvComp->RemoveFromAnyStorage(DraggedWidget->ItemData);
                    }
                }

                TargetMod->PersistentStorage.AddUnique(DraggedWidget->ItemData);
                TargetMod->StoredGridByItem.Add(DraggedWidget->ItemData, PlaceGrid);
                TargetMod->PersistentGridByItem.Add(DraggedWidget->ItemData, PlaceGrid);
                TargetMod->StoredCellByItem.Add(DraggedWidget->ItemData, FIntPoint(PlaceX, PlaceY));
                TargetMod->PersistentCellByItem.Add(DraggedWidget->ItemData, FIntPoint(PlaceX, PlaceY));
                TargetMod->StoredRotByItem.Add(DraggedWidget->ItemData, bPlaceRot);
                TargetMod->PersistentRotByItem.Add(DraggedWidget->ItemData, bPlaceRot);

                // Визуально: удаляем текущий виджет, чтобы не было наслоения
                RemoveExistingItemWidget(DraggedWidget->ItemData);
                DraggedWidget->RemoveFromParent();
                ItemToWidget.Remove(DraggedWidget->ItemData);

                // Перерисовываем окно мода (если открыто) и гриды модов брони.
                if (OpenedEquipMod == TargetMod)
                {
                    UpdateEquipModStorageGrid();
                }
                UpdateArmorModGrids();
                return true;
            }
        }
    }
    
    // Проверяем, не перетаскиваем ли экипированный предмет из слота экипировки
    bool bIsEquippedItemDragged = false;
    if (UEquippableItemData* EquippedItem = Cast<UEquippableItemData>(DraggedWidget->ItemData))
    {
        if (EquippedItem->bIsEquipped)
        {
            bIsEquippedItemDragged = true;
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange, 
                    FString::Printf(TEXT("🎯 Dragging equipped item from slot: %d"), (int32)EquippedItem->EquipmentSlot));
            }
        }
    }
    
    // Сначала проверяем, не дропаем ли на слоты экипировки (левая панель).
    // ВАЖНО: НЕ завязываемся на EquipmentPanelRef.IsUnderLocation(), т.к. после верстки слот может визуально находиться
    // вне геометрии панели и тогда экипировка никогда не сработает.
    if (VestSlotRef && VestSlotRef->GetCachedGeometry().IsUnderLocation(ScreenPos))
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("🎯 Drop detected on Vest Slot"));
        }

        // Дроп на слот жилета
        if (UEquippableItemData* VestItem = Cast<UEquippableItemData>(DraggedWidget->ItemData))
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green,
                    FString::Printf(TEXT("🎯 VestItem cast successful, EquipmentSlot: %d"), (int32)VestItem->EquipmentSlot));
            }

            if (VestItem->EquipmentSlot == Vest)
            {
                if (APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(GetOwningPlayerPawn()))
                {
                    if (UInventoryComponent* InvComp = PlayerChar->InventoryComponent)
                    {
                        // Если слот жилета занят — сначала снимаем текущий жилет (обычно в хранилище рюкзака/инвентарь),
                        // иначе EquipItemFromInventory вернёт false из-за занятого слота.
                        if (InvComp->GetEquippedItem(Vest) != nullptr)
                        {
                            const bool bUnequippedOld = InvComp->UnequipItemToInventory(Vest, false);
                            if (!bUnequippedOld)
                            {
                                if (GEngine)
                                {
                                    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("❌ Failed to unequip current vest before equipping new one"));
                                }
                                return false;
                            }
                        }

                        // Удаляем из текущего места хранения
                        InvComp->RemoveFromAnyStorage(DraggedWidget->ItemData);
                        // EquipItemFromInventory требует, чтобы предмет был в BackpackItems.
                        // При перетаскивании из хранилища рюкзака/карманов предмет там отсутствует — добавляем временно.
                        InvComp->BackpackItems.AddUnique(VestItem);
                        // Экипируем
                        if (InvComp->EquipItemFromInventory(VestItem))
                        {
                            DraggedWidget->SetTint(FLinearColor(1.f, 1.f, 1.f, 1.f));
                            UpdateEquipmentSlots();
                            UpdateBackpackStorageGrid();
                            UpdateVestGrid();
                            RefreshInventoryUI();
                            return true;
                        }
                        if (GEngine)
                        {
                            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("❌ Failed to equip vest!"));
                        }
                    }
                }
            }
        }
        return false;
    }

    if (HelmetSlotRef && HelmetSlotRef->GetCachedGeometry().IsUnderLocation(ScreenPos))
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("🎯 Drop detected on Helmet Slot"));
        }

        if (UEquippableItemData* HelmetItem = Cast<UEquippableItemData>(DraggedWidget->ItemData))
        {
            if (HelmetItem->EquipmentSlot == Helmet)
            {
                if (APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(GetOwningPlayerPawn()))
                {
                    if (UInventoryComponent* InvComp = PlayerChar->InventoryComponent)
                    {
                        // Если слот головы занят — снимаем текущий шлем (в инвентарь)
                        if (InvComp->GetEquippedItem(Helmet) != nullptr)
                        {
                            const bool bUnequippedOld = InvComp->UnequipItemToInventory(Helmet, false);
                            if (!bUnequippedOld)
                            {
                                if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("❌ Failed to unequip current helmet before equipping new one")); }
                                return false;
                            }
                        }

                        InvComp->RemoveFromAnyStorage(DraggedWidget->ItemData);
                        InvComp->BackpackItems.AddUnique(HelmetItem);

                        if (InvComp->EquipItemFromInventory(HelmetItem))
                        {
                            DraggedWidget->SetTint(FLinearColor(1.f, 1.f, 1.f, 1.f));
                            UpdateEquipmentSlots();
                            UpdateBackpackStorageGrid();
                            UpdateVestGrid();
                            RefreshInventoryUI();
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }

    if (ArmorSlotRef && ArmorSlotRef->GetCachedGeometry().IsUnderLocation(ScreenPos))
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("🎯 Drop detected on Armor Slot"));
        }

        if (UEquippableItemData* ArmorItem = Cast<UEquippableItemData>(DraggedWidget->ItemData))
        {
            if (ArmorItem->EquipmentSlot == Armor)
            {
                if (APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(GetOwningPlayerPawn()))
                {
                    if (UInventoryComponent* InvComp = PlayerChar->InventoryComponent)
                    {
                        // Если слот бронежилета занят — снимаем текущий бронежилет (в инвентарь)
                        if (InvComp->GetEquippedItem(Armor) != nullptr)
                        {
                            const bool bUnequippedOld = InvComp->UnequipItemToInventory(Armor, false);
                            if (!bUnequippedOld)
                            {
                                if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("❌ Failed to unequip current armor before equipping new one")); }
                                return false;
                            }
                        }

                        InvComp->RemoveFromAnyStorage(DraggedWidget->ItemData);
                        InvComp->BackpackItems.AddUnique(ArmorItem);

                        if (InvComp->EquipItemFromInventory(ArmorItem))
                        {
                            DraggedWidget->SetTint(FLinearColor(1.f, 1.f, 1.f, 1.f));
                            UpdateEquipmentSlots();
                            UpdateBackpackStorageGrid();
                            UpdateVestGrid();
                            RefreshInventoryUI();
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }

    if (BackpackSlotRef && BackpackSlotRef->GetCachedGeometry().IsUnderLocation(ScreenPos))
    {
        // Дроп на слот рюкзака
        if (UEquippableItemData* BackpackItem = Cast<UEquippableItemData>(DraggedWidget->ItemData))
        {
            if (BackpackItem->EquipmentSlot == Backpack)
            {
                if (APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(GetOwningPlayerPawn()))
                {
                    if (UInventoryComponent* InvComp = PlayerChar->InventoryComponent)
                    {
                        // Удаляем из текущего места хранения
                        InvComp->RemoveFromAnyStorage(DraggedWidget->ItemData);
                        // См. комментарий выше: для предметов из внутренних хранилищ нужно обеспечить наличие в BackpackItems
                        InvComp->BackpackItems.AddUnique(BackpackItem);
                        // Экипируем
                        if (InvComp->EquipItemFromInventory(BackpackItem))
                        {
                            DraggedWidget->SetTint(FLinearColor(1.f, 1.f, 1.f, 1.f));
                            UpdateEquipmentSlots();
                            UpdateBackpackStorageGrid();
                            UpdateVestGrid();
                            RefreshInventoryUI();
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }
    
    int32 GridIdx = FindGridAtPoint(RootLocal);
    if (GridIdx == INDEX_NONE)
    {
        // Fallback: allow dropping on pockets/vest by geometry even if not registered
        if (RightPanelRef)
        {
            const int32 Count = RightPanelRef->GetChildrenCount();
            for (int32 i = 0; i < Count; ++i)
            {
                if (UWidget* Child = RightPanelRef->GetChildAt(i))
                {
                    const FName Nm = Child->GetFName();
                    const bool bIsPocket = (Nm == TEXT("карман1") || Nm == TEXT("карман2") || Nm == TEXT("карман3") || Nm == TEXT("карман4"));
                    const bool bIsVestGrid = Nm.ToString().StartsWith(TEXT("VestGrid_"));
                    const bool bIsArmorModGrid = Nm.ToString().StartsWith(TEXT("ArmorModGrid_"));
                    if ((bIsPocket || bIsVestGrid || bIsArmorModGrid) &&
                        Child->GetCachedGeometry().IsUnderLocation(ScreenPos))
                    {
                        // Проверка по реальному размеру виджета-ячейки (геометрия)
                        const FGeometry ChildGeom = Child->GetCachedGeometry();
                        const FVector2D TargetSize = ChildGeom.GetLocalSize();
                        auto CanFitWithRotate = [&](bool& bOutRotate) -> bool
                        {
                            // Сначала проверяем текущую ориентацию
                            const int32 CurX = DraggedWidget->bRotated ? DraggedWidget->SizeY : DraggedWidget->SizeX;
                            const int32 CurY = DraggedWidget->bRotated ? DraggedWidget->SizeX : DraggedWidget->SizeY;
                            const FVector2D PixCur(60.f * CurX, 60.f * CurY);
                            if (PixCur.X <= TargetSize.X && PixCur.Y <= TargetSize.Y)
                            { bOutRotate = false; return true; }
                            // Затем пробуем повернуть
                            const int32 RotX = DraggedWidget->bRotated ? DraggedWidget->SizeX : DraggedWidget->SizeY;
                            const int32 RotY = DraggedWidget->bRotated ? DraggedWidget->SizeY : DraggedWidget->SizeX;
                            const FVector2D PixRot(60.f * RotX, 60.f * RotY);
                            if (PixRot.X <= TargetSize.X && PixRot.Y <= TargetSize.Y)
                            { bOutRotate = true; return true; }
        return false;
                        };

                        bool bRotate = false;
                        if (!CanFitWithRotate(bRotate))
                        {
                            return false; // физически больше слота — отклоняем
                        }

                        if (APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(GetOwningPlayerPawn()))
                        {
                            if (UInventoryComponent* InvComp = PlayerChar->InventoryComponent)
                            {
                                if (bIsPocket)
                                {
                                    int32 PocketIndex = 1;
                                    if (Nm == TEXT("карман1")) PocketIndex = 1;
                                    else if (Nm == TEXT("карман2")) PocketIndex = 2;
                                    else if (Nm == TEXT("карман3")) PocketIndex = 3;
                                    else if (Nm == TEXT("карман4")) PocketIndex = 4;

                                    // Определяем локальную ячейку внутри мини‑грида кармана
                                    const float CellSizePx = 60.f;
                                    const FVector2D LocalInChild = ChildGeom.AbsoluteToLocal(ScreenPos);
                                    const int32 CellsX = FMath::Max(1, FMath::RoundToInt(TargetSize.X / CellSizePx));
                                    const int32 CellsY = FMath::Max(1, FMath::RoundToInt(TargetSize.Y / CellSizePx));
                                    const int32 LocalCellX = FMath::Clamp((int32)FMath::FloorToInt(LocalInChild.X / CellSizePx), 0, CellsX - 1);
                                    const int32 LocalCellY = FMath::Clamp((int32)FMath::FloorToInt(LocalInChild.Y / CellSizePx), 0, CellsY - 1);

                                    // Применяем автоповорот, если нужно (для карманов сейчас только 1x1, но логика единообразная)
                                    if (bRotate)
                                    {
                                        DraggedWidget->bRotated = !DraggedWidget->bRotated;
                                        DraggedWidget->UpdateVisualSize(FVector2D(60.f, 60.f));
                                    }

                                    const bool bMoved = InvComp->MoveItemToPocketAt(PocketIndex, LocalCellX, LocalCellY, DraggedWidget->bRotated, DraggedWidget->ItemData);
                                    if (!bMoved)
                                    {
                                        return false;
                                    }

                                    // Убираем виджет из старого контейнера и перестраиваем карманы,
                                    // чтобы не было наслоения в одном CanvasPanel.
                                    DraggedWidget->RemoveFromParent();
                                    UpdatePocketsGrid();
                                    UpdateBackpackStorageGrid();
                                    UpdateVestGrid();
                                    RefreshOpenedEquipModStorage();
                                    return true;
                                }
                                else if (bIsVestGrid)
                                {
                                    // Аналогичная логика дропа в жилет, как в основной ветке DropArea.Name.StartsWith("жилет"):
                                    // кладём предмет именно в ячейку под курсором или отменяем дроп.
                                    if (UEquippableItemData* EquippedVest = InvComp->GetEquippedItem(Vest))
                                    {
                                        // Определяем индекс мини‑грида жилета максимально надёжно:
                                        // 1) по реальному указателю в массиве VestGrids
                                        // 2) фолбэк: по имени "VestGrid_жилетN"
                                        int32 VestLocalIdx = INDEX_NONE;
                                        if (UCanvasPanel* AsCanvas = Cast<UCanvasPanel>(Child))
                                        {
                                            VestLocalIdx = VestGrids.Find(AsCanvas);
                                        }
                                        if (VestLocalIdx == INDEX_NONE)
                                        {
                                            VestLocalIdx = VestGridIndexFromVestGridWidgetName(Child->GetName());
                                        }
                                        if (VestLocalIdx == INDEX_NONE) VestLocalIdx = 0;

                                        // Локальная ячейка внутри мини‑грида (динамический размер)
                                        const FIntPoint MiniGridSize = VestMiniGridSizeByIndex(EquippedVest, VestLocalIdx);
                                        const int32 CellsX = FMath::Max(1, MiniGridSize.X);
                                        const int32 CellsY = FMath::Max(1, MiniGridSize.Y);
                                        const FVector2D CellSz(TargetSize.X / CellsX, TargetSize.Y / CellsY);
                                        const FVector2D LocalInChild = ChildGeom.AbsoluteToLocal(ScreenPos);
                                        const int32 LocalCellX = FMath::Clamp((int32)FMath::FloorToInt(LocalInChild.X / CellSz.X), 0, CellsX - 1);
                                        const int32 LocalCellY = FMath::Clamp((int32)FMath::FloorToInt(LocalInChild.Y / CellSz.Y), 0, CellsY - 1);

                                        // Применяем автоповорот для данных (если UI сказал, что нужно повернуть)
                                        if (bRotate)
                                        {
                                            DraggedWidget->bRotated = !DraggedWidget->bRotated;
                                            DraggedWidget->UpdateVisualSize(FVector2D(60.f, 60.f));
                                        }

                                        const FIntPoint GridSize = MiniGridSize;
                                        const int32 SizeX = DraggedWidget->bRotated ? DraggedWidget->SizeY : DraggedWidget->SizeX;
                                        const int32 SizeY = DraggedWidget->bRotated ? DraggedWidget->SizeX : DraggedWidget->SizeY;

                                        // Проверка границ для предмета внутри конкретного мини‑грида
                                        const int32 MaxGrids = (EquippedVest->AdditionalGrids.Num() > 0) ? EquippedVest->AdditionalGrids.Num() : 6;
                                        if (VestLocalIdx < 0 || VestLocalIdx >= MaxGrids) return false;
                                        if (LocalCellX + SizeX > GridSize.X || LocalCellY + SizeY > GridSize.Y)
                                        {
                                            return false;
                                        }

                                        // Если область занята — отменяем дроп
                                        if (!IsAreaFreeInVestMiniGrid(EquippedVest, VestLocalIdx, LocalCellX, LocalCellY, SizeX, SizeY, DraggedWidget->ItemData))
                                        {
                                            return false;
                                        }

                                        // Удаляем старый виджет предмета перед перестроением UI
                                        RemoveExistingItemWidget(DraggedWidget->ItemData);
                                        DraggedWidget->RemoveFromParent();
                                        ItemToWidget.Remove(DraggedWidget->ItemData);

                                        // Привязываем предмет к хранилищу жилета
                                        InvComp->MoveItemToVest(DraggedWidget->ItemData);

                                        // Сохраняем положение и поворот
                                        EquippedVest->StoredGridByItem.Add(DraggedWidget->ItemData, VestLocalIdx);
                                        EquippedVest->PersistentGridByItem.Add(DraggedWidget->ItemData, VestLocalIdx);
                                        EquippedVest->StoredCellByItem.Add(DraggedWidget->ItemData, FIntPoint(LocalCellX, LocalCellY));
                                        EquippedVest->PersistentCellByItem.Add(DraggedWidget->ItemData, FIntPoint(LocalCellX, LocalCellY));
                                        EquippedVest->StoredRotByItem.Add(DraggedWidget->ItemData, DraggedWidget->bRotated);
                                        EquippedVest->PersistentRotByItem.Add(DraggedWidget->ItemData, DraggedWidget->bRotated);

                                        // Полностью пересобираем гриды жилета по сохранённым данным, без ручного репарентинга
                                        UpdateVestGrid();
                                        // Сбрасываем подсветку
                                        DraggedWidget->SetTint(FLinearColor(1.f,1.f,1.f,1.f));
                                        return true;
                                    }
                                    return false;
                                }
                                else if (bIsArmorModGrid)
                                {
                                    // Дроп в конкретный мини‑грид хранилища мода брони
                                    int32 ModGridIdx = ArmorModGrids.Find(Cast<UCanvasPanel>(Child));
                                    if (!ArmorModGrids.IsValidIndex(ModGridIdx) ||
                                        !ArmorModGridModData.IsValidIndex(ModGridIdx) ||
                                        !ArmorModGridMiniIndices.IsValidIndex(ModGridIdx))
                                    {
                                        return false;
                                    }

                                    UEquipModBaseItemData* ModData = ArmorModGridModData[ModGridIdx];
                                    const int32 MiniIndex = ArmorModGridMiniIndices[ModGridIdx];
                                    if (!ModData || !ModData->bHasAdditionalStorage)
                                    {
                                        return false;
                                    }

                                    const FIntPoint GridSize = EquipModMiniGridSizeByIndex(ModData, MiniIndex);
                                    const int32 CellsX = FMath::Max(1, GridSize.X);
                                    const int32 CellsY = FMath::Max(1, GridSize.Y);
                                    const FVector2D CellSz(TargetSize.X / CellsX, TargetSize.Y / CellsY);
                                    const FVector2D LocalInChild = ChildGeom.AbsoluteToLocal(ScreenPos);
                                    const int32 LocalCellX = FMath::Clamp((int32)FMath::FloorToInt(LocalInChild.X / CellSz.X), 0, CellsX - 1);
                                    const int32 LocalCellY = FMath::Clamp((int32)FMath::FloorToInt(LocalInChild.Y / CellSz.Y), 0, CellsY - 1);

                                    if (bRotate)
                                    {
                                        DraggedWidget->bRotated = !DraggedWidget->bRotated;
                                        DraggedWidget->UpdateVisualSize(FVector2D(60.f, 60.f));
                                    }

                                    const int32 SizeX = DraggedWidget->bRotated ? DraggedWidget->SizeY : DraggedWidget->SizeX;
                                    const int32 SizeY = DraggedWidget->bRotated ? DraggedWidget->SizeX : DraggedWidget->SizeY;
                                    if (LocalCellX + SizeX > GridSize.X || LocalCellY + SizeY > GridSize.Y)
                                    {
                                        return false;
                                    }
                                    if (!IsAreaFreeInEquipModMiniGrid(ModData, MiniIndex, LocalCellX, LocalCellY, SizeX, SizeY, DraggedWidget->ItemData))
                                    {
                                        return false;
                                    }

                                    RemoveExistingItemWidget(DraggedWidget->ItemData);
                                    DraggedWidget->RemoveFromParent();
                                    ItemToWidget.Remove(DraggedWidget->ItemData);
                                    InvComp->RemoveFromAnyStorage(DraggedWidget->ItemData);

                                    ModData->PersistentStorage.AddUnique(DraggedWidget->ItemData);
                                    ModData->StoredGridByItem.Add(DraggedWidget->ItemData, MiniIndex);
                                    ModData->PersistentGridByItem.Add(DraggedWidget->ItemData, MiniIndex);
                                    ModData->StoredCellByItem.Add(DraggedWidget->ItemData, FIntPoint(LocalCellX, LocalCellY));
                                    ModData->PersistentCellByItem.Add(DraggedWidget->ItemData, FIntPoint(LocalCellX, LocalCellY));
                                    ModData->StoredRotByItem.Add(DraggedWidget->ItemData, DraggedWidget->bRotated);
                                    ModData->PersistentRotByItem.Add(DraggedWidget->ItemData, DraggedWidget->bRotated);

                                    DraggedWidget->SetTint(FLinearColor(1.f, 1.f, 1.f, 1.f));
                                    UpdateArmorModGrids();
                                    return true;
                                }

                                // Reparent visual to target grid (карманы и прочее, кроме жилета)
                                if (UCanvasPanel* TargetGrid = Cast<UCanvasPanel>(Child))
                                {
                                    RemoveExistingItemWidget(DraggedWidget->ItemData);
                                    // Учитываем автоповорот для визуала (вращаем только если нужно)
                                    if (bRotate)
                                    {
                                        DraggedWidget->bRotated = !DraggedWidget->bRotated;
                                    }
                                    DraggedWidget->UpdateVisualSize(FVector2D(60.f, 60.f));
                                    DraggedWidget->RemoveFromParent();
                                    if (UCanvasPanelSlot* NewSlot = Cast<UCanvasPanelSlot>(TargetGrid->AddChildToCanvas(DraggedWidget)))
                                    {
                                        NewSlot->SetAnchors(FAnchors(0.f,0.f,0.f,0.f));
                                        NewSlot->SetAlignment(FVector2D(0.f,0.f));
                                        NewSlot->SetPosition(FVector2D::ZeroVector);
                                        // Размер под итоговую ориентацию
                                        const int32 vx = DraggedWidget->bRotated ? DraggedWidget->SizeY : DraggedWidget->SizeX;
                                        const int32 vy = DraggedWidget->bRotated ? DraggedWidget->SizeX : DraggedWidget->SizeY;
                                        NewSlot->SetSize(FVector2D(60.f * vx, 60.f * vy));
                                    }
                                    DraggedWidget->SetTint(FLinearColor(1.f,1.f,1.f,1.f));
                                }
                                return true;
                            }
                        }
                    }
                }
            }
        }
                    return false;
                }
    const FGridArea& A = GridAreas[GridIdx];
    const FVector2D CellSize = FVector2D(A.Size.X / A.CellsX, A.Size.Y / A.CellsY);

    // Позиция курсора относительно выбранной области
    const FVector2D GridLocal = RootLocal - A.Position;
    int32 CellX = FMath::Clamp((int32)FMath::FloorToInt(GridLocal.X / CellSize.X), 0, A.CellsX - 1);
    int32 CellY = FMath::Clamp((int32)FMath::FloorToInt(GridLocal.Y / CellSize.Y), 0, A.CellsY - 1);

    const FVector2D Snapped(FMath::FloorToFloat(CellX * CellSize.X), FMath::FloorToFloat(CellY * CellSize.Y));

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(987654, 2.0f, FColor::Cyan,
            FString::Printf(TEXT("Drop over grid '%s' cells %dx%d -> Cell(%d,%d)"), *A.Name, A.CellsX, A.CellsY, CellX, CellY));
    }

    // Определяем зону дропа
    const FGridArea& DropArea = GridAreas[GridIdx];
    // Если дроп в рюкзак — репарентим в канву рюкзака, двигаем и сохраняем позицию
    if (DropArea.Name.Contains(TEXT("рюкзак")))
    {
        // Новый формат: дроп в конкретный мини‑грид "рюкзакN"
        const int32 BackpackGridIdx = BackpackGridIndexFromAreaName(DropArea.Name);
        UCanvasPanel* BackpackCanvas = BackpackGrids.IsValidIndex(BackpackGridIdx) ? BackpackGrids[BackpackGridIdx] : nullptr;
        if (!BackpackCanvas) return false;

        // Проверка доступности области и автоповорот
        APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(GetOwningPlayerPawn());
        UInventoryComponent* InvComp = PlayerChar ? PlayerChar->InventoryComponent : nullptr;
        UEquippableItemData* EquippedBackpack = InvComp ? InvComp->GetEquippedItem(Backpack) : nullptr;
        if (!EquippedBackpack) return false;

        // Запрещаем класть рюкзак в самого себя
        if (DraggedWidget->ItemData == EquippedBackpack)
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("❌ Нельзя поместить рюкзак в самого себя"));
            }
                    return false;
                }

        auto TryPlace = [&](bool bRotate) -> bool
        {
            const int32 SX = bRotate ? DraggedWidget->SizeY : DraggedWidget->SizeX;
            const int32 SY = bRotate ? DraggedWidget->SizeX : DraggedWidget->SizeY;
            // Границы по пикселям + контроль пересечений по StoredCellByItem
            const FVector2D ItemPix(CellSize.X * SX, CellSize.Y * SY);
            const bool bFitsByPixels = (Snapped.X + ItemPix.X <= A.Size.X && Snapped.Y + ItemPix.Y <= A.Size.Y);
            if (!bFitsByPixels) return false;
            if (!IsAreaFreeInBackpackMiniGrid(EquippedBackpack, BackpackGridIdx, CellX, CellY, SX, SY, DraggedWidget->ItemData))
            {
                    return false;
                }
            // Визуально
            DraggedWidget->bRotated = bRotate;
            DraggedWidget->UpdateVisualSize(FVector2D(60.f, 60.f));
            RemoveExistingItemWidget(DraggedWidget->ItemData);
            DraggedWidget->RemoveFromParent();
            if (UCanvasPanelSlot* NewSlot = Cast<UCanvasPanelSlot>(BackpackCanvas->AddChildToCanvas(DraggedWidget)))
            {
                NewSlot->SetAnchors(FAnchors(0.f,0.f,0.f,0.f));
                NewSlot->SetAlignment(FVector2D(0.f,0.f));
                NewSlot->SetPosition(Snapped);
                NewSlot->SetSize(FVector2D(CellSize.X * SX, CellSize.Y * SY));
                UpsertPlacement(DraggedWidget, GridIdx, CellX, CellY);
            }
            // Логика
            InvComp->RemoveFromAnyStorage(DraggedWidget->ItemData);
            InvComp->AddToEquipmentStorage(EquippedBackpack, DraggedWidget->ItemData);
            EquippedBackpack->StoredGridByItem.Add(DraggedWidget->ItemData, BackpackGridIdx);
            EquippedBackpack->PersistentGridByItem.Add(DraggedWidget->ItemData, BackpackGridIdx);
            EquippedBackpack->StoredCellByItem.Add(DraggedWidget->ItemData, FIntPoint(CellX, CellY));
            EquippedBackpack->PersistentCellByItem.Add(DraggedWidget->ItemData, FIntPoint(CellX, CellY));
            EquippedBackpack->StoredRotByItem.Add(DraggedWidget->ItemData, DraggedWidget->bRotated);
            EquippedBackpack->PersistentRotByItem.Add(DraggedWidget->ItemData, DraggedWidget->bRotated);
            return true;
        };

        // Сначала как есть, потом автоповорот
        if (!TryPlace(DraggedWidget->bRotated))
        {
            if (!TryPlace(!DraggedWidget->bRotated))
            {
                return false;
            }
        }
        // Сбрасываем подсветку после успешного дропа
        DraggedWidget->SetTint(FLinearColor(1.f, 1.f, 1.f, 1.f));
        UpdateBackpackStorageGrid();
        return true;
    }

    // Если дроп в открытое хранилище модуля (EquipModBase)
    if (DropArea.Name.StartsWith(TEXT("модуль")))
    {
        if (!OpenedEquipMod || !OpenedEquipMod->bHasAdditionalStorage) return false;

        const int32 ModGridIdx = EquipModGridIndexFromAreaName(DropArea.Name);
        UCanvasPanel* ModCanvas = EquipModGrids.IsValidIndex(ModGridIdx) ? EquipModGrids[ModGridIdx] : nullptr;
        if (!ModCanvas) return false;

        auto TryPlace = [&](bool bRotate) -> bool
        {
            const int32 SX = bRotate ? DraggedWidget->SizeY : DraggedWidget->SizeX;
            const int32 SY = bRotate ? DraggedWidget->SizeX : DraggedWidget->SizeY;
            const FVector2D ItemPix(CellSize.X * SX, CellSize.Y * SY);
            const bool bFitsByPixels = (Snapped.X + ItemPix.X <= A.Size.X && Snapped.Y + ItemPix.Y <= A.Size.Y);
            if (!bFitsByPixels) return false;
            if (!IsAreaFreeInEquipModMiniGrid(OpenedEquipMod, ModGridIdx, CellX, CellY, SX, SY, DraggedWidget->ItemData))
            {
                return false;
            }

            // Визуально
            DraggedWidget->bRotated = bRotate;
            DraggedWidget->UpdateVisualSize(FVector2D(60.f, 60.f));
            RemoveExistingItemWidget(DraggedWidget->ItemData);
            DraggedWidget->RemoveFromParent();
            if (UCanvasPanelSlot* NewSlot = Cast<UCanvasPanelSlot>(ModCanvas->AddChildToCanvas(DraggedWidget)))
            {
                NewSlot->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
                NewSlot->SetAlignment(FVector2D(0.f, 0.f));
                NewSlot->SetPosition(Snapped);
                NewSlot->SetSize(FVector2D(CellSize.X * SX, CellSize.Y * SY));
                UpsertPlacement(DraggedWidget, GridIdx, CellX, CellY);
            }

            // Логика: удаляем из любого места и кладём в PersistentStorage модуля
            if (APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(GetOwningPlayerPawn()))
            {
                if (UInventoryComponent* InvComp = PlayerChar->InventoryComponent)
                {
                    InvComp->RemoveFromAnyStorage(DraggedWidget->ItemData);
                }
            }

            OpenedEquipMod->PersistentStorage.AddUnique(DraggedWidget->ItemData);
            OpenedEquipMod->StoredGridByItem.Add(DraggedWidget->ItemData, ModGridIdx);
            OpenedEquipMod->PersistentGridByItem.Add(DraggedWidget->ItemData, ModGridIdx);
            OpenedEquipMod->StoredCellByItem.Add(DraggedWidget->ItemData, FIntPoint(CellX, CellY));
            OpenedEquipMod->PersistentCellByItem.Add(DraggedWidget->ItemData, FIntPoint(CellX, CellY));
            OpenedEquipMod->StoredRotByItem.Add(DraggedWidget->ItemData, DraggedWidget->bRotated);
            OpenedEquipMod->PersistentRotByItem.Add(DraggedWidget->ItemData, DraggedWidget->bRotated);
            return true;
        };

        if (!TryPlace(DraggedWidget->bRotated))
        {
            if (!TryPlace(!DraggedWidget->bRotated))
            {
                return false;
            }
        }

        DraggedWidget->SetTint(FLinearColor(1.f, 1.f, 1.f, 1.f));
        // Обновляем окно хранилища мода и гриды модов на броне.
        UpdateEquipModStorageGrid();
        UpdateArmorModGrids();
        return true;
    }
    
    if (APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(GetOwningPlayerPawn()))
    {
        if (UInventoryComponent* InvComp = PlayerChar->InventoryComponent)
        {
            if (DropArea.Name.StartsWith(TEXT("жилет")))
            {
                // Автоповорот по аналогии с рюкзаком
                auto ComputeDims = [&](bool bRot)
                {
                    return FVector2D(
                        bRot ? DraggedWidget->SizeY : DraggedWidget->SizeX,
                        bRot ? DraggedWidget->SizeX : DraggedWidget->SizeY
                    );
                };

                FVector2D Dims = ComputeDims(DraggedWidget->bRotated);
                bool bFitsBySection = (Dims.X <= A.CellsX && Dims.Y <= A.CellsY);
                if (!bFitsBySection)
                {
                    // Пробуем автоповорот
                    FVector2D RotDims = ComputeDims(!DraggedWidget->bRotated);
                    if (RotDims.X <= A.CellsX && RotDims.Y <= A.CellsY)
                    {
                        DraggedWidget->bRotated = !DraggedWidget->bRotated;
                        DraggedWidget->UpdateVisualSize(FVector2D(60.f, 60.f));
                        Dims = RotDims;
                        bFitsBySection = true;
                    }
                }
                if (!bFitsBySection)
                {
                    // Предмет физически не влезает в выбранный мини‑грид
                    return false;
                }

                if (UEquippableItemData* EquippedVest = InvComp->GetEquippedItem(Vest))
                {
                    const int32 SizeX = (int32)Dims.X;
                    const int32 SizeY = (int32)Dims.Y;

                    // 1) Вычисляем индекс мини‑грида и локальную ячейку под курсором
                    int32 VestLocalIdx = INDEX_NONE;
                    {
                        FString AreaName = A.Name; // "жилет3"
                        if (AreaName.RemoveFromStart(TEXT("жилет")))
                        {
                            const int32 Parsed = FCString::Atoi(*AreaName);
                            if (Parsed > 0) { VestLocalIdx = Parsed - 1; }
                        }
                    }
                    if (VestLocalIdx == INDEX_NONE)
                    {
                        VestLocalIdx = 0;
                    }

                    const FIntPoint GridSize = VestMiniGridSizeByIndex(EquippedVest, VestLocalIdx);
                    const int32 LocalX = FMath::Clamp(CellX, 0, FMath::Max(0, GridSize.X - 1));
                    const int32 LocalY = FMath::Clamp(CellY, 0, FMath::Max(0, GridSize.Y - 1));

                    // Шаг 1: проверяем, влезает ли предмет, начиная с ячейки под курсором (в рамках мини‑грида)
                    const int32 MaxGrids = (EquippedVest->AdditionalGrids.Num() > 0) ? EquippedVest->AdditionalGrids.Num() : 6;
                    if (VestLocalIdx < 0 || VestLocalIdx >= MaxGrids) return false;
                    if (LocalX + SizeX > GridSize.X || LocalY + SizeY > GridSize.Y)
                    {
                        return false;
                    }

                    // Шаг 2: если область занята — дроп отменяем
                    if (!IsAreaFreeInVestMiniGrid(EquippedVest, VestLocalIdx, LocalX, LocalY, SizeX, SizeY, DraggedWidget->ItemData))
                    {
                        return false;
                    }

                    // Шаг 3: удаляем старый виджет, сохраняем (GridIndex + LocalCell) и обновляем UI
                    RemoveExistingItemWidget(DraggedWidget->ItemData);
                    DraggedWidget->RemoveFromParent();
                    ItemToWidget.Remove(DraggedWidget->ItemData);

                    DestroyVestGrid();
                    InvComp->MoveItemToVest(DraggedWidget->ItemData);

                    EquippedVest->StoredGridByItem.Add(DraggedWidget->ItemData, VestLocalIdx);
                    EquippedVest->PersistentGridByItem.Add(DraggedWidget->ItemData, VestLocalIdx);
                    EquippedVest->StoredCellByItem.Add(DraggedWidget->ItemData, FIntPoint(LocalX, LocalY));
                    EquippedVest->PersistentCellByItem.Add(DraggedWidget->ItemData, FIntPoint(LocalX, LocalY));
                    EquippedVest->StoredRotByItem.Add(DraggedWidget->ItemData, DraggedWidget->bRotated);
                    EquippedVest->PersistentRotByItem.Add(DraggedWidget->ItemData, DraggedWidget->bRotated);

                    if (GEngine)
                    {
                        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green,
                            FString::Printf(TEXT("✅ Vest drop: grid(%d) cell(%d,%d) size(%d,%d)"), VestLocalIdx, LocalX, LocalY, SizeX, SizeY));
                    }

                    // Полное перестроение визуала жилета на основе сохранённых координат
                    UpdateVestGrid();
                }
            }
            else if (DropArea.Name.StartsWith(TEXT("карман")))
            {
                int32 PocketIndex = 1;
                if (DropArea.Name == TEXT("карман1")) PocketIndex = 1;
                else if (DropArea.Name == TEXT("карман2")) PocketIndex = 2;
                else if (DropArea.Name == TEXT("карман3")) PocketIndex = 3;
                else if (DropArea.Name == TEXT("карман4")) PocketIndex = 4;

                // Проверка на размер 1x1 и автоповорот для укладки в 1x1
                int32 FitX = DraggedWidget->bRotated ? DraggedWidget->SizeY : DraggedWidget->SizeX;
                int32 FitY = DraggedWidget->bRotated ? DraggedWidget->SizeX : DraggedWidget->SizeY;
                bool bFits = (FitX <= 1 && FitY <= 1);
                if (!bFits)
                {
                    const bool bCanRotateToFit = (DraggedWidget->SizeY <= 1 && DraggedWidget->SizeX <= 1);
                    if (bCanRotateToFit)
                    {
                        DraggedWidget->bRotated = !DraggedWidget->bRotated;
                        DraggedWidget->UpdateVisualSize(FVector2D(60.f, 60.f));
                        bFits = true;
                    }
                }
                if (!bFits)
                {
                    return false; // предмет больше 1x1 — не кладём в карман
                }

                // Логика хранения: используем конкретную ячейку DropArea.CellX/CellY
                const bool bMoved = InvComp->MoveItemToPocketAt(PocketIndex, CellX, CellY, DraggedWidget->bRotated, DraggedWidget->ItemData);
                if (!bMoved)
                {
                    // Ячейка занята или предмет не подходит
                    return false;
                }

                // Визуально/UI: убираем виджет из старого места и пересобираем карманы.
                DraggedWidget->RemoveFromParent();
                DraggedWidget->SetTint(FLinearColor(1.f, 1.f, 1.f, 1.f));

                UpdatePocketsGrid();
                UpdateBackpackStorageGrid();
                UpdateVestGrid();
                RefreshOpenedEquipModStorage();
                return true;
            }
        }
    }
    
    // Если перетащили экипированный предмет не в слот экипировки, очищаем соответствующие гриды
    if (bIsEquippedItemDragged)
    {
        if (UEquippableItemData* EquippedItem = Cast<UEquippableItemData>(DraggedWidget->ItemData))
        {
            if (EquippedItem->EquipmentSlot == Vest)
            {
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("🗑️ Vest unequipped via drag-and-drop - clearing vest grid"));
                }
                
                // Принудительно очищаем все виджеты предметов
                if (RightPanelRef)
                {
                    TArray<UWidget*> ChildrenToRemove;
                    const int32 Count = RightPanelRef->GetChildrenCount();
                    for (int32 i = 0; i < Count; ++i)
                    {
                        if (UWidget* Child = RightPanelRef->GetChildAt(i))
                        {
                            if (UInventoryItemWidget* ItemWidget = Cast<UInventoryItemWidget>(Child))
                            {
                                if (ItemWidget->ItemData == DraggedWidget->ItemData)
                                {
                                    ChildrenToRemove.Add(Child);
                                }
                            }
                        }
                    }
                    
                    for (UWidget* Child : ChildrenToRemove)
                    {
                        Child->RemoveFromParent();
                    }
                }
                
                ForceClearVestGrids();
                UpdateEquipmentSlots();
                UpdateBackpackStorageGrid();
                RefreshInventoryUI();
            }
            else if (EquippedItem->EquipmentSlot == Backpack)
            {
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("🗑️ Backpack unequipped via drag-and-drop - clearing backpack grid"));
                }
                UpdateEquipmentSlots();
                UpdateBackpackStorageGrid();
                RefreshInventoryUI();
            }
            else if (EquippedItem->EquipmentSlot == Armor)
            {
                // Бронежилет не имеет гридов — просто обновляем слоты/интерфейс
                UpdateEquipmentSlots();
                RefreshInventoryUI();
            }
            else if (EquippedItem->EquipmentSlot == Helmet)
            {
                // Шлем не имеет гридов — просто обновляем слоты/интерфейс
                UpdateEquipmentSlots();
                RefreshInventoryUI();
            }
        }
    }
    
    return true;
}
    
bool UInventoryWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    const FVector2D ScreenPos = InDragDropEvent.GetScreenSpacePosition();
    UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetTree ? WidgetTree->RootWidget : nullptr);
    if (!RootCanvas) return false;

    // Перетаскивание мода над превью брони — подсветка ячеек (зелёные/красные)
    if (ArmorModWindowBorder && ArmorModWindowBorder->GetVisibility() == ESlateVisibility::Visible &&
        OpenedArmorForMods && ArmorModPreviewActor && InOperation)
    {
        UInventoryItemWidget* DraggedWidget = Cast<UInventoryItemWidget>(InOperation->Payload);
        UEquipModBaseItemData* ModData = DraggedWidget && DraggedWidget->ItemData ? Cast<UEquipModBaseItemData>(DraggedWidget->ItemData) : nullptr;
        if (ModData)
        {
            if (ArmorModImage && ArmorModImage->GetCachedGeometry().IsUnderLocation(ScreenPos))
            {
                const FGeometry& ImgGeo = ArmorModImage->GetCachedGeometry();
                FVector2D LocalPos = ImgGeo.AbsoluteToLocal(ScreenPos);
                FVector2D WidgetSize = ImgGeo.GetLocalSize();
                int32 Side = 0, CellX = 0, CellY = 0;
                if (ArmorModPreviewActor->HitTestGridFromWidgetPosition(LocalPos, WidgetSize, Side, CellX, CellY))
                {
                    FIntPoint Foot = UEquippableItemData::GetModFootprint(ModData);
                    const bool bValid = OpenedArmorForMods->CanInstallModAt(Side, CellX, CellY, Foot);
                    TArray<AArmorModPreviewActor::FHighlightCell> Cells;
                    for (int32 dy = 0; dy < Foot.Y; ++dy)
                        for (int32 dx = 0; dx < Foot.X; ++dx)
                            Cells.Add(AArmorModPreviewActor::FHighlightCell{ Side, CellX + dx, CellY + dy, bValid });
                    ArmorModPreviewActor->SetHighlightCells(Cells);
                }
                else
                    ClearArmorModHighlight();
            }
            else
                ClearArmorModHighlight();
            return true;
        }
    }

    const FVector2D RootLocal = RootCanvas->GetCachedGeometry().AbsoluteToLocal(ScreenPos);
    const int32 GridIdx = FindGridAtPoint(RootLocal);
    if (GridIdx != INDEX_NONE) return true;
    
    // Проверяем, не наводимся ли на слоты экипировки (левая панель).
    // Аналогично NativeOnDrop: не завязываемся на EquipmentPanelRef.IsUnderLocation().
    if (VestSlotRef && VestSlotRef->GetCachedGeometry().IsUnderLocation(ScreenPos))
    {
        if (UInventoryItemWidget* DraggedWidget = Cast<UInventoryItemWidget>(InOperation ? InOperation->Payload : nullptr))
        {
            if (UEquippableItemData* VestItem = Cast<UEquippableItemData>(DraggedWidget->ItemData))
            {
                if (VestItem->EquipmentSlot == Vest)
                {
                    return true;
                }
            }
        }
    }

    if (HelmetSlotRef && HelmetSlotRef->GetCachedGeometry().IsUnderLocation(ScreenPos))
    {
        if (UInventoryItemWidget* DraggedWidget = Cast<UInventoryItemWidget>(InOperation ? InOperation->Payload : nullptr))
        {
            if (UEquippableItemData* HelmetItem = Cast<UEquippableItemData>(DraggedWidget->ItemData))
            {
                if (HelmetItem->EquipmentSlot == Helmet)
                {
                    return true;
                }
            }
        }
    }

    if (ArmorSlotRef && ArmorSlotRef->GetCachedGeometry().IsUnderLocation(ScreenPos))
    {
        if (UInventoryItemWidget* DraggedWidget = Cast<UInventoryItemWidget>(InOperation ? InOperation->Payload : nullptr))
        {
            if (UEquippableItemData* ArmorItem = Cast<UEquippableItemData>(DraggedWidget->ItemData))
            {
                if (ArmorItem->EquipmentSlot == Armor)
                {
                    return true;
                }
            }
        }
    }

    if (BackpackSlotRef && BackpackSlotRef->GetCachedGeometry().IsUnderLocation(ScreenPos))
    {
        if (UInventoryItemWidget* DraggedWidget = Cast<UInventoryItemWidget>(InOperation ? InOperation->Payload : nullptr))
        {
            if (UEquippableItemData* BackpackItem = Cast<UEquippableItemData>(DraggedWidget->ItemData))
            {
                if (BackpackItem->EquipmentSlot == Backpack)
                {
                    return true;
                }
            }
        }
    }
    // Фолбэк: если зона не найдена среди зарегистрированных — принимаем, если курсор над карманом/жилетом визуально
    if (RightPanelRef)
    {
        const int32 Count = RightPanelRef->GetChildrenCount();
        for (int32 i = 0; i < Count; ++i)
        {
            if (UWidget* Child = RightPanelRef->GetChildAt(i))
            {
                const FName Nm = Child->GetFName();
                const bool bIsPocket = (Nm == TEXT("карман1") || Nm == TEXT("карман2") || Nm == TEXT("карман3") || Nm == TEXT("карман4"));
                const bool bIsVestGrid = Nm.ToString().StartsWith(TEXT("VestGrid_"));
                if ((bIsPocket || bIsVestGrid) &&
                    Child->GetCachedGeometry().IsUnderLocation(InDragDropEvent.GetScreenSpacePosition()))
                {
                    return true;
                }
            }
        }
    }
                    return false;
                }

FReply UInventoryWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    if (InKeyEvent.GetKey() == EKeys::Escape)
    {
        // Закрываем окно модификации броника, если открыто
        if (OpenedArmorForMods && ArmorModWindowBorder && ArmorModWindowBorder->GetVisibility() == ESlateVisibility::Visible)
        {
            CloseArmorModWindow();
            return FReply::Handled();
        }

        // Если открыто окно модуля — закрываем только его
        if (OpenedEquipMod && EquipModWindowBorder && EquipModWindowBorder->GetVisibility() == ESlateVisibility::Visible)
        {
            CloseEquipModStorage();
            return FReply::Handled();
        }

        // Закрываем инвентарь при нажатии Escape
        SetVisible(false);
        
        // Возвращаем управление в игровой режим
        if (APlayerController* PC = Cast<APlayerController>(GetOwningPlayer()))
        {
            UWidgetBlueprintLibrary::SetInputMode_GameOnly(PC);
            PC->SetIgnoreLookInput(false);
            PC->SetShowMouseCursor(false);
        }
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Inventory CLOSED by Escape"));
        }
        
        return FReply::Handled();
    }
    
    // Обработка вращения предмета
    if (HandleItemRotation(InGeometry, InKeyEvent).IsEventHandled())
    {
        return FReply::Handled();
    }
    return FReply::Unhandled();
}

FReply UInventoryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    // Закрываем контекстное меню при клике вне его
    if (UCanvasPanel* RootLocal = Cast<UCanvasPanel>(WidgetTree->RootWidget))
    {
        TArray<UWidget*> Children2 = RootLocal->GetAllChildren();
        for (UWidget* W2 : Children2)
        {
            if (W2 && W2->GetFName() == TEXT("ContextMenu"))
            {
                RootLocal->RemoveChild(W2);
                break;
            }
        }
    }

    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        const FVector2D ScreenPos = InMouseEvent.GetScreenSpacePosition();

        // Перетаскивание окна хранилища модуля (EquipModBase)
        if (EquipModWindowBorder && EquipModWindowBorder->GetVisibility() == ESlateVisibility::Visible &&
            EquipModWindowBorder->GetCachedGeometry().IsUnderLocation(ScreenPos))
        {
            if (UCanvasPanelSlot* WS = Cast<UCanvasPanelSlot>(EquipModWindowBorder->Slot))
            {
                bDraggingEquipModWindow = true;
                EquipModDragStartMouseScreen = ScreenPos;
                EquipModDragStartWindowPos = WS->GetPosition();
                return FReply::Handled().CaptureMouse(TakeWidget());
            }
        }

        // Окно модификации броника: зона ресайза (правый/нижний край) -> ресайз; по превью — вращение; иначе — перетаскивание
        if (ArmorModWindowBorder && ArmorModWindowBorder->GetVisibility() == ESlateVisibility::Visible &&
            ArmorModWindowBorder->GetCachedGeometry().IsUnderLocation(ScreenPos))
        {
            const FGeometry& WinGeo = ArmorModWindowBorder->GetCachedGeometry();
            const FVector2D WinSize = WinGeo.GetLocalSize();
            const FVector2D LocalPos = WinGeo.AbsoluteToLocal(ScreenPos);
            const bool bOnRightEdge = (WinSize.X > ArmorModResizeEdgePx && LocalPos.X >= WinSize.X - ArmorModResizeEdgePx);
            const bool bOnBottomEdge = (WinSize.Y > ArmorModResizeEdgePx && LocalPos.Y >= WinSize.Y - ArmorModResizeEdgePx);

            if (bOnRightEdge || bOnBottomEdge)
            {
                if (UCanvasPanelSlot* ResizeSlot = Cast<UCanvasPanelSlot>(ArmorModWindowBorder->Slot))
                {
                    bResizingArmorModWindow = true;
                    ArmorModResizeStartMouseScreen = ScreenPos;
                    ArmorModResizeStartSize = ResizeSlot->GetSize();
                    ArmorModResizeStartPosition = ResizeSlot->GetPosition();
                    bArmorModResizeRight = bOnRightEdge;
                    bArmorModResizeBottom = bOnBottomEdge;
                    return FReply::Handled().CaptureMouse(TakeWidget());
                }
            }
            // Вращение превью по области картинки
            if (ArmorModImage && ArmorModImage->GetCachedGeometry().IsUnderLocation(ScreenPos))
            {
                bRotatingArmorPreview = true;
                ArmorPreviewLastMouse = ScreenPos;
                return FReply::Handled().CaptureMouse(TakeWidget());
            }
            // Перетаскивание окна (клик по рамке/заголовку, не по картинке и не по зоне ресайза)
            if (UCanvasPanelSlot* WS = Cast<UCanvasPanelSlot>(ArmorModWindowBorder->Slot))
            {
                bDraggingArmorModWindow = true;
                ArmorModDragStartMouseScreen = ScreenPos;
                ArmorModDragStartWindowPos = WS->GetPosition();
                return FReply::Handled().CaptureMouse(TakeWidget());
            }
        }

    }

    // ПКМ по превью брони: контекстное меню (Установить/Снять) или вращение, если клик не по ячейке
    if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton &&
        ArmorModWindowBorder && ArmorModWindowBorder->GetVisibility() == ESlateVisibility::Visible &&
        ArmorModImage && ArmorModImage->GetCachedGeometry().IsUnderLocation(InMouseEvent.GetScreenSpacePosition()) &&
        OpenedArmorForMods && ArmorModPreviewActor)
    {
        const FVector2D ScreenPos = InMouseEvent.GetScreenSpacePosition();
        const FGeometry& ImgGeo = ArmorModImage->GetCachedGeometry();
        FVector2D LocalPos = ImgGeo.AbsoluteToLocal(ScreenPos);
        FVector2D WidgetSize = ImgGeo.GetLocalSize();
        int32 Side = 0, CellX = 0, CellY = 0;
        if (ArmorModPreviewActor->HitTestGridFromWidgetPosition(LocalPos, WidgetSize, Side, CellX, CellY))
        {
            const int32 ModIndex = OpenedArmorForMods->GetInstalledModIndexAt(Side, CellX, CellY);
            UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetTree ? WidgetTree->RootWidget : nullptr);
            if (RootCanvas)
            {
                for (UWidget* W2 : RootCanvas->GetAllChildren())
                    if (W2 && W2->GetFName() == TEXT("ContextMenu")) { RootCanvas->RemoveChild(W2); break; }
                UBorder* Menu = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("ContextMenu"));
                Menu->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 0.9f));
                Menu->SetPadding(FMargin(8.f));
                UVerticalBox* VBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ContextMenuVBox"));
                Menu->SetContent(VBox);
                if (ModIndex >= 0)
                {
                    ArmorModContextMenuModIndex = ModIndex;
                    UButton* UninstallBtn = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
                    UTextBlock* UninstallTxt = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
                    UninstallTxt->SetText(FText::FromString(TEXT("Снять")));
                    UninstallBtn->AddChild(UninstallTxt);
                    VBox->AddChildToVerticalBox(UninstallBtn);
                    UninstallBtn->OnClicked.Clear();
                    UninstallBtn->OnClicked.AddDynamic(this, &UInventoryWidget::OnArmorModContextUninstallClicked);
                }
                else
                {
                    ArmorModInstallSide = Side;
                    ArmorModInstallCellX = CellX;
                    ArmorModInstallCellY = CellY;
                    UButton* InstallBtn = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
                    UTextBlock* InstallTxt = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
                    InstallTxt->SetText(FText::FromString(TEXT("Установить")));
                    InstallBtn->AddChild(InstallTxt);
                    InstallBtn->SetIsEnabled(AnyModCanInstallAtArmorCell(Side, CellX, CellY));
                    if (!InstallBtn->GetIsEnabled())
                        InstallTxt->SetColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f, 0.8f)));
                    VBox->AddChildToVerticalBox(InstallBtn);
                    InstallBtn->OnClicked.Clear();
                    InstallBtn->OnClicked.AddDynamic(this, &UInventoryWidget::OnArmorModContextInstallClicked);
                }
                if (UCanvasPanelSlot* S = RootCanvas->AddChildToCanvas(Menu))
                {
                    const FVector2D Local = RootCanvas->GetCachedGeometry().AbsoluteToLocal(ScreenPos);
                    S->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
                    S->SetAlignment(FVector2D(0.f, 0.f));
                    S->SetPosition(Local + FVector2D(6.f, 6.f));
                    S->SetSize(FVector2D(200.f, 80.f));
                    S->SetZOrder(9999);
                }
            }
            return FReply::Handled();
        }
        bRotatingArmorPreview = true;
        ArmorPreviewLastMouse = ScreenPos;
        return FReply::Handled().CaptureMouse(TakeWidget());
    }

    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UInventoryWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        if (bRotatingArmorPreview)
        {
            bRotatingArmorPreview = false;
            return FReply::Handled().ReleaseMouseCapture();
        }
        if (bResizingArmorModWindow)
        {
            bResizingArmorModWindow = false;
            return FReply::Handled().ReleaseMouseCapture();
        }
        bool bHadDrag = false;
        if (bDraggingEquipModWindow)
        {
            bDraggingEquipModWindow = false;
            bHadDrag = true;
        }
        if (bDraggingArmorModWindow)
        {
            bDraggingArmorModWindow = false;
            bHadDrag = true;
        }
        if (bHadDrag)
        {
            return FReply::Handled().ReleaseMouseCapture();
        }
    }
    if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton && bRotatingArmorPreview)
    {
        bRotatingArmorPreview = false;
        return FReply::Handled().ReleaseMouseCapture();
    }
    return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

void UInventoryWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    ClearArmorModHighlight();
}

FReply UInventoryWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (bDraggingEquipModWindow && EquipModWindowBorder && EquipModWindowBorder->GetVisibility() == ESlateVisibility::Visible)
    {
        UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetTree ? WidgetTree->RootWidget : nullptr);
        if (RootCanvas)
        {
            const FVector2D MouseNowLocal = RootCanvas->GetCachedGeometry().AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
            const FVector2D MouseStartLocal = RootCanvas->GetCachedGeometry().AbsoluteToLocal(EquipModDragStartMouseScreen);
            const FVector2D DeltaLocal = MouseNowLocal - MouseStartLocal;

            if (UCanvasPanelSlot* WS = Cast<UCanvasPanelSlot>(EquipModWindowBorder->Slot))
            {
                WS->SetPosition(EquipModDragStartWindowPos + DeltaLocal);
            }

            // ВАЖНО: обновляем зарегистрированные зоны "модуль*" (они в координатах RootCanvas),
            // иначе дроп/снап будет работать по старым координатам.
            for (FGridArea& A : GridAreas)
            {
                if (A.Name.StartsWith(TEXT("модуль")))
                {
                    A.Position += DeltaLocal;
                }
            }
        }
        return FReply::Handled();
    }

    if ((bResizingArmorModWindow || bDraggingArmorModWindow || bRotatingArmorPreview) && ArmorModWindowBorder && ArmorModWindowBorder->GetVisibility() == ESlateVisibility::Visible)
    {
        UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetTree ? WidgetTree->RootWidget : nullptr);
        if (RootCanvas)
        {
            const FVector2D ScreenPos = InMouseEvent.GetScreenSpacePosition();

            if (bResizingArmorModWindow)
            {
                const FVector2D MouseNowLocal = RootCanvas->GetCachedGeometry().AbsoluteToLocal(ScreenPos);
                const FVector2D MouseStartLocal = RootCanvas->GetCachedGeometry().AbsoluteToLocal(ArmorModResizeStartMouseScreen);
                FVector2D Delta = MouseNowLocal - MouseStartLocal;
                FVector2D NewSize = ArmorModResizeStartSize;
                if (bArmorModResizeRight)  NewSize.X = FMath::Max(ArmorModMinWidth, ArmorModResizeStartSize.X + Delta.X);
                if (bArmorModResizeBottom) NewSize.Y = FMath::Max(ArmorModMinHeight, ArmorModResizeStartSize.Y + Delta.Y);
                if (UCanvasPanelSlot* WS = Cast<UCanvasPanelSlot>(ArmorModWindowBorder->Slot))
                {
                    WS->SetSize(NewSize);
                    WS->SetPosition(ArmorModResizeStartPosition);
                }
            }
            else if (bDraggingArmorModWindow)
            {
                const FVector2D MouseNowLocal = RootCanvas->GetCachedGeometry().AbsoluteToLocal(ScreenPos);
                const FVector2D MouseStartLocal = RootCanvas->GetCachedGeometry().AbsoluteToLocal(ArmorModDragStartMouseScreen);
                const FVector2D DeltaLocal = MouseNowLocal - MouseStartLocal;

                if (UCanvasPanelSlot* WS = Cast<UCanvasPanelSlot>(ArmorModWindowBorder->Slot))
                {
                    WS->SetPosition(ArmorModDragStartWindowPos + DeltaLocal);
                }
            }

            if (bRotatingArmorPreview)
            {
                const FVector2D Delta = ScreenPos - ArmorPreviewLastMouse;
                ArmorPreviewLastMouse = ScreenPos;
                const float Sens = 0.8f;
                const float PanSens = 0.5f;
                const bool bBothButtons = InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton) && InMouseEvent.IsMouseButtonDown(EKeys::RightMouseButton);
                const bool bRollMode = InMouseEvent.IsControlDown();

                if (ArmorModPreviewActor)
                {
                    if (bBothButtons)
                    {
                        // Перемещение камеры в трёх плоскостях без вращения модели (ЛКМ + ПКМ)
                        ArmorModPreviewActor->AddPanDelta(Delta.X * PanSens, -Delta.Y * PanSens, 0.f);
                    }
                    else if (bRollMode)
                    {
                        ArmorModPreviewActor->PreviewRollDegrees += Delta.X * Sens;
                        ArmorModPreviewActor->ApplyRotation();
                    }
                    else
                    {
                        ArmorModPreviewActor->PreviewYawDegrees += Delta.X * Sens;
                        ArmorModPreviewActor->PreviewPitchDegrees -= Delta.Y * Sens;
                        ArmorModPreviewActor->ApplyRotation();
                    }
                }
                else if (ArmorModImage)
                {
                    const float AngleDelta = Delta.X * Sens;
                    FWidgetTransform T = ArmorModImage->GetRenderTransform();
                    T.Angle += AngleDelta;
                    T.Scale = FVector2D(ArmorModPreviewScale, ArmorModPreviewScale);
                    ArmorModImage->SetRenderTransform(T);
                }
            }
        }
        return FReply::Handled();
    }

    return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
}

FReply UInventoryWidget::NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (ArmorModWindowBorder && ArmorModWindowBorder->GetVisibility() == ESlateVisibility::Visible &&
        ArmorModImage && ArmorModImage->GetCachedGeometry().IsUnderLocation(InMouseEvent.GetScreenSpacePosition()))
    {
        const float Delta = InMouseEvent.GetWheelDelta();
        if (ArmorModPreviewActor)
        {
            // Приближение/удаление камеры от модели (положительный Delta — колесо вверх — приближение)
            const float ZoomSpeed = 18.f;
            ArmorModPreviewActor->AddZoomDelta(Delta * ZoomSpeed);
        }
        else
        {
            // Заглушка: масштаб 2D-картинки, если превью-актора нет
            ArmorModPreviewScale = FMath::Clamp(ArmorModPreviewScale + Delta * 0.12f, 0.3f, 3.f);
            FWidgetTransform T = ArmorModImage->GetRenderTransform();
            T.Scale = FVector2D(ArmorModPreviewScale, ArmorModPreviewScale);
            ArmorModImage->SetRenderTransform(T);
        }
        return FReply::Handled();
    }
    return Super::NativeOnMouseWheel(InGeometry, InMouseEvent);
}

void UInventoryWidget::MarkCellsAsOccupied(int32 X, int32 Y, int32 SizeX, int32 SizeY)
{
    const int32 MaxCols = OccupiedCells.Num() > 0 ? OccupiedCells[0].Num() : 0;
    const int32 MaxRows = OccupiedCells.Num();
    
    for (int32 dy = 0; dy < SizeY && Y + dy < MaxRows; ++dy)
    {
        for (int32 dx = 0; dx < SizeX && X + dx < MaxCols; ++dx)
        {
            if (Y + dy >= 0 && X + dx >= 0)
            {
                OccupiedCells[Y + dy][X + dx] = true;
            }
        }
    }
}

void UInventoryWidget::MarkCellsAsFree(int32 X, int32 Y, int32 SizeX, int32 SizeY)
{
    const int32 MaxCols = OccupiedCells.Num() > 0 ? OccupiedCells[0].Num() : 0;
    const int32 MaxRows = OccupiedCells.Num();
    
    for (int32 dy = 0; dy < SizeY && Y + dy < MaxRows; ++dy)
    {
        for (int32 dx = 0; dx < SizeX && X + dx < MaxCols; ++dx)
        {
            if (Y + dy >= 0 && X + dx >= 0)
            {
                OccupiedCells[Y + dy][X + dx] = false;
            }
        }
    }
}

bool UInventoryWidget::AreCellsFree(int32 X, int32 Y, int32 SizeX, int32 SizeY) const
{
    const int32 MaxCols = OccupiedCells.Num() > 0 ? OccupiedCells[0].Num() : 0;
    const int32 MaxRows = OccupiedCells.Num();
    
    // Проверяем границы
    if (X < 0 || Y < 0 || X + SizeX > MaxCols || Y + SizeY > MaxRows)
    {
        return false;
    }
    
    // Проверяем занятость ячеек
    for (int32 dy = 0; dy < SizeY; ++dy)
    {
        for (int32 dx = 0; dx < SizeX; ++dx)
        {
            if (OccupiedCells[Y + dy][X + dx])
            {
                    return false;
            }
        }
    }
    
            return true;
        }

void UInventoryWidget::RegisterGrid(const FString& Name, const FVector2D& Pos, const FVector2D& Size, int32 CellsX, int32 CellsY)
{
    FGridArea Area;
    Area.Name = Name;
    Area.Position = Pos;
    Area.Size = Size;
    Area.CellsX = CellsX;
    Area.CellsY = CellsY;
    GridAreas.Add(Area);
}

int32 UInventoryWidget::FindGridAtPoint(const FVector2D& LocalPoint) const
{
    // Идём с конца, чтобы отдавать приоритет последним зарегистрированным (поверх лежащим) грид‑областям
    for (int32 i = GridAreas.Num() - 1; i >= 0; --i)
    {
        const FGridArea& Area = GridAreas[i];
        if (LocalPoint.X >= Area.Position.X && LocalPoint.Y >= Area.Position.Y &&
            LocalPoint.X < Area.Position.X + Area.Size.X && LocalPoint.Y < Area.Position.Y + Area.Size.Y)
        {
            return i;
        }
    }
    return INDEX_NONE;
}

FVector2D UInventoryWidget::SnapToCellInGrid(int32 GridIndex, const FVector2D& LocalPoint) const
{
    if (!GridAreas.IsValidIndex(GridIndex)) return LocalPoint;
    
    const FGridArea& A = GridAreas[GridIndex];
    const FVector2D CellSize = FVector2D(A.Size.X / A.CellsX, A.Size.Y / A.CellsY);
    const int32 X = FMath::Clamp((int32)FMath::FloorToInt((LocalPoint.X - A.Position.X) / CellSize.X), 0, A.CellsX-1);
    const int32 Y = FMath::Clamp((int32)FMath::FloorToInt((LocalPoint.Y - A.Position.Y) / CellSize.Y), 0, A.CellsY-1);
    return A.Position + FVector2D(X * CellSize.X, Y * CellSize.Y);
}

bool UInventoryWidget::TryAutoPlaceInGrid(int32 GridIndex, int32 SizeX, int32 SizeY, int32& OutCellX, int32& OutCellY) const
{
    if (!GridAreas.IsValidIndex(GridIndex)) return false;
    
    const FGridArea& A = GridAreas[GridIndex];
    for (int32 Y = 0; Y <= A.CellsY - SizeY; ++Y)
    {
        for (int32 X = 0; X <= A.CellsX - SizeX; ++X)
        {
            if (IsAreaFree(GridIndex, X, Y, SizeX, SizeY, nullptr))
            {
                OutCellX = X;
                OutCellY = Y;
                return true;
            }
        }
    }
    return false;
}

bool UInventoryWidget::IsAreaFree(int32 GridIndex, int32 CellX, int32 CellY, int32 SizeX, int32 SizeY, UInventoryItemWidget* Ignore) const
{
    if (!GridAreas.IsValidIndex(GridIndex)) return false;
    const FGridArea& A = GridAreas[GridIndex];
    // Для гридов жилета пока не ведём учёт занятости — разрешаем дроп при попадании в границы
    if (A.Name.StartsWith(TEXT("жилет")))
    {
        return (CellX >= 0 && CellY >= 0 && CellX + SizeX <= A.CellsX && CellY + SizeY <= A.CellsY);
    }
    
    if (CellX < 0 || CellY < 0 || CellX + SizeX > A.CellsX || CellY + SizeY > A.CellsY) return false;
    for (const FPlacedItem& P : Placed)
    {
        if (P.Widget == Ignore) continue;
        if (P.GridIndex != GridIndex) continue;
        const int32 Ax0 = CellX, Ay0 = CellY, Ax1 = CellX + SizeX - 1, Ay1 = CellY + SizeY - 1;
        const int32 Bx0 = P.CellX, By0 = P.CellY, Bx1 = P.CellX + P.SizeX - 1, By1 = P.CellY + P.SizeY - 1;
        const bool bOverlap = !(Ax1 < Bx0 || Bx1 < Ax0 || Ay1 < By0 || By1 < Ay0);
        if (bOverlap) return false;
    }
    return true;
}

void UInventoryWidget::UpsertPlacement(UInventoryItemWidget* Widget, int32 GridIndex, int32 CellX, int32 CellY)
{
    if (!Widget) return;
    int32 SX = Widget->bRotated ? Widget->SizeY : Widget->SizeX;
    int32 SY = Widget->bRotated ? Widget->SizeX : Widget->SizeY;
    for (FPlacedItem& P : Placed)
    {
        if (P.Widget == Widget)
        {
            P.GridIndex = GridIndex; P.CellX = CellX; P.CellY = CellY; P.SizeX = SX; P.SizeY = SY; P.bRotated = Widget->bRotated;
            return;
        }
    }
    FPlacedItem NewP; NewP.Widget=Widget; NewP.GridIndex=GridIndex; NewP.CellX=CellX; NewP.CellY=CellY; NewP.SizeX=SX; NewP.SizeY=SY; NewP.bRotated=Widget->bRotated; Placed.Add(NewP);
}



void UInventoryWidget::CreateVestGrid()
{
    if (false && GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, TEXT("🔍 CreateVestGrid called"));
    }
    
    if (VestGrids.Num() > 0)
    {
        DestroyVestGrid();
    }

    // На всякий случай очищаем старые абстрактные области жилета (если они были зарегистрированы ранее),
    // иначе FindGridAtPoint может продолжать возвращать "жилет1" и перехватывать дроп.
    GridAreas.RemoveAll([](const FGridArea& A)
    {
        return A.Name.StartsWith(TEXT("жилет"));
    });
    
    // Размещаем гриды жилета внутри RightPanelRef, чтобы координаты совпадали с hit-test
    UCanvasPanel* RightPanel = RightPanelRef;
    if (!RightPanel)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("❌ RightPanelRef not found"));
        }
        return;
    }
    
    // Создаем грид жилета в стиле остальных гридов
            auto AddVestLabeledGrid = [&](const TCHAR* Label, float XPos, float YPos, FVector2D GridSize, int32 GridWidth, int32 GridHeight, const TCHAR* SlotLabel = nullptr, bool bShowLabel = true)
    {
        // Слот слева от надписи
        if (SlotLabel)
        {
            UBorder* Slot = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
            Slot->SetBrushColor(FLinearColor(1.f,1.f,1.f,0.05f));
            if (UCanvasPanelSlot* SS = RightPanel->AddChildToCanvas(Slot))
            {
                SS->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
                SS->SetAlignment(FVector2D(0.f, 0.f));
                SS->SetPosition(FVector2D(XPos, YPos));
                SS->SetSize(FVector2D(60.f, 60.f));
            }
            
            UTextBlock* SlotText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
            SlotText->SetText(FText::FromString(SlotLabel));
            SlotText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
            SlotText->SetJustification(ETextJustify::Center);
            if (UCanvasPanelSlot* ST = RightPanel->AddChildToCanvas(SlotText))
            {
                ST->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
                ST->SetAlignment(FVector2D(0.f, 0.f));
                ST->SetPosition(FVector2D(XPos, YPos));
                ST->SetSize(FVector2D(60.f, 60.f));
            }
        }
        
        // Создаем грид
        const FName GridName(*FString::Printf(TEXT("VestGrid_%s"), Label));
        UCanvasPanel* Grid = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), GridName);
        if (Grid)
        {
            VestGrids.Add(Grid); // Сохраняем ссылку в массив
            VestGridSizes.Add(GridSize); // Сохраняем размер
            
            FVector2D Pos = FVector2D(XPos, YPos);
            if (UCanvasPanelSlot* GS = RightPanel->AddChildToCanvas(Grid))
            {
                GS->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
                GS->SetAlignment(FVector2D(0.f, 0.f));
                GS->SetPosition(Pos);
                GS->SetSize(GridSize);
                GS->SetAutoSize(false);
                // Инвертируем порядок отрисовки по вертикали:
                // гриды, расположенные выше (меньший YPos), должны рисоваться поверх нижних.
                // Используем базовое смещение, чтобы гарантировать ZOrder > 0 и не лезть в экстремальные значения.
                const int32 BaseZ = 100;
                const int32 InvertedY = 10000 - static_cast<int32>(YPos);
                GS->SetZOrder(BaseZ + InvertedY);
                // ВАЖНО: для жилета больше НЕ регистрируем эти гриды в GridAreas.
                // Hit-test и дроп для жилета обрабатываются только через геометрию VestGrid_* (bIsVestGrid),
                // чтобы каждый мини‑грид был независимым и не перехватывался абстрактным гридом 6x2.
            }
            
            // Добавляем фон грида (видимый)
            UBorder* GridBackground = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
            if (GridBackground)
            {
                // Единый стиль как у пояса
                GridBackground->SetBrushColor(FLinearColor(1.f,1.f,1.f,0.05f));
                UCanvasPanelSlot* BgSlot = Grid->AddChildToCanvas(GridBackground);
                if (BgSlot)
                {
                    BgSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
                    BgSlot->SetOffsets(FMargin(0.0f));
                }
            }
            
            // Добавляем сетку ячеек
            const int32 CellSize = 60;
            for (int32 Row = 0; Row < GridHeight; ++Row)
            {
                for (int32 Col = 0; Col < GridWidth; ++Col)
                {
                    // Создаем контейнер для ячейки
                    const FName CellName(*FString::Printf(TEXT("VestCell_%s_r%d_c%d"), Label, Row, Col));
                    UCanvasPanel* CellContainer = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), CellName);
                    if (CellContainer)
                    {
                        // Добавляем фон ячейки
                        const FName CellBgName(*FString::Printf(TEXT("VestCellBg_%s_r%d_c%d"), Label, Row, Col));
                        UBorder* CellBackground = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), CellBgName);
                        if (CellBackground)
                        {
                            // Унифицированный стиль как у пояса/рюкзака/карманов
                            CellBackground->SetBrushColor(FLinearColor(1.f,1.f,1.f,0.05f));
                            UCanvasPanelSlot* BgSlot = CellContainer->AddChildToCanvas(CellBackground);
                            if (BgSlot)
                            {
                                BgSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
                                BgSlot->SetOffsets(FMargin(1.0f)); // Отступы для границ
                            }
                        }
                        
                        // Добавляем контейнер в грид
                        UCanvasPanelSlot* CellSlot = Grid->AddChildToCanvas(CellContainer);
                        if (CellSlot)
                        {
                            CellSlot->SetPosition(FVector2D(Col * CellSize, Row * CellSize));
                            CellSlot->SetSize(FVector2D(CellSize, CellSize));
                        }
                    }
                }
            }
            
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                    FString::Printf(TEXT("✅ Vest grid created: %s Pos(%.0f,%.0f), Size(%.0fx%.0f)"), 
                        *GridName.ToString(), Pos.X, Pos.Y, GridSize.X, GridSize.Y));
            }
        }
    };
    
    // Создаем N отдельных гридов жилета. Размеры и смещения (в клетках) берём из экипированного жилета, если заданы.
    const float VestY = 20.f;
    const float VestXStart = 70.f;
    // Небольшой визуальный зазор между мини‑гридами, чтобы они не сливались в один блок
    const int32 GridGapPx = 6;
    const float Gap = 10.f; // legacy (используется только в старой раскладке/логах, не критично)
    const int32 CellSize = 60;

    // Пытаемся взять конфигурацию из экипированного жилета (AdditionalGrids + AdditionalGridOffsets).
    // Если пусто — фолбэк на старые 6 мини‑гридов и раскладку "в линию".
    TArray<FIntPoint> GridSizes;
    TArray<FIntPoint> GridOffsetsCells;
    if (APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(GetOwningPlayerPawn()))
    {
        if (UInventoryComponent* Inv = PlayerChar->InventoryComponent)
        {
            if (UEquippableItemData* EquippedVest = Inv->GetEquippedItem(Vest))
            {
                if (EquippedVest->AdditionalGrids.Num() > 0)
                {
                    GridSizes = EquippedVest->AdditionalGrids;
                    GridOffsetsCells = EquippedVest->AdditionalGridOffsets;
                }
            }
        }
    }
    if (GridSizes.Num() == 0)
    {
        GridSizes = {
            FIntPoint(1, 1),
            FIntPoint(1, 1),
            FIntPoint(1, 2),
            FIntPoint(1, 2),
            FIntPoint(1, 1),
            FIntPoint(1, 1)
        };
    }

    // Если оффсеты не заданы или длина не совпадает — раскладываем по умолчанию слева направо
    if (GridOffsetsCells.Num() != GridSizes.Num())
    {
        GridOffsetsCells.SetNum(GridSizes.Num());
        float XCells = 0.f;
        for (int32 i = 0; i < GridSizes.Num(); ++i)
        {
            GridOffsetsCells[i] = FIntPoint(FMath::RoundToInt(XCells), 0);
            const float W = (float)FMath::Max(1, GridSizes[i].X);
            XCells += W; // впритык; реальный зазор добавляется на уровне пикселей (GridGapPx)
        }
    }

    {
        TMap<int32, float> ColStartPx;
        TMap<int32, float> RowStartPx;
        BuildTableLayoutPx(GridSizes, GridOffsetsCells, CellSize, GridGapPx, ColStartPx, RowStartPx);

        // Инвертируем вертикальное расположение мини‑гридов жилета:
        // рассчитываем минимальный и максимальный старт по Y, чтобы затем отзеркалить координаты.
        float MinRowStart = 0.f;
        float MaxRowStart = 0.f;
        bool bHasRowStart = false;
        for (const TPair<int32, float>& Pair : RowStartPx)
        {
            const float V = Pair.Value;
            if (!bHasRowStart)
            {
                MinRowStart = MaxRowStart = V;
                bHasRowStart = true;
            }
            else
            {
                MinRowStart = FMath::Min(MinRowStart, V);
                MaxRowStart = FMath::Max(MaxRowStart, V);
            }
        }
        if (!bHasRowStart)
        {
            MinRowStart = MaxRowStart = 0.f;
        }

        for (int32 i = 0; i < GridSizes.Num(); ++i)
        {
            const FIntPoint G = FIntPoint(FMath::Max(1, GridSizes[i].X), FMath::Max(1, GridSizes[i].Y));
            const FIntPoint Off = GridOffsetsCells.IsValidIndex(i) ? GridOffsetsCells[i] : FIntPoint(0, 0);
            const float X0 = ColStartPx.Contains(Off.X) ? ColStartPx[Off.X] : 0.f;
            const float Y0Raw = RowStartPx.Contains(Off.Y) ? RowStartPx[Off.Y] : 0.f;
            // Зеркалим Y: самый нижний мини‑грид жилета становится верхним и наоборот.
            const float Y0 = (MinRowStart + MaxRowStart) - Y0Raw;
            const float XPos = VestXStart + X0;
            const float YPos = VestY + Y0;
            const FVector2D GridSizePx = FVector2D((float)(G.X * CellSize), (float)(G.Y * CellSize));
            const int32 LabelIdx = i + 1;
            const FString LabelStr = FString::Printf(TEXT("жилет%d"), LabelIdx);
            AddVestLabeledGrid(*LabelStr, XPos, YPos, GridSizePx, G.X, G.Y, nullptr, false);
        }
    }

    // Инициализируем VestOccupiedCells под созданные гриды (динамически)
    VestOccupiedCells.SetNum(GridSizes.Num());
    for (int32 i = 0; i < GridSizes.Num(); ++i)
    {
        const int32 W = FMath::Max(1, GridSizes[i].X);
        const int32 H = FMath::Max(1, GridSizes[i].Y);
        VestOccupiedCells[i].SetNum(H);
        for (int32 r = 0; r < H; ++r)
        {
            VestOccupiedCells[i][r].SetNum(W);
            for (int32 c = 0; c < W; ++c)
            {
                VestOccupiedCells[i][r][c] = false;
            }
        }
    }
    
    // Проверяем, что гриды создались
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, 
            FString::Printf(TEXT("🔍 Created %d vest grids successfully"), VestGrids.Num()));
    }
}


void UInventoryWidget::DestroyVestGrid()
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, 
            FString::Printf(TEXT("🔍 DestroyVestGrid called, destroying %d grids"), VestGrids.Num()));
    }
    
    // Удаляем все гриды жилета
    for (int32 GridIdx = 0; GridIdx < VestGrids.Num(); ++GridIdx)
    {
        UCanvasPanel* Grid = VestGrids[GridIdx];
        if (Grid)
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow,
                    FString::Printf(TEXT("🗑️ Removing VestGrid[%d]: %s, children: %d"), GridIdx, *Grid->GetName(), Grid->GetChildrenCount()));
            }

            // Сначала очищаем всех детей грида
            Grid->ClearChildren();
            // Очищаем все виджеты предметов из этого грида
            for (auto& Pair : ItemToWidget)
            {
                if (Pair.Value && Pair.Value->bIsStaticEquipmentSlot == false)
                {
                    UCanvasPanel* ParentPanel = Cast<UCanvasPanel>(Pair.Value->GetParent());
                    if (ParentPanel == Grid)
                    {
                        Pair.Value->RemoveFromParent();
                    }
                }
            }
            
            // Принудительно удаляем грид
            Grid->RemoveFromParent();
        }
    }
    
    VestGrids.Empty();
    VestGridSizes.Empty();
    
    // Очищаем занятые ячейки гридов жилета
    VestOccupiedCells.Empty();
    
    // Дополнительно: проходим по RightPanelRef и удаляем любые оставшиеся VestGrid*
    if (RightPanelRef)
    {
        TArray<UWidget*> ToRemove;
        const int32 Count = RightPanelRef->GetChildrenCount();
        for (int32 i = 0; i < Count; ++i)
        {
            if (UWidget* Child = RightPanelRef->GetChildAt(i))
            {
                if (Child->GetName().StartsWith(TEXT("VestGrid_")))
                {
                    ToRemove.Add(Child);
                }
            }
        }
        for (UWidget* W : ToRemove)
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red,
                    FString::Printf(TEXT("🧹 Sweeping leftover widget: %s"), *W->GetName()));
            }
            RightPanelRef->RemoveChild(W);
        }
    }
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
            FString::Printf(TEXT("🗑️ Уничтожены все гриды жилета (было: %d)"), VestGrids.Num()));
    }
}

void UInventoryWidget::DestroyArmorModGrids()
{
    for (UCanvasPanel* Grid : ArmorModGrids)
    {
        if (Grid)
        {
            Grid->ClearChildren();
            for (auto& Pair : ItemToWidget)
            {
                if (Pair.Value && !Pair.Value->bIsStaticEquipmentSlot)
                {
                    if (Cast<UCanvasPanel>(Pair.Value->GetParent()) == Grid)
                    {
                        Pair.Value->RemoveFromParent();
                    }
                }
            }
            Grid->RemoveFromParent();
        }
    }
    ArmorModGrids.Empty();
    ArmorModGridSizes.Empty();
    ArmorModGridModData.Empty();
    ArmorModGridMiniIndices.Empty();
    if (RightPanelRef)
    {
        TArray<UWidget*> ToRemove;
        for (int32 i = 0; i < RightPanelRef->GetChildrenCount(); ++i)
        {
            if (UWidget* W = RightPanelRef->GetChildAt(i))
            {
                if (W->GetName().StartsWith(TEXT("ArmorModGrid_")))
                    ToRemove.Add(W);
            }
        }
        for (UWidget* W : ToRemove)
            RightPanelRef->RemoveChild(W);
    }
}

void UInventoryWidget::CreateArmorModGrids()
{
    if (ArmorModGrids.Num() > 0)
        DestroyArmorModGrids();

    APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(GetOwningPlayerPawn());
    if (!PlayerChar || !PlayerChar->InventoryComponent || !RightPanelRef)
        return;

    UEquippableItemData* EquippedArmor = Cast<UEquippableItemData>(PlayerChar->InventoryComponent->GetEquippedItem(Armor));
    if (!EquippedArmor)
        return;

    const float VestY = 20.f;
    const float VestXStart = 70.f;
    const int32 CellSize = 60;
    const int32 GridGapPx = 6;

    float XPos = VestXStart;
    if (VestGrids.Num() > 0 && VestGridSizes.Num() > 0)
    {
        for (const FVector2D& Sz : VestGridSizes)
            XPos += Sz.X + GridGapPx;
    }
    const float ArmorModsBaseX = XPos;

    // Собираем список модов с хранилищем и их позицией на сетке, чтобы разместить мини‑гриды в порядке, зависящем от CellY/CellX
    struct FArmorModForGrid
    {
        UEquipModBaseItemData* ModData = nullptr;
        int32 CellX = 0;
        int32 CellY = 0;
    };

    TArray<FArmorModForGrid> ModsForGrids;
    ModsForGrids.Reserve(EquippedArmor->InstalledMods.Num());

    for (const FInstalledArmorMod& Inst : EquippedArmor->InstalledMods)
    {
        UEquipModBaseItemData* ModData = Inst.ModItemData ? Cast<UEquipModBaseItemData>(Inst.ModItemData) : nullptr;
        if (!ModData || !ModData->bHasAdditionalStorage)
            continue;

        FArmorModForGrid Entry;
        Entry.ModData = ModData;
        Entry.CellX = Inst.CellX;
        Entry.CellY = Inst.CellY;
        ModsForGrids.Add(Entry);
    }

    // Сортируем по строке, затем по столбцу сетки модов:
    // если один мод стоит выше другого на сетке (меньший CellY), он идёт раньше.
    ModsForGrids.Sort([](const FArmorModForGrid& A, const FArmorModForGrid& B)
    {
        if (A.CellY != B.CellY)
        {
            return A.CellY < B.CellY;
        }
        return A.CellX < B.CellX;
    });

    // Строим сжатые ранги по CellY для модов: порядок "выше/ниже" берём из CellY,
    // но затем используем только ранги 0..N-1, чтобы рисовать моды плотной вертикальной колонкой
    // без дыр, независимо от конкретных значений CellY.
    TArray<int32> DistinctCellYs;
    DistinctCellYs.Reserve(ModsForGrids.Num());
    for (const FArmorModForGrid& Entry : ModsForGrids)
    {
        DistinctCellYs.AddUnique(Entry.CellY);
    }
    DistinctCellYs.Sort();

    TMap<int32, int32> CellYToRank;
    for (int32 Index = 0; Index < DistinctCellYs.Num(); ++Index)
    {
        // Инвертируем порядок: больший CellY (ниже на броне) получает меньший ранг,
        // чтобы визуально располагаться выше в 2D‑представлении.
        const int32 CellY = DistinctCellYs[Index];
        const int32 InvertedRank = (DistinctCellYs.Num() - 1) - Index;
        CellYToRank.Add(CellY, InvertedRank);
    }

    // Для отображения используем "сжатые" визуальные строки модов по CellY:
    // моды с меньшим CellY идут в более верхних строках (ранг 0,1,2,...),
    // при этом расстояние между строками фиксированное и не зависит от разницы в CellY.
    TMap<int32, float> ModRowToNextX;
    for (int32 RankIdx = 0; RankIdx < DistinctCellYs.Num(); ++RankIdx)
    {
        ModRowToNextX.Add(RankIdx, ArmorModsBaseX);
    }

    // Прямоугольники уже размещённых мини‑гридов для проверки наложения (в координатах канваса).
    TArray<FBox2D> PlacedMiniGridRects;
    const float StepY = (float)(CellSize + GridGapPx);

    // Для каждого мода из отсортированного списка создаём его мини‑гриды
    for (const FArmorModForGrid& ModEntry : ModsForGrids)
    {
        UEquipModBaseItemData* ModData = ModEntry.ModData;
        if (!ModData)
        {
            continue;
        }

        TArray<FIntPoint> GridSizes = ModData->AdditionalGrids;
        TArray<FIntPoint> GridOffsetsCells = ModData->AdditionalGridOffsets;
        if (GridSizes.Num() == 0)
        {
            GridSizes.Add(FIntPoint(1, 1));
        }
        if (GridOffsetsCells.Num() != GridSizes.Num())
        {
            GridOffsetsCells.SetNum(GridSizes.Num());
            float XCells = 0.f;
            for (int32 i = 0; i < GridSizes.Num(); ++i)
            {
                GridOffsetsCells[i] = FIntPoint(FMath::RoundToInt(XCells), 0);
                XCells += (float)FMath::Max(1, GridSizes[i].X);
            }
        }

        TMap<int32, float> ColStartPx;
        TMap<int32, float> RowStartPx;
        BuildTableLayoutPx(GridSizes, GridOffsetsCells, CellSize, GridGapPx, ColStartPx, RowStartPx);

        // Вертикальное положение мини‑гридов внутри одного мода берём напрямую из Off.Y
        // (индекса строки в сетке модов), но "сжимаем" эти значения в ранги 0..N-1
        // и рисуем строки друг над другом с фиксированным интервалом, без дыр.
        TSet<int32> DistinctRowIndicesSet;
        for (const FIntPoint& Off : GridOffsetsCells)
        {
            DistinctRowIndicesSet.Add(Off.Y);
        }
        TArray<int32> DistinctRowIndices = DistinctRowIndicesSet.Array();
        DistinctRowIndices.Sort(); // меньший Off.Y = "выше" на сетке

        // Карта: исходный Off.Y -> сжатый ранг 0..N-1
        TMap<int32, int32> RowIndexToRank;
        for (int32 RankIdx = 0; RankIdx < DistinctRowIndices.Num(); ++RankIdx)
        {
            // Инвертируем порядок строк внутри мода: больший Off.Y (ниже на сетке)
            // получает меньший ранг, чтобы визуально быть выше.
            const int32 RowIndex = DistinctRowIndices[RankIdx];
            const int32 InvertedRank = (DistinctRowIndices.Num() - 1) - RankIdx;
            RowIndexToRank.Add(RowIndex, InvertedRank);
        }
        // Для каждого ранга (строки) храним текущий X‑оффсет,
        // чтобы новые мини‑гриды в этой строке шли от начала строки подряд.
        TMap<int32, float> RowRankToNextX;
        for (const int32 RowIndex : DistinctRowIndices)
        {
            const int32 CompactRank = RowIndexToRank.Contains(RowIndex) ? RowIndexToRank[RowIndex] : 0;
            RowRankToNextX.FindOrAdd(CompactRank) = 0.f;
        }

        // Дополнительно вычисляем "реальную" высоту каждой строки в клетках,
        // чтобы строки с высокими (2‑клеточными по вертикали) мини‑гридами
        // подталкивали нижние строки ниже и не было наложения.
        TArray<int32> RowMaxHeightCells;
        RowMaxHeightCells.SetNumZeroed(DistinctRowIndices.Num());
        for (int32 GridIdx = 0; GridIdx < GridSizes.Num(); ++GridIdx)
        {
            const FIntPoint Off = GridOffsetsCells.IsValidIndex(GridIdx) ? GridOffsetsCells[GridIdx] : FIntPoint(0, 0);
            const int32* RankPtr = RowIndexToRank.Find(Off.Y);
            const int32 RowRank = RankPtr ? *RankPtr : 0;
            if (!RowMaxHeightCells.IsValidIndex(RowRank))
            {
                continue;
            }
            const FIntPoint GSize = FIntPoint(FMath::Max(1, GridSizes[GridIdx].X), FMath::Max(1, GridSizes[GridIdx].Y));
            RowMaxHeightCells[RowRank] = FMath::Max(RowMaxHeightCells[RowRank], GSize.Y);
        }

        // Строим базовый Y для каждой строки как накопленную сумму высот
        // предыдущих строк плюс стандартный отступ между строками.
        TArray<float> RowBaseY;
        RowBaseY.SetNumZeroed(DistinctRowIndices.Num());
        const float RowGapPx = (float)GridGapPx;
        for (int32 Rank = 1; Rank < RowBaseY.Num(); ++Rank)
        {
            const int32 PrevHeightCells = FMath::Max(1, RowMaxHeightCells[Rank - 1]);
            const float PrevHeightPx = (float)PrevHeightCells * (float)CellSize;
            RowBaseY[Rank] = RowBaseY[Rank - 1] + PrevHeightPx + RowGapPx;
        }

        // Вычисляем общую ширину модуля, чтобы сдвинуть X‑оффсет для соответствующей строки.
        // Вертикальное расположение модов берём из CellY, но через сжатый ранг:
        // моды идут один над другим с одинаковым интервалом, без дыр по CellY.
        float MaxLocalX = 0.f;
        const int32* BaseRankPtr = CellYToRank.Find(ModEntry.CellY);
        const int32 BaseRank = BaseRankPtr ? *BaseRankPtr : 0;
        // Ранг по CellY напрямую задаёт визуальную строку (0,1,2,...):
        // чем выше мод на сетке (меньший CellY), тем меньше VisualRow.
        const int32 VisualRow = FMath::Max(0, BaseRank);
        const float ModYOffsetPx = (float)VisualRow * (CellSize + GridGapPx);
        float* RowNextXPtr = ModRowToNextX.Find(VisualRow);
        float ModBaseX = RowNextXPtr ? *RowNextXPtr : ArmorModsBaseX;
        const float StandardStepY = (float)CellSize + (float)GridGapPx; // используется для локального Y внутри строки

        // Строим порядок обхода мини‑гридов внутри мода: сначала по "строке" (RowRank),
        // затем по их исходному Off.X, чтобы после сжатия слева направо верхние
        // одноклеточные гриды действительно оказывались слева.
        TArray<int32> GridOrder;
        GridOrder.SetNum(GridSizes.Num());
        for (int32 i = 0; i < GridSizes.Num(); ++i)
        {
            GridOrder[i] = i;
        }
        GridOrder.Sort([&](int32 A, int32 B)
        {
            const FIntPoint OffA = GridOffsetsCells.IsValidIndex(A) ? GridOffsetsCells[A] : FIntPoint(0, 0);
            const FIntPoint OffB = GridOffsetsCells.IsValidIndex(B) ? GridOffsetsCells[B] : FIntPoint(0, 0);
            const int32 RowRankA = RowIndexToRank.Contains(OffA.Y) ? RowIndexToRank[OffA.Y] : 0;
            const int32 RowRankB = RowIndexToRank.Contains(OffB.Y) ? RowIndexToRank[OffB.Y] : 0;
            if (RowRankA != RowRankB)
            {
                // Сначала обрабатываем более нижние (больший RowRank),
                // чтобы при проверке наложения сдвигался именно верхний грид.
                return RowRankA > RowRankB;
            }

            // Внутри строки идём по Off.X справа налево:
            // чем больше Off.X в исходной сетке, тем раньше мини‑грид
            // будет отрендерен и тем левее он окажется после упаковки.
            return OffA.X > OffB.X;
        });

        // Сначала считаем все мини‑гриды мода и их позиции. Если есть наложение,
        // будем смещать вверх ВЕСЬ мод целиком (все его мини‑гриды вместе).
        struct FPendingMiniGrid
        {
            int32 MiniIndex = 0;
            FIntPoint Cells = FIntPoint(1, 1);
            FVector2D SizePx = FVector2D(0.f, 0.f);
            FVector2D Pos = FVector2D(0.f, 0.f);
        };

        TArray<FPendingMiniGrid> PendingMiniGrids;
        PendingMiniGrids.Reserve(GridSizes.Num());

        for (int32 OrderIdx = 0; OrderIdx < GridOrder.Num(); ++OrderIdx)
        {
            const int32 GridIdx = GridOrder[OrderIdx];
            const FIntPoint G = FIntPoint(FMath::Max(1, GridSizes[GridIdx].X), FMath::Max(1, GridSizes[GridIdx].Y));
            const FIntPoint Off = GridOffsetsCells.IsValidIndex(GridIdx) ? GridOffsetsCells[GridIdx] : FIntPoint(0, 0);

            // Берём "сжатый" ранг ряда для вертикали
            const int32 RowRank = RowIndexToRank.Contains(Off.Y) ? RowIndexToRank[Off.Y] : 0;
            const float Y0 = RowBaseY.IsValidIndex(RowRank) ? RowBaseY[RowRank] : 0.f;

            // Горизонтально внутри строки начинаем от X=0 и сдвигаем каждый
            // следующий мини‑грид в этой строке на ширину предыдущих + стандартный интервал.
            float* NextXPtr = RowRankToNextX.Find(RowRank);
            float X0 = NextXPtr ? *NextXPtr : 0.f;
            const FVector2D GridSizePx((float)(G.X * CellSize), (float)(G.Y * CellSize));

            // Обновляем "хвост" строки и общую ширину модуля
            const float NewNextX = X0 + GridSizePx.X + (float)GridGapPx;
            if (NextXPtr)
            {
                *NextXPtr = NewNextX;
            }
            else
            {
                RowRankToNextX.Add(RowRank, NewNextX);
            }
            MaxLocalX = FMath::Max(MaxLocalX, NewNextX);

            // Запоминаем размещение мини‑грида (без финального ModShiftY)
            FPendingMiniGrid P;
            P.MiniIndex = GridIdx;
            P.Cells = G;
            P.SizePx = GridSizePx;
            P.Pos = FVector2D(ModBaseX + X0, VestY + ModYOffsetPx + Y0);
            PendingMiniGrids.Add(P);
        }

        // Разрешаем наложение: смещаем ВЕРХНИЙ мод вверх на 1 шаг (StepY) всем блоком.
        float ModShiftY = 0.f;
        int32 OverlapSafetyIters = 0;
        for (;;)
        {
            bool bAnyOverlap = false;
            float MaxRequiredUp = 0.f; // сколько пикселей надо поднять (положительное число)

            for (const FPendingMiniGrid& P : PendingMiniGrids)
            {
                const float PosX = P.Pos.X;
                const float PosY = P.Pos.Y + ModShiftY;
                const float CurMaxX = PosX + P.SizePx.X;
                const float CurMaxY = PosY + P.SizePx.Y;

                for (const FBox2D& R : PlacedMiniGridRects)
                {
                    if (CurMaxX > R.Min.X && PosX < R.Max.X && CurMaxY > R.Min.Y && PosY < R.Max.Y)
                    {
                        bAnyOverlap = true;

                        // Чтобы уйти вверх, нам нужно, чтобы CurMaxY <= R.Min.Y
                        // => (PosY + SizeY) <= R.Min.Y
                        const float DeltaUpPx = (CurMaxY - R.Min.Y);
                        MaxRequiredUp = FMath::Max(MaxRequiredUp, DeltaUpPx);
                    }
                }
            }

            if (!bAnyOverlap)
            {
                break;
            }

            const int32 Steps = FMath::Max(1, FMath::FloorToInt(MaxRequiredUp / StepY) + 1);
            ModShiftY -= (float)Steps * StepY;

            if (++OverlapSafetyIters > 2000)
            {
                break;
            }
        }

        // После финального ModShiftY создаём виджеты и добавляем прямоугольники в общий список
        for (const FPendingMiniGrid& P : PendingMiniGrids)
        {
            const FVector2D FinalPos(P.Pos.X, P.Pos.Y + ModShiftY);

            const FName GridName(*FString::Printf(TEXT("ArmorModGrid_%d_%d"), ArmorModGrids.Num(), P.MiniIndex));
            UCanvasPanel* Grid = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), GridName);
            if (!Grid)
            {
                continue;
            }

            ArmorModGrids.Add(Grid);
            ArmorModGridSizes.Add(P.SizePx);
            ArmorModGridModData.Add(ModData);
            ArmorModGridMiniIndices.Add(P.MiniIndex);

            PlacedMiniGridRects.Add(FBox2D(FinalPos, FinalPos + P.SizePx));

            if (UCanvasPanelSlot* GS = RightPanelRef->AddChildToCanvas(Grid))
            {
                GS->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
                GS->SetAlignment(FVector2D(0.f, 0.f));
                GS->SetPosition(FinalPos);
                GS->SetSize(P.SizePx);
                GS->SetZOrder(5);
            }

            if (UBorder* GridBackground = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass()))
            {
                GridBackground->SetBrushColor(FLinearColor(1.f, 1.f, 1.f, 0.05f));
                if (UCanvasPanelSlot* BgSlot = Grid->AddChildToCanvas(GridBackground))
                {
                    BgSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
                    BgSlot->SetOffsets(FMargin(0.f));
                }
            }

            for (int32 Row = 0; Row < P.Cells.Y; ++Row)
            {
                for (int32 Col = 0; Col < P.Cells.X; ++Col)
                {
                    UCanvasPanel* CellContainer = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass());
                    if (CellContainer)
                    {
                        UBorder* CellBg = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
                        if (CellBg)
                        {
                            CellBg->SetBrushColor(FLinearColor(1.f, 1.f, 1.f, 0.05f));
                            if (UCanvasPanelSlot* BgSlot = CellContainer->AddChildToCanvas(CellBg))
                            {
                                BgSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
                                BgSlot->SetOffsets(FMargin(1.f));
                            }
                        }
                        if (UCanvasPanelSlot* CellSlot = Grid->AddChildToCanvas(CellContainer))
                        {
                            CellSlot->SetPosition(FVector2D(Col * CellSize, Row * CellSize));
                            CellSlot->SetSize(FVector2D((float)CellSize, (float)CellSize));
                        }
                    }
                }
            }
        }

        // Обновляем X‑оффсет выбранной строки для следующего мода в этой строке.
        const float NewRowNextX = ModBaseX + MaxLocalX;
        if (RowNextXPtr)
        {
            *RowNextXPtr = NewRowNextX;
        }
        else
        {
            ModRowToNextX.Add(VisualRow, NewRowNextX);
        }
    }
}

void UInventoryWidget::UpdateArmorModGrids()
{
    static bool bInsideArmorModUpdate = false;
    if (bInsideArmorModUpdate)
    {
        return;
    }

    if (ArmorModGrids.Num() == 0 || ArmorModGridModData.Num() != ArmorModGrids.Num() || ArmorModGridMiniIndices.Num() != ArmorModGrids.Num())
        return;

    APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(GetOwningPlayerPawn());
    if (!PlayerChar)
        return;

    const float CellSize = 60.f;

    bInsideArmorModUpdate = true;

    for (int32 GridIdx = 0; GridIdx < ArmorModGrids.Num(); ++GridIdx)
    {
        UEquipModBaseItemData* ModData = ArmorModGridModData[GridIdx];
        UCanvasPanel* Grid = ArmorModGrids[GridIdx];
        if (!ModData || !Grid)
            continue;

        for (UWidget* W : Grid->GetAllChildren())
        {
            if (UInventoryItemWidget* IW = Cast<UInventoryItemWidget>(W))
            {
                if (IW->ItemData)
                    ItemToWidget.Remove(IW->ItemData);
                W->RemoveFromParent();
            }
        }
    }

    for (int32 GridIdx = 0; GridIdx < ArmorModGrids.Num(); ++GridIdx)
    {
        UEquipModBaseItemData* ModData = ArmorModGridModData[GridIdx];
        UCanvasPanel* Grid = ArmorModGrids[GridIdx];
        const int32 MiniIndex = ArmorModGridMiniIndices.IsValidIndex(GridIdx) ? ArmorModGridMiniIndices[GridIdx] : 0;
        if (!ModData || !Grid)
            continue;

        for (UInventoryItemData* Data : ModData->PersistentStorage)
        {
            if (!Data) continue;
            int32 ItemGridIdx = INDEX_NONE;
            FIntPoint LocalCell(0, 0);
            if (!GetEquipModPlacement(ModData, Data, ItemGridIdx, LocalCell))
                continue;
            if (ItemGridIdx != MiniIndex)
                continue;
            const FIntPoint GridSize = EquipModMiniGridSizeByIndex(ModData, MiniIndex);
            bool bRot = false;
            GetEquipModRotation(ModData, Data, bRot);
            const int32 SX = bRot ? FMath::Max(1, Data->SizeInCellsY) : FMath::Max(1, Data->SizeInCellsX);
            const int32 SY = bRot ? FMath::Max(1, Data->SizeInCellsX) : FMath::Max(1, Data->SizeInCellsY);

            UInventoryItemWidget* ItemWidget = WidgetTree->ConstructWidget<UInventoryItemWidget>(UInventoryItemWidget::StaticClass());
            if (ItemWidget)
            {
                ItemWidget->bRotated = bRot;
                ItemWidget->Init(Data, Data->Icon, FVector2D(60.f, 60.f));
                if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Grid->AddChildToCanvas(ItemWidget)))
                {
                    CanvasSlot->SetAnchors(FAnchors(0, 0, 0, 0));
                    CanvasSlot->SetAlignment(FVector2D(0, 0));
                    CanvasSlot->SetPosition(FVector2D(LocalCell.X * CellSize, LocalCell.Y * CellSize));
                    CanvasSlot->SetSize(FVector2D(SX * CellSize, SY * CellSize));
                }
                ItemToWidget.Add(Data, ItemWidget);
            }
        }
    }

    bInsideArmorModUpdate = false;
}

void UInventoryWidget::UpdateVestGrid()
{
    // ВАЖНО: не спамим debug и не пересоздаём гриды каждый апдейт — это сильно лагало при экипировке предметов с хранилищами.
    
    // Не выходим, если VestGridRef == nullptr — ниже создадим грид при необходимости
    
    // Получаем персонажа и экипированный жилет
    APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(GetOwningPlayerPawn());
    if (!PlayerChar || !PlayerChar->InventoryComponent) 
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("❌ PlayerChar or InventoryComponent is null"));
        }
        return;
    }
    
    UEquippableItemData* EquippedVest = Cast<UEquippableItemData>(PlayerChar->InventoryComponent->GetEquippedItem(Vest));
    
    // (debug вывод убран)
    
    // Управляем гридом жилета
    if (EquippedVest && EquippedVest->bHasAdditionalStorage)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("✅ Жилет найден с дополнительным хранилищем"));
        }
        // Создаём гриды только если их ещё нет. Не пересоздаём, чтобы не удалять содержимое
        if (VestGrids.Num() == 0)
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("🔧 Создаём гриды жилета (однократно)"));
            }
            CreateVestGrid();
                    }
                }
                else
                {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("❌ Жилет не найден или без хранилища"));
        }
        
        if (VestGrids.Num() > 0)
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("🗑️ Удаляем гриды жилета..."));
            }
            DestroyVestGrid();
        }
        // Гриды модов брони: при отсутствии жилета показываем гриды модов брони в зоне жилета
        UEquippableItemData* EquippedArmorNoVest = Cast<UEquippableItemData>(PlayerChar->InventoryComponent->GetEquippedItem(Armor));
        int32 ExpectedArmorModPanelCount = 0;
        if (EquippedArmorNoVest)
        {
            for (const FInstalledArmorMod& Inst : EquippedArmorNoVest->InstalledMods)
            {
                UEquipModBaseItemData* ModData = Inst.ModItemData ? Cast<UEquipModBaseItemData>(Inst.ModItemData) : nullptr;
                if (!ModData || !ModData->bHasAdditionalStorage) continue;
                ExpectedArmorModPanelCount += (ModData->AdditionalGrids.Num() > 0) ? ModData->AdditionalGrids.Num() : 1;
            }
        }
        if (ExpectedArmorModPanelCount > 0)
        {
            if (ArmorModGrids.Num() != ExpectedArmorModPanelCount)
            {
                DestroyArmorModGrids();
                CreateArmorModGrids();
            }
            UpdateArmorModGrids();
        }
        else
        {
            DestroyArmorModGrids();
        }
        return; // Если жилет не экипирован, выходим
    }
    
    // Получаем предметы из хранилища жилета
    TArray<UInventoryItemData*> VestItems = PlayerChar->InventoryComponent->GetEquipmentStorageItems(EquippedVest);

    // Определяем текущий лэйаут и пересоздаём гриды ТОЛЬКО если он изменился
    TArray<FIntPoint> GridSizes = EquippedVest->AdditionalGrids;
    TArray<FIntPoint> GridOffsets = EquippedVest->AdditionalGridOffsets;
    if (GridSizes.Num() == 0)
    {
        GridSizes = {
            FIntPoint(1, 1),
            FIntPoint(1, 1),
            FIntPoint(1, 2),
            FIntPoint(1, 2),
            FIntPoint(1, 1),
            FIntPoint(1, 1)
        };
    }
    if (GridOffsets.Num() != GridSizes.Num())
    {
        GridOffsets.SetNum(GridSizes.Num());
        float XCells = 0.f;
        for (int32 i = 0; i < GridSizes.Num(); ++i)
        {
            GridOffsets[i] = FIntPoint(FMath::RoundToInt(XCells), 0);
            XCells += (float)FMath::Max(1, GridSizes[i].X);
        }
    }
    const uint32 LayoutHash = HashGridLayout(GridSizes, GridOffsets);
    const bool bNeedRebuild = (VestGrids.Num() != GridSizes.Num()) || (CachedVestLayoutHash != LayoutHash);
    if (bNeedRebuild)
    {
        DestroyVestGrid();
        CreateVestGrid();
        CachedVestLayoutHash = LayoutHash;
    }
    else
    {
        // Лэйаут тот же — чистим только предметы, не фон/ячейки
        for (UCanvasPanel* G : VestGrids)
        {
            if (!G) continue;
            TArray<UWidget*> Children = G->GetAllChildren();
            for (UWidget* W : Children)
            {
                if (Cast<UInventoryItemWidget>(W))
                {
                    W->RemoveFromParent();
                }
            }
        }
    }
    ItemToWidget.Empty();

    // Размещаем предметы в независимые мини‑гриды жилета.
    // Источник правды: StoredGridByItem + StoredCellByItem (локальные координаты внутри мини‑грида).
    for (UInventoryItemData* Data : VestItems)
    {
        if (!Data) continue;
        RemoveExistingItemWidget(Data);
        bool bPlaced = false;
        if (EquippedVest)
        {
            int32 GridIdx = INDEX_NONE;
            FIntPoint LocalCell(0, 0);
            if (!GetVestPlacement(EquippedVest, Data, GridIdx, LocalCell))
            {
                continue;
            }
            const int32 MaxGrids = (EquippedVest->AdditionalGrids.Num() > 0) ? EquippedVest->AdditionalGrids.Num() : 6;
            if (GridIdx < 0 || GridIdx >= MaxGrids) continue;

            const FIntPoint GridSize = VestMiniGridSizeByIndex(EquippedVest, GridIdx);

            bool bRot = false;
            GetVestRotation(EquippedVest, Data, bRot);

            const int32 SX = bRot ? FMath::Max(1, Data->SizeInCellsY) : FMath::Max(1, Data->SizeInCellsX);
            const int32 SY = bRot ? FMath::Max(1, Data->SizeInCellsX) : FMath::Max(1, Data->SizeInCellsY);

            // Проверка, что предмет влезает в конкретный мини‑грид
            if (LocalCell.X < 0 || LocalCell.Y < 0) continue;
            if (LocalCell.X + SX > GridSize.X || LocalCell.Y + SY > GridSize.Y) continue;

            // Проверка занятости/наложений (с учётом поворотов и размеров)
            if (!IsAreaFreeInVestMiniGrid(EquippedVest, GridIdx, LocalCell.X, LocalCell.Y, SX, SY, Data))
            {
                continue;
            }

            const int32 VestLocalIdx = FMath::Clamp(GridIdx, 0, VestGrids.Num()-1);
            UInventoryItemWidget* ItemWidget = WidgetTree->ConstructWidget<UInventoryItemWidget>(UInventoryItemWidget::StaticClass());
            if (ItemWidget)
            {
                ItemWidget->bRotated = bRot;
                ItemWidget->Init(Data, Data->Icon, FVector2D(60.f, 60.f));
                if (UCanvasPanel* TargetGrid = VestGrids[VestLocalIdx])
                {
                    if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(TargetGrid->AddChildToCanvas(ItemWidget)))
                    {
                        CanvasSlot->SetAnchors(FAnchors(0,0,0,0));
                        CanvasSlot->SetAlignment(FVector2D(0,0));
                        const FVector2D CellSize(60.f, 60.f);
                        CanvasSlot->SetPosition(FVector2D(LocalCell.X * CellSize.X, LocalCell.Y * CellSize.Y));
                        CanvasSlot->SetSize(FVector2D(CellSize.X * SX, CellSize.Y * SY));
                    }

                    // Нормализуем сохранение в новый формат (grid+local cell), чтобы убрать старые "глобальные" значения
                    EquippedVest->StoredGridByItem.Add(Data, GridIdx);
                    EquippedVest->PersistentGridByItem.Add(Data, GridIdx);
                    EquippedVest->StoredCellByItem.Add(Data, LocalCell);
                    EquippedVest->PersistentCellByItem.Add(Data, LocalCell);
                    EquippedVest->StoredRotByItem.Add(Data, bRot);
                    EquippedVest->PersistentRotByItem.Add(Data, bRot);

                    ItemToWidget.Add(Data, ItemWidget);
                    bPlaced = true;
                }
            }
        }
        if (!bPlaced)
        {
            // Нет сохранённой позиции — вообще не раскладываем автоматически, чтобы не портить порядок.
            // Игрок сам положит предмет; UI не создаёт лишних виджетов.
        }
    }

    // Гриды модов брони (в зоне жилета): создаём/обновляем или удаляем
    UEquippableItemData* EquippedArmor = Cast<UEquippableItemData>(PlayerChar->InventoryComponent->GetEquippedItem(Armor));
    int32 ExpectedArmorModPanelCountWithVest = 0;
    if (EquippedArmor)
    {
        for (const FInstalledArmorMod& Inst : EquippedArmor->InstalledMods)
        {
            UEquipModBaseItemData* ModData = Inst.ModItemData ? Cast<UEquipModBaseItemData>(Inst.ModItemData) : nullptr;
            if (!ModData || !ModData->bHasAdditionalStorage) continue;
            ExpectedArmorModPanelCountWithVest += (ModData->AdditionalGrids.Num() > 0) ? ModData->AdditionalGrids.Num() : 1;
        }
    }
    if (ExpectedArmorModPanelCountWithVest > 0)
    {
        if (ArmorModGrids.Num() != ExpectedArmorModPanelCountWithVest)
        {
            DestroyArmorModGrids();
            CreateArmorModGrids();
        }
        UpdateArmorModGrids();
    }
    else
    {
        DestroyArmorModGrids();
    }
}


void UInventoryWidget::ForceClearVestGrids()
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
            FString::Printf(TEXT("🗑️ ForceClearVestGrids called, destroying %d grids"), VestGrids.Num()));
    }
    
    // Принудительно удаляем все гриды жилета
    DestroyVestGrid();
    
    // Принудительно обновляем UI после удаления гридов
    if (RightPanelRef)
    {
        RightPanelRef->InvalidateLayoutAndVolatility();
    }
    
    // Также обновляем весь виджет
    InvalidateLayoutAndVolatility();
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("✅ All vest grids force cleared and UI updated"));
    }
}

void UInventoryWidget::AddVestGridItemIcon(UInventoryItemData* ItemData, int32 Index)
{
    if (!ItemData || VestGrids.Num() == 0) return;
    
    UInventoryItemWidget* ItemWidget = WidgetTree->ConstructWidget<UInventoryItemWidget>(UInventoryItemWidget::StaticClass());
    if (!ItemWidget) return;
    ItemWidget->Init(ItemData, ItemData->Icon, FVector2D(60.f, 60.f));
    
    // Do not place visually here to avoid duplicate/override; actual placement happens in UpdateVestGrid
    ItemToWidget.Add(ItemData, ItemWidget);
}

bool UInventoryWidget::CanDropOnVestGrid(const FGeometry& Geometry, const FVector2D& ScreenPosition) const
{
    // Функция временно отключена для работы с 6 отдельными гридами
    return false;
    
    // Проверяем, находится ли курсор над гридом жилета
    FVector2D LocalPosition = Geometry.AbsoluteToLocal(ScreenPosition);
    
    // Получаем позицию и размер грида жилета
    FVector2D VestGridPosition = FVector2D(70.f, 300.f);
    FVector2D VestGridSizeLocal = FVector2D(360.f, 120.f);
    
    return LocalPosition.X >= VestGridPosition.X && 
           LocalPosition.X <= VestGridPosition.X + VestGridSizeLocal.X &&
           LocalPosition.Y >= VestGridPosition.Y && 
           LocalPosition.Y <= VestGridPosition.Y + VestGridSizeLocal.Y;
}

bool UInventoryWidget::HandleVestGridDrop(UInventoryItemData* ItemData, const FVector2D& Position)
{
    // Функция временно отключена для работы с 6 отдельными гридами
    return false;
    
    // Получаем экипированный жилет
    APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(GetOwningPlayerPawn());
    if (!PlayerChar || !PlayerChar->InventoryComponent) return false;
    
    UEquippableItemData* EquippedVest = Cast<UEquippableItemData>(PlayerChar->InventoryComponent->GetEquippedItem(Vest));
    if (!EquippedVest || !EquippedVest->bHasAdditionalStorage) return false;
    
    // Вычисляем позицию в ячейках грида
    FVector2D VestGridPosition = FVector2D(70.f, 300.f);
    FVector2D LocalPosition = Position - VestGridPosition;
    const int32 CellSize = 60;
    
    int32 CellX = FMath::FloorToInt(LocalPosition.X / CellSize);
    int32 CellY = FMath::FloorToInt(LocalPosition.Y / CellSize);
    
    // Проверяем границы
    if (CellX < 0 || CellX >= 6 || CellY < 0 || CellY >= 2) return false;
    
    // Проверяем, помещается ли предмет
    if (CellX + ItemData->SizeInCellsX > 6 || CellY + ItemData->SizeInCellsY > 2) return false;
    
    // Проверяем, свободны ли ячейки
    for (int32 dy = 0; dy < ItemData->SizeInCellsY; ++dy)
    {
        for (int32 dx = 0; dx < ItemData->SizeInCellsX; ++dx)
        {
            if (CellY + dy >= 2 || CellX + dx >= 6) return false;
            // if (VestOccupiedCells[CellY + dy][CellX + dx]) return false;
        }
    }
    
    // Добавляем предмет в хранилище жилета
    bool bSuccess = PlayerChar->InventoryComponent->AddToEquipmentStorage(EquippedVest, ItemData);
    
    if (bSuccess)
    {
        // Отмечаем ячейки как занятые
        for (int32 dy = 0; dy < ItemData->SizeInCellsY; ++dy)
        {
            for (int32 dx = 0; dx < ItemData->SizeInCellsX; ++dx)
            {
                // VestOccupiedCells[CellY + dy][CellX + dx] = true;
            }
        }
        
        // Обновляем грид
        UpdateVestGrid();
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                FString::Printf(TEXT("✅ Added '%s' to vest grid at (%d,%d)"), 
                    *ItemData->DisplayName.ToString(), CellX, CellY));
        }
    }
    
    return bSuccess;
}

bool UInventoryWidget::HandleVestGridItemDrag(UInventoryItemData* ItemData)
{
    if (!ItemData) return false;
    
    // Получаем экипированный жилет
    APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(GetOwningPlayerPawn());
    if (!PlayerChar || !PlayerChar->InventoryComponent) return false;
    
    UEquippableItemData* EquippedVest = Cast<UEquippableItemData>(PlayerChar->InventoryComponent->GetEquippedItem(Vest));
    if (!EquippedVest || !EquippedVest->bHasAdditionalStorage) return false;
    
    // Удаляем предмет из хранилища жилета
    bool bSuccess = PlayerChar->InventoryComponent->RemoveFromEquipmentStorage(EquippedVest, ItemData);
    
    if (bSuccess)
    {
        // Добавляем предмет обратно в рюкзак
        PlayerChar->InventoryComponent->AddToBackpack(ItemData);
        
        // Обновляем UI
        SyncBackpack(PlayerChar->InventoryComponent->BackpackItems);
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
                FString::Printf(TEXT("🔄 Moved '%s' from vest to backpack"), 
                    *ItemData->DisplayName.ToString()));
        }
    }
    
    return bSuccess;
}

FVector2D UInventoryWidget::GetVestGridCellPosition(int32 CellX, int32 CellY) const
{
    FVector2D VestGridPosition = FVector2D(70.f, 200.f); // Новая позиция над поясом
    const int32 CellSize = 60;
    
    return VestGridPosition + FVector2D(CellX * CellSize, CellY * CellSize);
}

void UInventoryWidget::OpenEquipModStorage(UEquipModBaseItemData* ModItem)
{
    if (!ModItem) return;
    if (!ModItem->bHasAdditionalStorage) return;

    OpenedEquipMod = ModItem;

    UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetTree ? WidgetTree->RootWidget : nullptr);
    if (!RootCanvas) return;

    // ВАЖНО: окно хранилища появляется поверх инвентаря БЕЗ затемнения фона.
    if (EquipModBackdrop)
    {
        EquipModBackdrop->SetVisibility(ESlateVisibility::Collapsed);
    }

    // Window
    if (!EquipModWindowBorder)
    {
        EquipModWindowBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("EquipModWindow"));
        EquipModWindowBorder->SetBrushColor(FLinearColor(0.02f, 0.02f, 0.02f, 1.f));
        if (UCanvasPanelSlot* S = RootCanvas->AddChildToCanvas(EquipModWindowBorder))
        {
            S->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
            S->SetAlignment(FVector2D(0.f, 0.f));
            S->SetPosition(FVector2D(0.f, 0.f));
            S->SetSize(FVector2D(200.f, 200.f)); // будет пересчитано в Update
            S->SetZOrder(9001);
        }

        EquipModWindowCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("EquipModWindowCanvas"));
        EquipModWindowBorder->SetContent(EquipModWindowCanvas);

        // Close button (X)
        UButton* CloseBtn = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("EquipModCloseBtn"));
        UTextBlock* CloseTxt = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("EquipModCloseTxt"));
        CloseTxt->SetText(FText::FromString(TEXT("X")));
        CloseTxt->SetColorAndOpacity(FLinearColor::White);
        CloseBtn->AddChild(CloseTxt);
        CloseBtn->OnClicked.Clear();
        CloseBtn->OnClicked.AddDynamic(this, &UInventoryWidget::HandleEquipModCloseClicked);

        if (UCanvasPanelSlot* CS = Cast<UCanvasPanelSlot>(EquipModWindowCanvas->AddChildToCanvas(CloseBtn)))
        {
            CS->SetAnchors(FAnchors(1.f, 0.f, 1.f, 0.f));
            CS->SetAlignment(FVector2D(1.f, 0.f));
            CS->SetPosition(FVector2D(-8.f, 8.f));
            CS->SetSize(FVector2D(24.f, 24.f));
            CS->SetZOrder(100);
        }
    }
    else
    {
        EquipModWindowBorder->SetVisibility(ESlateVisibility::Visible);
    }

    UpdateEquipModStorageGrid();
}

void UInventoryWidget::HandleEquipModCloseClicked()
{
    CloseEquipModStorage();
}

void UInventoryWidget::CloseEquipModStorage()
{
    // Убираем grid areas, чтобы не перехватывать дроп/hover после закрытия
    GridAreas.RemoveAll([](const FGridArea& A) { return A.Name.StartsWith(TEXT("модуль")); });

    OpenedEquipMod = nullptr;
    CachedEquipModLayoutHash = 0;

    // Удаляем мини‑гриды
    for (UCanvasPanel* G : EquipModGrids)
    {
        if (G) G->RemoveFromParent();
    }
    EquipModGrids.Empty();
    EquipModGridSizes.Empty();

    // Скрываем окно и фон (не уничтожаем, чтобы открытие было быстрым)
    if (EquipModWindowBorder) EquipModWindowBorder->SetVisibility(ESlateVisibility::Collapsed);
    if (EquipModBackdrop) EquipModBackdrop->SetVisibility(ESlateVisibility::Collapsed);
}

void UInventoryWidget::RefreshOpenedEquipModStorage()
{
    if (OpenedEquipMod && EquipModWindowBorder && EquipModWindowBorder->GetVisibility() == ESlateVisibility::Visible)
    {
        UpdateEquipModStorageGrid();
    }
}

void UInventoryWidget::UpdateEquipModStorageGrid()
{
    if (!OpenedEquipMod || !OpenedEquipMod->bHasAdditionalStorage) return;
    if (!EquipModWindowCanvas || !EquipModWindowBorder) return;

    static bool bInsideUpdate = false;
    if (bInsideUpdate) return;
    bInsideUpdate = true;

    UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetTree ? WidgetTree->RootWidget : nullptr);
    if (!RootCanvas)
    {
        bInsideUpdate = false;
        return;
    }

    const int32 CellSizePx = 60;
    const int32 GridGapPx = 6;
    const float WindowPaddingPx = 12.f;
    const float HeaderH = 36.f; // место под крестик/рамку

    // Поддержка нескольких гридов: если AdditionalGrids пустой — один грид 1x1; иначе используем массив (с лимитом и размерами)
    const int32 MaxModGrids = 12;
    const int32 MaxCellsPerDim = 8;
    TArray<FIntPoint> GridSizes = OpenedEquipMod->AdditionalGrids;
    TArray<FIntPoint> GridOffsets = OpenedEquipMod->AdditionalGridOffsets;

    if (GridSizes.Num() == 0)
    {
        GridSizes.Add(FIntPoint(1, 1));
        GridOffsets.Add(FIntPoint(0, 0));
    }
    if (GridOffsets.Num() != GridSizes.Num())
    {
        GridOffsets.SetNum(GridSizes.Num());
        for (int32 i = 0; i < GridSizes.Num(); ++i)
            GridOffsets[i] = FIntPoint(i, 0);
    }
    if (GridSizes.Num() > MaxModGrids)
        GridSizes.SetNum(MaxModGrids);
    for (FIntPoint& Sz : GridSizes)
    {
        Sz.X = FMath::Clamp(FMath::Max(1, Sz.X), 1, MaxCellsPerDim);
        Sz.Y = FMath::Clamp(FMath::Max(1, Sz.Y), 1, MaxCellsPerDim);
    }

    const uint32 LayoutHash = HashGridLayout(GridSizes, GridOffsets);

    const bool bNeedRebuild = (EquipModGrids.Num() != GridSizes.Num()) || (LayoutHash != CachedEquipModLayoutHash);
    if (bNeedRebuild)
    {
        // удаляем старые гриды
        for (UCanvasPanel* G : EquipModGrids)
        {
            if (G) G->RemoveFromParent();
        }
        EquipModGrids.Empty();
        EquipModGridSizes.Empty();
        GridAreas.RemoveAll([](const FGridArea& A) { return A.Name.StartsWith(TEXT("модуль")); });
    }
    else
    {
        // лэйаут тот же — чистим только предметы
        for (UCanvasPanel* G : EquipModGrids)
        {
            if (!G) continue;
            TArray<UWidget*> Children = G->GetAllChildren();
            for (UWidget* W : Children)
            {
                if (Cast<UInventoryItemWidget>(W))
                {
                    W->RemoveFromParent();
                }
            }
        }
    }

    // Табличная раскладка для мини‑гридов
    TMap<int32, float> ColStartPx;
    TMap<int32, float> RowStartPx;
    BuildTableLayoutPx(GridSizes, GridOffsets, CellSizePx, GridGapPx, ColStartPx, RowStartPx);

    // Считаем экстенты окна по пикселям
    float MaxX = 0.f;
    float MaxY = 0.f;
    for (int32 i = 0; i < GridSizes.Num(); ++i)
    {
        const FIntPoint Sz(FMath::Max(1, GridSizes[i].X), FMath::Max(1, GridSizes[i].Y));
        const FIntPoint Off = GridOffsets.IsValidIndex(i) ? GridOffsets[i] : FIntPoint(0, 0);
        const float X0 = ColStartPx.Contains(Off.X) ? ColStartPx[Off.X] : 0.f;
        const float Y0 = RowStartPx.Contains(Off.Y) ? RowStartPx[Off.Y] : 0.f;
        MaxX = FMath::Max(MaxX, X0 + (float)(Sz.X * CellSizePx));
        MaxY = FMath::Max(MaxY, Y0 + (float)(Sz.Y * CellSizePx));
    }

    const FVector2D WindowSize(WindowPaddingPx * 2.f + MaxX, WindowPaddingPx * 2.f + HeaderH + MaxY);
    const FVector2D ScreenSize = RootCanvas->GetCachedGeometry().GetLocalSize();
    const FVector2D WindowPos = FVector2D(
        FMath::Max(0.f, (ScreenSize.X - WindowSize.X) * 0.5f),
        FMath::Max(0.f, (ScreenSize.Y - WindowSize.Y) * 0.5f)
    );

    if (UCanvasPanelSlot* WS = Cast<UCanvasPanelSlot>(EquipModWindowBorder->Slot))
    {
        WS->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
        WS->SetAlignment(FVector2D(0.f, 0.f));
        WS->SetPosition(WindowPos);
        WS->SetSize(WindowSize);
    }

    auto AddModMiniGrid = [&](int32 GridIdx, const FVector2D& LocalPos, const FIntPoint& Cells)
    {
        const FString Label = FString::Printf(TEXT("модуль%d"), GridIdx + 1);
        const FName GridName(*FString::Printf(TEXT("EquipModGrid_%s"), *Label));
        UCanvasPanel* Grid = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), GridName);
        if (!Grid) return;

        const FVector2D SizePx((float)(Cells.X * CellSizePx), (float)(Cells.Y * CellSizePx));
        EquipModGrids.Add(Grid);
        EquipModGridSizes.Add(SizePx);

        if (UCanvasPanelSlot* GS = Cast<UCanvasPanelSlot>(EquipModWindowCanvas->AddChildToCanvas(Grid)))
        {
            GS->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
            GS->SetAlignment(FVector2D(0.f, 0.f));
            GS->SetPosition(LocalPos);
            GS->SetSize(SizePx);
            GS->SetZOrder(10);
        }

        if (UBorder* GridBackground = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass()))
        {
            GridBackground->SetBrushColor(FLinearColor(1.f, 1.f, 1.f, 0.05f));
            GridBackground->SetVisibility(ESlateVisibility::HitTestInvisible);
            if (UCanvasPanelSlot* BgSlot = Grid->AddChildToCanvas(GridBackground))
            {
                BgSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
                BgSlot->SetOffsets(FMargin(0.0f));
                BgSlot->SetZOrder(0);
            }
        }

        // Регистрируем область мини‑грида для dnd/снаппинга в координатах корня
        const FVector2D RootPos = WindowPos + LocalPos;
        RegisterGrid(Label, RootPos, SizePx, Cells.X, Cells.Y);

        // Рисуем сетку
        for (int32 yCell = 0; yCell < Cells.Y; ++yCell)
        {
            for (int32 xCell = 0; xCell < Cells.X; ++xCell)
            {
                UBorder* Cell = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
                if (!Cell) continue;
                Cell->SetBrushColor(FLinearColor(1.f, 1.f, 1.f, 0.06f));
                Cell->SetVisibility(ESlateVisibility::HitTestInvisible);
                if (UCanvasPanelSlot* CellSlot = Grid->AddChildToCanvas(Cell))
                {
                    CellSlot->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
                    CellSlot->SetAlignment(FVector2D(0.f, 0.f));
                    CellSlot->SetPosition(FVector2D(xCell * 60.f + 1.f, yCell * 60.f + 1.f));
                    CellSlot->SetSize(FVector2D(58.f, 58.f));
                    CellSlot->SetZOrder(1);
                }
            }
        }
    };

    if (bNeedRebuild)
    {
        for (int32 i = 0; i < GridSizes.Num(); ++i)
        {
            const FIntPoint Sz(FMath::Max(1, GridSizes[i].X), FMath::Max(1, GridSizes[i].Y));
            const FIntPoint Off = GridOffsets.IsValidIndex(i) ? GridOffsets[i] : FIntPoint(0, 0);
            const float X0 = ColStartPx.Contains(Off.X) ? ColStartPx[Off.X] : 0.f;
            const float Y0 = RowStartPx.Contains(Off.Y) ? RowStartPx[Off.Y] : 0.f;
            const FVector2D LocalPos = FVector2D(WindowPaddingPx + X0, WindowPaddingPx + HeaderH + Y0);
            AddModMiniGrid(i, LocalPos, Sz);
        }
        CachedEquipModLayoutHash = LayoutHash;
    }

    // Отрисовываем предметы
    const FVector2D CellSize(60.f, 60.f);
    for (UInventoryItemData* It : OpenedEquipMod->PersistentStorage)
    {
        if (!It) continue;
        UInventoryItemWidget* W = WidgetTree->ConstructWidget<UInventoryItemWidget>(UInventoryItemWidget::StaticClass());
        W->Init(It, It->Icon, CellSize);
        W->SetVisibility(ESlateVisibility::Visible);
        W->SetIsEnabled(true);

        int32 GridIdx = 0;
        FIntPoint LocalCell(0, 0);
        GetEquipModPlacement(OpenedEquipMod, It, GridIdx, LocalCell);

        bool bRot = false;
        GetEquipModRotation(OpenedEquipMod, It, bRot);

        UCanvasPanel* TargetGrid = EquipModGrids.IsValidIndex(GridIdx) ? EquipModGrids[GridIdx] : (EquipModGrids.Num() > 0 ? EquipModGrids[0] : nullptr);
        if (TargetGrid)
        {
            if (UCanvasPanelSlot* S = Cast<UCanvasPanelSlot>(TargetGrid->AddChildToCanvas(W)))
            {
                S->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
                S->SetAlignment(FVector2D(0.f, 0.f));
                S->SetPosition(FVector2D(LocalCell.X * CellSize.X, LocalCell.Y * CellSize.Y));
                const int32 SX = bRot ? FMath::Max(1, It->SizeInCellsY) : FMath::Max(1, It->SizeInCellsX);
                const int32 SY = bRot ? FMath::Max(1, It->SizeInCellsX) : FMath::Max(1, It->SizeInCellsY);
                W->bRotated = bRot;
                S->SetSize(FVector2D(CellSize.X * SX, CellSize.Y * SY));
                S->SetZOrder(2);
            }
        }

        // Нормализуем (храним и runtime, и persistent одинаково)
        OpenedEquipMod->StoredGridByItem.Add(It, GridIdx);
        OpenedEquipMod->PersistentGridByItem.Add(It, GridIdx);
        OpenedEquipMod->StoredCellByItem.Add(It, LocalCell);
        OpenedEquipMod->PersistentCellByItem.Add(It, LocalCell);
        OpenedEquipMod->StoredRotByItem.Add(It, bRot);
        OpenedEquipMod->PersistentRotByItem.Add(It, bRot);
    }
    bInsideUpdate = false;
}

void UInventoryWidget::OpenArmorModWindow(UEquippableItemData* ArmorItem)
{
    if (!ArmorItem) return;
    OpenedArmorForMods = ArmorItem;

    UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetTree ? WidgetTree->RootWidget : nullptr);
    if (!RootCanvas) return;

    // Создаём окно, если ещё не создано
    if (!ArmorModWindowBorder)
    {
        ArmorModWindowBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("ArmorModWindow"));
        ArmorModWindowBorder->SetBrushColor(FLinearColor(0.02f, 0.02f, 0.02f, 0.95f));
        if (UCanvasPanelSlot* S = RootCanvas->AddChildToCanvas(ArmorModWindowBorder))
        {
            S->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
            S->SetAlignment(FVector2D(0.f, 0.f));
            S->SetPosition(FVector2D(200.f, 120.f));
            S->SetSize(FVector2D(500.f, 400.f));
            S->SetZOrder(9500);
        }

        ArmorModWindowCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("ArmorModWindowCanvas"));
        ArmorModWindowBorder->SetContent(ArmorModWindowCanvas);

        // Кнопка закрытия (X)
        UButton* CloseBtn = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("ArmorModCloseBtn"));
        UTextBlock* CloseTxt = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ArmorModCloseTxt"));
        CloseTxt->SetText(FText::FromString(TEXT("X")));
        CloseTxt->SetColorAndOpacity(FLinearColor::White);
        CloseBtn->AddChild(CloseTxt);
        CloseBtn->OnClicked.Clear();
        CloseBtn->OnClicked.AddDynamic(this, &UInventoryWidget::CloseArmorModWindow);

        if (UCanvasPanelSlot* CS = Cast<UCanvasPanelSlot>(ArmorModWindowCanvas->AddChildToCanvas(CloseBtn)))
        {
            CS->SetAnchors(FAnchors(1.f, 0.f, 1.f, 0.f));
            CS->SetAlignment(FVector2D(1.f, 0.f));
            CS->SetPosition(FVector2D(-8.f, 8.f));
            CS->SetSize(FVector2D(24.f, 24.f));
            CS->SetZOrder(100);
        }

        // Область превью (3D-модель через RenderTarget)
        ArmorModImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("ArmorModImage"));
        if (UCanvasPanelSlot* IS = Cast<UCanvasPanelSlot>(ArmorModWindowCanvas->AddChildToCanvas(ArmorModImage)))
        {
            IS->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
            IS->SetAlignment(FVector2D(0.f, 0.f));
            IS->SetPosition(FVector2D(12.f, 36.f));
            IS->SetSize(FVector2D(-24.f, -48.f));
            IS->SetZOrder(1);
        }

        // Подсказка: вращение по трём осям
        UTextBlock* RotateHint = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ArmorModRotateHint"));
        RotateHint->SetText(FText::FromString(TEXT("Превью: ЛКМ/ПКМ — поворот; ЛКМ+ПКМ — сдвиг; Ctrl+драг — крен. Колесо — приближение/удаление камеры. Тяните край окна — размер.")));
        RotateHint->SetColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.85f, 0.85f, 0.9f)));
        if (UCanvasPanelSlot* HS = Cast<UCanvasPanelSlot>(ArmorModWindowCanvas->AddChildToCanvas(RotateHint)))
        {
            HS->SetAnchors(FAnchors(0.5f, 1.f, 0.5f, 1.f));
            HS->SetAlignment(FVector2D(0.5f, 1.f));
            HS->SetPosition(FVector2D(-180.f, -28.f));
            HS->SetSize(FVector2D(360.f, 22.f));
            HS->SetZOrder(10);
        }
    }

    ArmorModWindowBorder->SetVisibility(ESlateVisibility::Visible);
    ArmorModPreviewScale = 1.f;
    if (ArmorModImage)
    {
        ArmorModImage->SetRenderTransform(FWidgetTransform());
    }

    // Уничтожаем предыдущий превью-актор, если был
    if (ArmorModPreviewActor)
    {
        ArmorModPreviewActor->Destroy();
        ArmorModPreviewActor = nullptr;
    }
    ArmorModRenderTarget = nullptr;

    // Гарантируем, что в данных есть EquippedMesh, иначе превью будет пустым.
    if (!ArmorItem->EquippedMesh || !ArmorItem->EquippedMesh->IsValidLowLevel())
    {
        // Пытаемся подхватить дефолтный меш бронежилета
        const FString FallbackPath = TEXT("/Game/BackToZaraysk/Core/Items/Equipment/SK_Bulletproof_Bege.SK_Bulletproof_Bege");
        if (USkeletalMesh* FallbackSk = LoadObject<USkeletalMesh>(nullptr, *FallbackPath))
        {
            ArmorItem->EquippedMesh = FallbackSk;
        }
        else
        {
            // Последний шанс — куб из Engine
            const FString CubePath = TEXT("/Engine/BasicShapes/Cube.Cube");
            if (UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, *CubePath))
            {
                ArmorItem->EquippedMesh = CubeMesh;
            }
        }
    }

    UWorld* World = GetWorld();
    if (World && ArmorModImage)
    {
        // Создаём RenderTarget (владелец — виджет)
        ArmorModRenderTarget = NewObject<UTextureRenderTarget2D>(this, UTextureRenderTarget2D::StaticClass(), NAME_None, RF_Transient);
        if (ArmorModRenderTarget)
        {
            ArmorModRenderTarget->RenderTargetFormat = RTF_RGBA8;
            ArmorModRenderTarget->InitAutoFormat(512, 512);
            ArmorModRenderTarget->UpdateResource();
        }

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        // Размещаем актор далеко от игрового мира, чтобы не мешать
        FVector SpawnLoc(10000.f, 10000.f, 0.f);
        AArmorModPreviewActor* Preview = World->SpawnActor<AArmorModPreviewActor>(SpawnLoc, FRotator::ZeroRotator, SpawnParams);
        if (Preview && ArmorModRenderTarget)
        {
            ArmorModPreviewActor = Preview;
            
            // Отображаем визуал объекта (меш бронежилета из EquippedMesh)
            Preview->SetMeshFromEquipped(ArmorItem->EquippedMesh);
            // Визуальная сетка для установки модулей (из броника или по умолчанию из bounds)
            Preview->SetModGrids(ArmorItem->ModSideGrids);
            Preview->SetInstalledMods(ArmorItem);
            if (GEngine && ArmorItem->EquippedMesh)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow,
                    FString::Printf(TEXT("🔍 Окно модификации: %s"), *ArmorItem->EquippedMesh->GetName()));
            }
            
            // Сохраняем указатель на RT в локальную переменную для захвата в лямбде
            UTextureRenderTarget2D* LocalRT = ArmorModRenderTarget;
            
            // Устанавливаем RT сразу
            Preview->SetRenderTarget(LocalRT);
            
            // Обновляем изображение в UI сразу
            FSlateBrush Brush;
            Brush.SetResourceObject(LocalRT);
            Brush.ImageSize = FVector2D(512.f, 512.f);
            if (ArmorModImage)
            {
                ArmorModImage->SetBrush(Brush);
                ArmorModImage->SetRenderTransform(FWidgetTransform());
            }
            
            // Небольшая задержка перед обновлением UI, чтобы RT успел захватить сцену
            World->GetTimerManager().SetTimerForNextTick([this, Preview, LocalRT]()
            {
                if (Preview && LocalRT && ArmorModImage)
                {
                    // Обновляем изображение в UI после захвата
                    FSlateBrush Brush;
                    Brush.SetResourceObject(LocalRT);
                    Brush.ImageSize = FVector2D(512.f, 512.f);
                    ArmorModImage->SetBrush(Brush);
                    ArmorModImage->SetRenderTransform(FWidgetTransform());
                    
                    // Принудительно обновляем виджет
                    ArmorModImage->InvalidateLayoutAndVolatility();
                    
                    // Еще одно обновление через кадр для гарантии
                    if (UWorld* World = GetWorld())
                    {
                        World->GetTimerManager().SetTimerForNextTick([this, LocalRT]()
                        {
                            if (ArmorModImage && LocalRT)
                            {
                                FSlateBrush Brush;
                                Brush.SetResourceObject(LocalRT);
                                Brush.ImageSize = FVector2D(512.f, 512.f);
                                ArmorModImage->SetBrush(Brush);
                                ArmorModImage->InvalidateLayoutAndVolatility();
                            }
                        });
                    }
                }
            });
        }
        else
        {
            if (Preview) Preview->Destroy();
            ArmorModPreviewActor = nullptr;
            // Fallback: иконка предмета
            UTexture2D* Icon = ArmorItem->Icon
                ? ArmorItem->Icon
                : LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture"));
            FSlateBrush Brush;
            Brush.SetResourceObject(Icon);
            Brush.ImageSize = FVector2D(256.f, 256.f);
            ArmorModImage->SetBrush(Brush);
        }
    }
}

void UInventoryWidget::CloseArmorModWindow()
{
    bArmorModInstallWaitingClick = false;
    ArmorModContextMenuModIndex = -1;
    ClearArmorModHighlight();
    OpenedArmorForMods = nullptr;
    bDraggingArmorModWindow = false;
    bResizingArmorModWindow = false;
    bRotatingArmorPreview = false;

    if (ArmorModPreviewActor)
    {
        ArmorModPreviewActor->Destroy();
        ArmorModPreviewActor = nullptr;
    }
    ArmorModRenderTarget = nullptr;

    if (ArmorModWindowBorder)
    {
        ArmorModWindowBorder->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UInventoryWidget::ClearArmorModHighlight()
{
    if (ArmorModPreviewActor)
    {
        TArray<AArmorModPreviewActor::FHighlightCell> Empty;
        ArmorModPreviewActor->SetHighlightCells(Empty);
    }
}

bool UInventoryWidget::AnyModCanInstallAtArmorCell(int32 Side, int32 CellX, int32 CellY) const
{
    if (!OpenedArmorForMods || !OpenedArmorForMods->bIsModdable) return false;
    APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(GetOwningPlayerPawn());
    if (!PlayerChar || !PlayerChar->InventoryComponent) return false;
    UInventoryComponent* Inv = PlayerChar->InventoryComponent;
    auto CheckMod = [this, Side, CellX, CellY](UInventoryItemData* Item) -> bool
    {
        if (!Item) return false;
        UEquipModBaseItemData* Mod = Cast<UEquipModBaseItemData>(Item);
        if (!Mod) return false;
        FIntPoint Foot = UEquippableItemData::GetModFootprint(Item);
        return OpenedArmorForMods->CanInstallModAt(Side, CellX, CellY, Foot);
    };
    for (UInventoryItemData* Item : Inv->BackpackItems)
        if (CheckMod(Item)) return true;
    for (int32 p = 1; p <= 4; ++p)
    {
        TArray<TObjectPtr<UInventoryItemData>>* Pocket = nullptr;
        switch (p) { case 1: Pocket = &Inv->Pocket1Items; break; case 2: Pocket = &Inv->Pocket2Items; break; case 3: Pocket = &Inv->Pocket3Items; break; case 4: Pocket = &Inv->Pocket4Items; break; }
        if (Pocket) for (UInventoryItemData* Item : *Pocket) if (CheckMod(Item)) return true;
    }
    if (UEquippableItemData* EquippedVest = Inv->GetEquippedItem(EEquipmentSlotType::Vest))
        if (EquippedVest->bHasAdditionalStorage)
            for (UInventoryItemData* Item : EquippedVest->PersistentStorage)
                if (CheckMod(Item)) return true;
    return false;
}

void UInventoryWidget::TryInstallModOnArmor(UInventoryItemData* ModItem, int32 Side, int32 CellX, int32 CellY)
{
    if (!ModItem || !OpenedArmorForMods || !OpenedArmorForMods->bIsModdable) return;
    FIntPoint Foot = UEquippableItemData::GetModFootprint(ModItem);
    if (!OpenedArmorForMods->CanInstallModAt(Side, CellX, CellY, Foot)) return;
    APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(GetOwningPlayerPawn());
    if (!PlayerChar || !PlayerChar->InventoryComponent) return;
    UInventoryComponent* Inv = PlayerChar->InventoryComponent;
    Inv->RemoveFromAnyStorage(ModItem);
    if (!OpenedArmorForMods->InstallMod(ModItem, Side, CellX, CellY)) return;
    if (UEquipmentComponent* Equip = PlayerChar->FindComponentByClass<UEquipmentComponent>())
        Equip->RefreshEquippedArmorMods();
    RefreshInventoryUI();
    ClearArmorModHighlight();
    if (ArmorModPreviewActor && OpenedArmorForMods)
        ArmorModPreviewActor->SetInstalledMods(OpenedArmorForMods);
}

bool UInventoryWidget::TryInstallModOnArmorFromWaiting(UInventoryItemData* ModItem)
{
    if (!bArmorModInstallWaitingClick || !ModItem || !OpenedArmorForMods) return false;
    UEquipModBaseItemData* Mod = Cast<UEquipModBaseItemData>(ModItem);
    if (!Mod) return false;
    FIntPoint Foot = UEquippableItemData::GetModFootprint(ModItem);
    if (!OpenedArmorForMods->CanInstallModAt(ArmorModInstallSide, ArmorModInstallCellX, ArmorModInstallCellY, Foot)) return false;
    TryInstallModOnArmor(ModItem, ArmorModInstallSide, ArmorModInstallCellX, ArmorModInstallCellY);
    bArmorModInstallWaitingClick = false;
    return true;
}

void UInventoryWidget::OnArmorModContextInstallClicked()
{
    bArmorModInstallWaitingClick = true;
    UCanvasPanel* Root = Cast<UCanvasPanel>(WidgetTree ? WidgetTree->RootWidget : nullptr);
    if (Root) for (UWidget* W : Root->GetAllChildren()) if (W && W->GetFName() == TEXT("ContextMenu")) { Root->RemoveChild(W); break; }
}

void UInventoryWidget::OnArmorModContextUninstallClicked()
{
    if (!OpenedArmorForMods || ArmorModContextMenuModIndex < 0) return;
    APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(GetOwningPlayerPawn());
    if (!PlayerChar || !PlayerChar->InventoryComponent) return;
    PlayerChar->InventoryComponent->UninstallArmorMod(OpenedArmorForMods, ArmorModContextMenuModIndex);
    ArmorModContextMenuModIndex = -1;
    RefreshInventoryUI();
    ClearArmorModHighlight();
    if (ArmorModPreviewActor && OpenedArmorForMods)
        ArmorModPreviewActor->SetInstalledMods(OpenedArmorForMods);
    UCanvasPanel* Root = Cast<UCanvasPanel>(WidgetTree ? WidgetTree->RootWidget : nullptr);
    if (Root) for (UWidget* W : Root->GetAllChildren()) if (W && W->GetFName() == TEXT("ContextMenu")) { Root->RemoveChild(W); break; }
}

// Функции для работы с рюкзаком


