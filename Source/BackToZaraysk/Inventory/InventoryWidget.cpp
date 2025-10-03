#include "InventoryWidget.h"
#include "InventoryItemData.h"
#include "EquippableItemData.h"
#include "EquipmentSlotType.h"
#include "BackToZaraysk/Characters/PlayerCharacter.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/PanelWidget.h"
#include "Components/Image.h"
#include "InventoryGridWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Slate/SlateBrushAsset.h"
#include "Styling/SlateColor.h"
#include "Engine/Texture2D.h"
#include "Components/Border.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "BackToZaraysk/Characters/BTZBaseCharacter.h"
#include "BackToZaraysk/Inventory/InventoryItemWidget.h"
#include "BackToZaraysk/Inventory/InventoryComponent.h"

void UInventoryWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // Строим UI один раз на экземпляр (важно для нового PIE сеанса)
    if (bUIBuilt) return;
    bUIBuilt = true;

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
                    RegisterGrid(Label, Pos, GridSize, GridWidth, GridHeight);
                }
            };
            AddLabeledGrid(TEXT("разгрузка"), 0.f, FVector2D(480.f, 240.f), 8, 4, TEXT("разгрузка"), false);
            AddLabeledGrid(TEXT("пояс"), 280.f, FVector2D(480.f, 120.f), 8, 2, TEXT("пояс"), false);
            AddLabeledGrid(TEXT("карманы"), 430.f, FVector2D(240.f, 60.f), 4, 1, nullptr, true);
            // Запомним позицию и размер рюкзака для размещения иконок
            BackpackGridPosition = FVector2D(70.f, 520.f);
            BackpackGridSize = FVector2D(480.f, 220.f);
            AddLabeledGrid(TEXT("рюкзак"), 520.f, BackpackGridSize, 8, 4, TEXT("рюкзак"), false);
            // Найдём индекс рюкзака в массивах областей
            for (int32 i=0;i<GridAreas.Num();++i){ if (GridAreas[i].Name==TEXT("рюкзак")) { BackpackGridIndex=i; break; }}

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

            // Голова: на уровне "лицо", по центру силуэта (справа от "лицо")
            const float HeadY = 150.f;
            AddEquipSlot(TEXT("голова"), FVector2D(SilCenterX - SlotHalfWidth, HeadY)); y+=110.f;

            // Обувь остаётся внизу силуэта (фиксированная позиция)
            const float ShoeY = SilTop + SilHeight - (LabelHeight + BoxHeight); // 720

            // Тело и штаны делят отрезок [голова..обувь] на три равные части
            const float Segment = (ShoeY - HeadY) / 3.f;
            AddEquipSlot(TEXT("тело"), FVector2D(SilCenterX - SlotHalfWidth, HeadY + Segment * 1.f));
            // Перчатки справа от слота "тело" на одном уровне
            AddEquipSlot(TEXT("перчатки"), FVector2D(SilCenterX - SlotHalfWidth + 120.f + 20.f, HeadY + Segment * 1.f));
            AddEquipSlot(TEXT("штаны"), FVector2D(SilCenterX - SlotHalfWidth, HeadY + Segment * 2.f));
            AddEquipSlot(TEXT("обувь"), FVector2D(SilCenterX - SlotHalfWidth, ShoeY));

            // Бронежилет на уровне слота "тело" (по Y как у тела, по X левый столбец)
            AddEquipSlot(TEXT("бронежилет"), FVector2D(60.f, HeadY + Segment * 1.f));
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
            UImage* Backdrop = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("BackdropSimple"));
            Backdrop->SetColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.7f));
            RootPanel->AddChild(Backdrop);
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
                    S->SetSize(FVector2D(BackpackCellSize.X * P.SizeX, BackpackCellSize.Y * P.SizeY));
                }
            }
        }
    }
}

void UInventoryWidget::SetVisible(bool bIsVisible)
{
	bShown = bIsVisible;
    SetVisibility(bShown ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    if (!bShown && WidgetTree && WidgetTree->RootWidget)
    {
        if (UCanvasPanel* Root = Cast<UCanvasPanel>(WidgetTree->RootWidget))
        {
            TArray<UWidget*> Children = Root->GetAllChildren();
            for (UWidget* W : Children)
            {
                if (W && W->GetFName() == TEXT("ContextMenu"))
                {
                    Root->RemoveChild(W);
                    break;
                }
            }
        }
    }
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, bShown ? FColor::Green : FColor::Silver, bShown ? TEXT("Inventory visibility: VISIBLE") : TEXT("Inventory visibility: HIDDEN"));
    }
}



void UInventoryWidget::AddBackpackItemIcon(UInventoryItemData* ItemData)
{
    if (!RightPanelRef) return;
    // Создаём виджет предмета, поддерживающий drag&drop
    class UInventoryItemWidget* ItemW = WidgetTree->ConstructWidget<UInventoryItemWidget>(UInventoryItemWidget::StaticClass());
    UTexture2D* Icon = (ItemData && ItemData->Icon) ? ItemData->Icon : LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture"));
    ItemW->Init(ItemData, Icon, BackpackCellSize);
    const int32 Cols = FMath::Max(1, (int32)(BackpackGridSize.X / BackpackCellSize.X));
    const int32 Index = BackpackCount++;
    const int32 X = Index % Cols;
    const int32 Y = Index / Cols;
    if (UCanvasPanelSlot* S = RightPanelRef->AddChildToCanvas(ItemW))
    {
        S->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
        S->SetAlignment(FVector2D(0.f, 0.f));
        S->SetPosition(BackpackGridPosition + FVector2D(BackpackCellSize.X * X, BackpackCellSize.Y * Y));
        int32 SX = ItemData ? FMath::Max(1, ItemData->SizeInCellsX) : 1;
        int32 SY = ItemData ? FMath::Max(1, ItemData->SizeInCellsY) : 1;
        S->SetSize(FVector2D(BackpackCellSize.X * SX, BackpackCellSize.Y * SY));
        S->SetZOrder(5);
    }
    BackpackIconWidgets.Add(ItemW);
    ItemToWidget.Add(ItemData, ItemW);
}

void UInventoryWidget::SyncBackpack(const TArray<UInventoryItemData*>& Items)
{
    if (!RightPanelRef) return;

    // Обновляем слоты экипировки
    UpdateEquipmentSlots();

    // 1) Удаляем виджеты тех предметов, которых больше нет в данных
    for (auto ItMap = ItemToWidget.CreateIterator(); ItMap; ++ItMap)
    {
        UInventoryItemData* Data = ItMap.Key();
        UInventoryItemWidget* W = ItMap.Value();
        if (!Items.Contains(Data))
        {
            if (W && RightPanelRef)
            {
                RightPanelRef->RemoveChild(W);
            }
            ItMap.RemoveCurrent();
        }
    }

    // 2) Создаем виджеты только для новых предметов. Сохраняем позицию ранее размещенных
    for (UInventoryItemData* It : Items)
    {
        if (ItemToWidget.Contains(It))
        {
            // Уже есть виджет — убедимся, что он в нужном гриде и размере по данным размещения
            UInventoryItemWidget* W = ItemToWidget[It];
            for (const FPlacedItem& P : Placed)
            {
                if (P.Widget == W)
                {
                    if (UCanvasPanelSlot* S = Cast<UCanvasPanelSlot>(W->Slot))
                    {
                        const FGridArea& A = GridAreas[P.GridIndex];
                        const FVector2D CellSize(A.Size.X / FMath::Max(1, A.CellsX), A.Size.Y / FMath::Max(1, A.CellsY));
                        S->SetPosition(A.Position + FVector2D(CellSize.X * P.CellX, CellSize.Y * P.CellY));
                        S->SetSize(FVector2D(BackpackCellSize.X * P.SizeX, BackpackCellSize.Y * P.SizeY));
                    }
                    break;
                }
            }
            continue;
        }

        // Новый предмет — создаём и авторазмещаем
        int32 SX = It ? FMath::Max(1, It->SizeInCellsX) : 1;
        int32 SY = It ? FMath::Max(1, It->SizeInCellsY) : 1;
        int32 CellX, CellY;
        const int32 GridIndex = BackpackGridIndex;
        if (TryAutoPlaceInGrid(GridIndex, SX, SY, CellX, CellY))
        {
            UInventoryItemWidget* ItemW = WidgetTree->ConstructWidget<UInventoryItemWidget>(UInventoryItemWidget::StaticClass());
            UTexture2D* Icon = (It && It->Icon) ? It->Icon : LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture"));
            ItemW->Init(It, Icon, BackpackCellSize);
            const FGridArea& A = GridAreas[GridIndex];
            const FVector2D CellSize(A.Size.X / FMath::Max(1, A.CellsX), A.Size.Y / FMath::Max(1, A.CellsY));
            const FVector2D Pos = A.Position + FVector2D(CellX * CellSize.X, CellY * CellSize.Y);
            if (UCanvasPanelSlot* S = RightPanelRef->AddChildToCanvas(ItemW))
            {
                S->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
                S->SetAlignment(FVector2D(0.f, 0.f));
                S->SetPosition(Pos);
                S->SetSize(FVector2D(BackpackCellSize.X * SX, BackpackCellSize.Y * SY));
                S->SetZOrder(5);
                BackpackIconWidgets.Add(ItemW);
                ItemToWidget.Add(It, ItemW);
                UpsertPlacement(ItemW, GridIndex, CellX, CellY);
            }
        }
    }
}
bool UInventoryWidget::TryAutoPlaceInGrid(int32 GridIndex, int32 SizeX, int32 SizeY, int32& OutCellX, int32& OutCellY) const
{
    if (!GridAreas.IsValidIndex(GridIndex)) return false;
    const FGridArea& A = GridAreas[GridIndex];
    for (int32 y = 0; y < A.CellsY; ++y)
    {
        for (int32 x = 0; x < A.CellsX; ++x)
        {
            if (IsAreaFree(GridIndex, x, y, SizeX, SizeY, nullptr))
            {
                OutCellX = x; OutCellY = y; return true;
            }
        }
    }
    return false;
}

void UInventoryWidget::RegisterGrid(const FString& Name, const FVector2D& Pos, const FVector2D& Size, int32 CellsX, int32 CellsY)
{
    FGridArea A; A.Name=Name; A.Position=Pos; A.Size=Size; A.CellsX=CellsX; A.CellsY=CellsY; GridAreas.Add(A);
}

int32 UInventoryWidget::FindGridAtPoint(const FVector2D& LocalPoint) const
{
    for (int32 i=0;i<GridAreas.Num();++i)
    {
        const FGridArea& A = GridAreas[i];
        const FVector2D P = LocalPoint - A.Position;
        if (P.X>=0 && P.Y>=0 && P.X<A.Size.X && P.Y<A.Size.Y)
        {
            return i;
        }
    }
    return INDEX_NONE;
}

FVector2D UInventoryWidget::SnapToCellInGrid(int32 GridIndex, const FVector2D& LocalPoint) const
{
    const FGridArea& A = GridAreas[GridIndex];
    FVector2D P = LocalPoint - A.Position;
    const FVector2D CellSize(A.Size.X / FMath::Max(1, A.CellsX), A.Size.Y / FMath::Max(1, A.CellsY));
    const int32 X = FMath::Clamp((int32)FMath::FloorToInt(P.X / CellSize.X), 0, A.CellsX-1);
    const int32 Y = FMath::Clamp((int32)FMath::FloorToInt(P.Y / CellSize.Y), 0, A.CellsY-1);
    return A.Position + FVector2D(X * CellSize.X, Y * CellSize.Y);
}

bool UInventoryWidget::IsPointInsideBackpack(const FVector2D& LocalPoint) const
{
    const FVector2D P = LocalPoint - BackpackGridPosition;
    return P.X >= 0.f && P.Y >= 0.f && P.X < BackpackGridSize.X && P.Y < BackpackGridSize.Y;
}

FVector2D UInventoryWidget::SnapToBackpackCell(const FVector2D& LocalPoint) const
{
    FVector2D P = LocalPoint - BackpackGridPosition;
    const int32 MaxX = FMath::Max(0, (int32)(BackpackGridSize.X/BackpackCellSize.X)-1);
    const int32 MaxY = FMath::Max(0, (int32)(BackpackGridSize.Y/BackpackCellSize.Y)-1);
    const int32 X = FMath::Clamp((int32)FMath::FloorToInt(P.X / BackpackCellSize.X), 0, MaxX);
    const int32 Y = FMath::Clamp((int32)FMath::FloorToInt(P.Y / BackpackCellSize.Y), 0, MaxY);
    return BackpackGridPosition + FVector2D(X * BackpackCellSize.X, Y * BackpackCellSize.Y);
}

// Удалён ранний заглушечный обработчик NativeOnDragOver (заменён полноценной реализацией ниже)

bool UInventoryWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    if (!RightPanelRef || !InOperation) return false;
    
    // Переводим в локальные RightPanel координаты
    const FVector2D PanelLocal = RightPanelRef->GetCachedGeometry().AbsoluteToLocal(InDragDropEvent.GetScreenSpacePosition());
    const int32 GridIndex = FindGridAtPoint(PanelLocal);
    if (GridIndex == INDEX_NONE) 
    {
        // Очищаем состояние при неудачном дропе
        if (Highlight) { RightPanelRef->RemoveChild(Highlight); Highlight = nullptr; }
        DragItem = nullptr;
        bDragActive = false;
        return false;
    }
    
    if (UWidget* W = Cast<UWidget>(InOperation->Payload))
    {
        if (UCanvasPanelSlot* S = Cast<UCanvasPanelSlot>(W->Slot))
        {
            DragItem = Cast<UInventoryItemWidget>(W);
            if (!DragItem) return false;
            
            // Запомним стартовое положение на случай отмены
            DragStartPos = S->GetPosition();
            DragStartSize = S->GetSize();
            
            const FVector2D LocalInGrid = PanelLocal - GridAreas[GridIndex].Position;
            const FVector2D CellSize = FVector2D(
                GridAreas[GridIndex].Size.X / FMath::Max(1, GridAreas[GridIndex].CellsX),
                GridAreas[GridIndex].Size.Y / FMath::Max(1, GridAreas[GridIndex].CellsY)
            );
            
            int32 CellX = FMath::FloorToInt(LocalInGrid.X / CellSize.X);
            int32 CellY = FMath::FloorToInt(LocalInGrid.Y / CellSize.Y);
            
            // Ограничиваем координаты границами грида
            CellX = FMath::Clamp(CellX, 0, GridAreas[GridIndex].CellsX - 1);
            CellY = FMath::Clamp(CellY, 0, GridAreas[GridIndex].CellsY - 1);
            
            int32 SX = DragItem->bRotated ? DragItem->SizeY : DragItem->SizeX;
            int32 SY = DragItem->bRotated ? DragItem->SizeX : DragItem->SizeY;
            
            // Проверяем, помещается ли предмет в выбранную позицию
            if (!IsAreaFree(GridIndex, CellX, CellY, SX, SY, DragItem))
            {
                // Попробуем найти ближайшее свободное место
                int32 AutoX, AutoY;
                if (TryAutoPlaceInGrid(GridIndex, SX, SY, AutoX, AutoY))
                {
                    CellX = AutoX; 
                    CellY = AutoY;
                }
                else
                {
                    // Недопустимо — вернуть назад
                    DragItem->SetTint(FLinearColor::White);
                    S->SetPosition(DragStartPos);
                    S->SetSize(DragStartSize);
                    if (Highlight) { RightPanelRef->RemoveChild(Highlight); Highlight = nullptr; }
                    DragItem = nullptr;
                    bDragActive = false;
                    return false;
                }
            }
            
            // Допустимо — размещаем предмет
            const FVector2D FinalPos = GridAreas[GridIndex].Position + FVector2D(CellX * CellSize.X, CellY * CellSize.Y);
            S->SetPosition(FinalPos);
            S->SetSize(FVector2D(BackpackCellSize.X * SX, BackpackCellSize.Y * SY));
            UpsertPlacement(DragItem, GridIndex, CellX, CellY);
            DragItem->SetTint(FLinearColor::White);
            // Обновим связь данных -> виджет, чтобы положение сохранялось при повторном открытии
            if (DragItem->ItemData)
            {
                ItemToWidget.FindOrAdd(DragItem->ItemData) = DragItem;
            }
            
            // Очищаем состояние
            if (Highlight) { RightPanelRef->RemoveChild(Highlight); Highlight = nullptr; }
            DragItem = nullptr;
            bDragActive = false;
            return true;
        }
    }
    
    // Очищаем состояние при неудачном дропе
    if (Highlight) { RightPanelRef->RemoveChild(Highlight); Highlight = nullptr; }
    DragItem = nullptr;
    bDragActive = false;
    return false;
}

bool UInventoryWidget::IsAreaFree(int32 GridIndex, int32 CellX, int32 CellY, int32 SizeX, int32 SizeY, UInventoryItemWidget* Ignore) const
{
    if (!GridAreas.IsValidIndex(GridIndex)) return false;
    const FGridArea& A = GridAreas[GridIndex];
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

void UInventoryWidget::RemoveItemWidget(UInventoryItemWidget* Widget)
{
    if (!Widget) return;
    if (RightPanelRef)
    {
        RightPanelRef->RemoveChild(Widget);
    }
    for (int32 i=Placed.Num()-1;i>=0;--i)
    {
        if (Placed[i].Widget == Widget)
        {
            Placed.RemoveAt(i);
        }
    }
}

void UInventoryWidget::RemoveItemMapping(UInventoryItemData* ItemData)
{
    ItemToWidget.Remove(ItemData);
}

FReply UInventoryWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    // Пропускаем Ctrl/Shift в персонажа, даже когда фокус на инвентаре
    const FKey Key = InKeyEvent.GetKey();
    if (Key == EKeys::LeftControl)
    {
        if (APlayerController* PC = GetOwningPlayer())
        {
            if (ABTZBaseCharacter* Ch = Cast<ABTZBaseCharacter>(PC->GetPawn()))
            {
                if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Cyan, TEXT("UI: Ctrl -> ChangeCrouchState"));
                Ch->ChangeCrouchState();
                return FReply::Handled();
            }
        }
    }
    if (Key == EKeys::LeftShift)
    {
        if (APlayerController* PC = GetOwningPlayer())
        {
            if (ABTZBaseCharacter* Ch = Cast<ABTZBaseCharacter>(PC->GetPawn()))
            {
                if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Cyan, TEXT("UI: Shift -> ChangeProneState"));
                Ch->ChangeProneState();
                return FReply::Handled();
            }
        }
    }
    return HandleItemRotation(InGeometry, InKeyEvent);
}

FReply UInventoryWidget::HandleItemRotation(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    if (InKeyEvent.GetKey() == EKeys::R)
    {
        // Вращение перетаскиваемого предмета
        if (DragItem && DragItem->ItemData && DragItem->ItemData->bRotatable)
        {
            DragItem->bRotated = !DragItem->bRotated;
            DragItem->UpdateVisualSize(BackpackCellSize);
            return FReply::Handled();
        }
        
        // Вращение предмета под курсором
        if (HoverItem && HoverItem->ItemData && HoverItem->ItemData->bRotatable)
        {
            // Находим запись о размещении предмета
            FPlacedItem* PlacedItem = nullptr;
            for (FPlacedItem& P : Placed)
            {
                if (P.Widget == HoverItem)
                {
                    PlacedItem = &P;
                    break;
                }
            }
            
            if (PlacedItem)
            {
                // Временно вращаем предмет
                HoverItem->bRotated = !HoverItem->bRotated;
                int32 SX = HoverItem->bRotated ? HoverItem->SizeY : HoverItem->SizeX;
                int32 SY = HoverItem->bRotated ? HoverItem->SizeX : HoverItem->SizeY;
                
                // Проверяем, помещается ли предмет в новом положении
                if (IsAreaFree(PlacedItem->GridIndex, PlacedItem->CellX, PlacedItem->CellY, SX, SY, HoverItem))
                {
                    // Обновляем запись о размещении
                    PlacedItem->SizeX = SX;
                    PlacedItem->SizeY = SY;
                    PlacedItem->bRotated = HoverItem->bRotated;
                    
                    // Обновляем визуальное представление
                    HoverItem->UpdateVisualSize(BackpackCellSize);
                    if (UCanvasPanelSlot* S = Cast<UCanvasPanelSlot>(HoverItem->Slot))
                    {
                        const FGridArea& A = GridAreas[PlacedItem->GridIndex];
                        const FVector2D CellSize = FVector2D(
                            A.Size.X / FMath::Max(1, A.CellsX),
                            A.Size.Y / FMath::Max(1, A.CellsY)
                        );
                        S->SetSize(FVector2D(BackpackCellSize.X * SX, BackpackCellSize.Y * SY));
                        S->SetPosition(A.Position + FVector2D(CellSize.X * PlacedItem->CellX, CellSize.Y * PlacedItem->CellY));
                    }
                }
                else
                {
                    // Откатываем вращение если не помещается
                    HoverItem->bRotated = !HoverItem->bRotated;
                    HoverItem->UpdateVisualSize(BackpackCellSize);
                }
            }
            return FReply::Handled();
        }
    }
    return FReply::Unhandled();
}

bool UInventoryWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    if (!RightPanelRef || !InOperation) return false;
    
    const FVector2D PanelLocal = RightPanelRef->GetCachedGeometry().AbsoluteToLocal(InDragDropEvent.GetScreenSpacePosition());
    const int32 GridIndex = FindGridAtPoint(PanelLocal);
    UInventoryItemWidget* Item = Cast<UInventoryItemWidget>(InOperation->Payload);
    
    if (GridIndex == INDEX_NONE || !Item) 
    { 
        if (Highlight) { RightPanelRef->RemoveChild(Highlight); Highlight = nullptr; } 
        return false; 
    }

    const FGridArea& A = GridAreas[GridIndex];
    const FVector2D LocalInGrid = PanelLocal - A.Position;
    const FVector2D CellSize = FVector2D(
        A.Size.X / FMath::Max(1, A.CellsX),
        A.Size.Y / FMath::Max(1, A.CellsY)
    );
    
    int32 CellX = FMath::FloorToInt(LocalInGrid.X / CellSize.X);
    int32 CellY = FMath::FloorToInt(LocalInGrid.Y / CellSize.Y);
    
    // Ограничиваем координаты границами грида
    CellX = FMath::Clamp(CellX, 0, A.CellsX - 1);
    CellY = FMath::Clamp(CellY, 0, A.CellsY - 1);
    
    int32 SX = Item->bRotated ? Item->SizeY : Item->SizeX;
    int32 SY = Item->bRotated ? Item->SizeX : Item->SizeY;
    
    bool bFree = IsAreaFree(GridIndex, CellX, CellY, SX, SY, Item);
    int32 AutoX, AutoY;
    if (!bFree && TryAutoPlaceInGrid(GridIndex, SX, SY, AutoX, AutoY))
    {
        bFree = true; 
        CellX = AutoX; 
        CellY = AutoY;
    }

    // Создаём/обновляем подсветку ячеек
    if (!Highlight)
    {
        Highlight = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
        if (UCanvasPanelSlot* HS = RightPanelRef->AddChildToCanvas(Highlight))
        {
            HS->SetZOrder(6);
        }
    }
    
    if (Highlight)
    {
        Highlight->SetBrushColor(bFree ? FLinearColor(0.f, 1.f, 0.f, 0.25f) : FLinearColor(1.f, 0.f, 0.f, 0.25f));
        if (UCanvasPanelSlot* HS = Cast<UCanvasPanelSlot>(Highlight->Slot))
        {
            HS->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f)); 
            HS->SetAlignment(FVector2D(0.f, 0.f));
            const FVector2D FinalPos = A.Position + FVector2D(CellX * CellSize.X, CellY * CellSize.Y);
            HS->SetPosition(FinalPos);
            HS->SetSize(FVector2D(BackpackCellSize.X * SX, BackpackCellSize.Y * SY));
        }
    }
    
    // Тонируем сам предмет
    Item->SetTint(bFree ? FLinearColor(1.f, 1.f, 1.f, 1.f) : FLinearColor(1.f, 0.2f, 0.2f, 1.f));
    DragItem = Item;
    bDragActive = true;
    return true;
}

void UInventoryWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    if (!bDragActive) return;
    if (DragItem)
    {
        if (UCanvasPanelSlot* S = Cast<UCanvasPanelSlot>(DragItem->Slot))
        {
            S->SetPosition(DragStartPos);
            S->SetSize(DragStartSize);
        }
        DragItem->SetTint(FLinearColor::White);
    }
    if (Highlight) { RightPanelRef->RemoveChild(Highlight); Highlight=nullptr; }
    DragItem = nullptr;
    bDragActive = false;
}

void UInventoryWidget::UpdateEquipmentSlots()
{
    if (!EquipmentPanelRef) return;
    
    // Очищаем существующие слоты
    for (TObjectPtr<UWidget> Widget : EquipmentSlotWidgets)
    {
        if (Widget)
        {
            EquipmentPanelRef->RemoveChild(Widget);
        }
    }
    EquipmentSlotWidgets.Empty();
    
    // Получаем компонент инвентаря персонажа
    APlayerController* PC = GetOwningPlayer();
    if (!PC) return;
    
    APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(PC->GetPawn());
    if (!PlayerChar || !PlayerChar->InventoryComponent) return;
    
    UInventoryComponent* InvComp = PlayerChar->InventoryComponent;
    
    // Создаем слоты для каждого типа экипировки
    TArray<EEquipmentSlotType> SlotTypes = {
        EEquipmentSlotType::Helmet,
        EEquipmentSlotType::Vest,
        EEquipmentSlotType::Backpack,
        EEquipmentSlotType::PrimaryWeapon,
        EEquipmentSlotType::SecondaryWeapon
    };
    
    for (int32 i = 0; i < SlotTypes.Num(); i++)
    {
        EEquipmentSlotType SlotType = SlotTypes[i];
        UEquippableItemData* EquippedItem = InvComp->GetEquippedItem(SlotType);
        CreateEquipmentSlotWidget(EquippedItem, i);
    }
}

void UInventoryWidget::CreateEquipmentSlotWidget(const UEquippableItemData* Item, int32 SlotIndex)
{
    if (!EquipmentPanelRef) return;
    
    // Создаем контейнер для слота
    UBorder* SlotContainer = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
    SlotContainer->SetBrushColor(FLinearColor(0.2f, 0.2f, 0.2f, 0.8f));
    
    // Добавляем текст с названием слота
    UTextBlock* SlotLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    TArray<FString> SlotNames = {"Голова", "Разгрузка", "Рюкзак", "Основное", "Вторичное"};
    if (SlotIndex < SlotNames.Num())
    {
        SlotLabel->SetText(FText::FromString(SlotNames[SlotIndex]));
    }
    SlotLabel->SetColorAndOpacity(FLinearColor::White);
    SlotContainer->AddChild(SlotLabel);
    
    // Если есть предмет в слоте, добавляем его иконку
    if (Item && Item->Icon)
    {
        UImage* ItemIcon = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
        FSlateBrush Brush;
        Brush.SetResourceObject(Item->Icon);
        Brush.ImageSize = EquipmentSlotSize;
        ItemIcon->SetBrush(Brush);
        SlotContainer->AddChild(ItemIcon);
    }
    
    // Добавляем слот в панель
    if (UCanvasPanelSlot* CanvasSlot = EquipmentPanelRef->AddChildToCanvas(SlotContainer))
    {
        CanvasSlot->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
        CanvasSlot->SetAlignment(FVector2D(0.f, 0.f));
        CanvasSlot->SetPosition(FVector2D(10.f, 10.f + SlotIndex * (EquipmentSlotSize.Y + 10.f)));
        CanvasSlot->SetSize(EquipmentSlotSize);
    }
    
    EquipmentSlotWidgets.Add(SlotContainer);
}
