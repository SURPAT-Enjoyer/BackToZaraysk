#include "InventoryWidget.h"
#include "InventoryItemData.h"
#include "EquippableItemData.h"
#include "EquipmentSlotType.h"
#include "BackToZaraysk/Characters/PlayerCharacter.h"
#include "BackToZaraysk/Inventory/InventoryComponent.h"
#include "BackToZaraysk/Inventory/EquippableItemData.h"
#include "BackToZaraysk/Characters/PlayerCharacter.h"
#include "BackToZaraysk/Inventory/InventoryItemWidget.h"
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
#include "BackToZaraysk/Characters/PlayerCharacter.h"
#include "BackToZaraysk/Inventory/InventoryItemWidget.h"
#include "BackToZaraysk/Inventory/InventoryComponent.h"

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
            // Карманы: вручную создаём четыре отдельных грида 1x1 по линии слева направо
            {
                const float PocketsTop = 430.f;
                const float StartX = 70.f;
                const float StepX = 70.f; // 60 + 10 отступ
                auto AddPocket = [&](const TCHAR* Name, float X)
                {
                    UCanvasPanel* Grid = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), FName(Name));
                    // Контейнер кармана не должен перехватывать хиты, только его дети (иконки предметов)
                    Grid->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
                    if (UCanvasPanelSlot* GS = RightPanel->AddChildToCanvas(Grid))
                    {
                        GS->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
                        GS->SetAlignment(FVector2D(0.f, 0.f));
                        GS->SetPosition(FVector2D(X, PocketsTop));
                        GS->SetSize(FVector2D(60.f, 60.f));
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
                        const FVector2D RootPos = RightBase->GetPosition() + FVector2D(X, PocketsTop);
                        RegisterGrid(Name, RootPos, FVector2D(60.f, 60.f), 1, 1);
                    }
                };
                AddPocket(TEXT("карман1"), StartX + StepX * 0);
                AddPocket(TEXT("карман2"), StartX + StepX * 1);
                AddPocket(TEXT("карман3"), StartX + StepX * 2);
                AddPocket(TEXT("карман4"), StartX + StepX * 3);
            }
            
            // Инициализируем систему отслеживания занятых ячеек
            InitializeOccupiedCells();
            
            // Инициализируем занятые ячейки для 6 гридов жилета (горизонтальная конфигурация)
            VestOccupiedCells.SetNum(6); // 6 гридов
            // Гриды 1-2: 1x1
            for (int32 GridIndex = 0; GridIndex < 2; ++GridIndex)
            {
                VestOccupiedCells[GridIndex].SetNum(1); // 1 ряд
                VestOccupiedCells[GridIndex][0].SetNum(1); // 1 колонка
                VestOccupiedCells[GridIndex][0][0] = false;
            }
            // Гриды 3-4: 1x2
            for (int32 GridIndex = 2; GridIndex < 4; ++GridIndex)
            {
                VestOccupiedCells[GridIndex].SetNum(2); // 2 ряда
                for (int32 Row = 0; Row < 2; ++Row)
                {
                    VestOccupiedCells[GridIndex][Row].SetNum(1); // 1 колонка
                    VestOccupiedCells[GridIndex][Row][0] = false;
                }
            }
            // Гриды 5-6: 1x1
            for (int32 GridIndex = 4; GridIndex < 6; ++GridIndex)
            {
                VestOccupiedCells[GridIndex].SetNum(1); // 1 ряд
                VestOccupiedCells[GridIndex][0].SetNum(1); // 1 колонка
                VestOccupiedCells[GridIndex][0][0] = false;
            }
            
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

    // Получаем экипированный рюкзак и отображаем его в слоте
    if (APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(GetOwningPlayerPawn()))
    {
        if (UInventoryComponent* InvComp = PlayerChar->InventoryComponent)
        {
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
        
        // Очищаем существующие виджеты предметов
        TArray<UWidget*> GridChildren = PocketGrid->GetAllChildren();
        for (UWidget* Child : GridChildren)
        {
            if (Child && Child->IsA<UInventoryItemWidget>())
            {
                PocketGrid->RemoveChild(Child);
            }
        }
        
        // Добавляем предметы из кармана
        const TArray<TObjectPtr<UInventoryItemData>>& PocketItems = *PocketArrays[PocketIndex];
        for (UInventoryItemData* Item : PocketItems)
        {
            if (!Item) continue;
            
            UInventoryItemWidget* ItemWidget = WidgetTree->ConstructWidget<UInventoryItemWidget>(UInventoryItemWidget::StaticClass());
            ItemWidget->Init(Item, Item->Icon, FVector2D(60.f, 60.f));
            ItemWidget->SetVisibility(ESlateVisibility::Visible);
            ItemWidget->SetIsEnabled(true);
            
            if (UCanvasPanelSlot* ItemSlot = Cast<UCanvasPanelSlot>(PocketGrid->AddChildToCanvas(ItemWidget)))
            {
                ItemSlot->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
                ItemSlot->SetAlignment(FVector2D(0.f, 0.f));
                ItemSlot->SetPosition(FVector2D::ZeroVector);
                ItemSlot->SetSize(FVector2D(60.f, 60.f));
            }
        }
    }
}

void UInventoryWidget::UpdateBackpackStorageGrid()
{
    // Работать будем относительно корневого Canvas, чтобы координаты совпадали с hit-test
    UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetTree ? WidgetTree->RootWidget : nullptr);
    if (!RootCanvas) return;

    // Удаляем старый грид, если есть
    // Удаляем старый Canvas по имени
    {
        TArray<UWidget*> ToRemove;
        const int32 NumChildren = RootCanvas->GetChildrenCount();
        for (int32 i = 0; i < NumChildren; ++i)
        {
            if (UWidget* C = RootCanvas->GetChildAt(i))
            {
                if (C->GetFName() == TEXT("BackpackStorageCanvas"))
                {
                    ToRemove.Add(C);
                }
            }
        }
        for (UWidget* R : ToRemove)
        {
            RootCanvas->RemoveChild(R);
        }
        BackpackStorageGrid = nullptr;
    }

    // Проверяем, экипирован ли рюкзак и есть ли у него хранилище
    if (APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(GetOwningPlayerPawn()))
    {
        if (UInventoryComponent* InvComp = PlayerChar->InventoryComponent)
        {
            if (UEquippableItemData* EquippedBackpack = InvComp->GetEquippedItem(Backpack))
            {
                // Всегда создаём видимую область грида для рюкзака, даже если флаг не выставлен
                const int32 GridX = (EquippedBackpack->AdditionalGridSize.X > 0 ? EquippedBackpack->AdditionalGridSize.X : 8);
                const int32 GridY = (EquippedBackpack->AdditionalGridSize.Y > 0 ? EquippedBackpack->AdditionalGridSize.Y : 6);

                // Рисуем контейнер под грид
                UCanvasPanel* GridCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("BackpackStorageCanvas"));
                if (GridCanvas)
                {
                    GridCanvas->SetVisibility(ESlateVisibility::Visible);
                    GridCanvas->SetIsEnabled(true);
                }
                // Абсолютная позиция верхней левой ячейки рюкзака (смещено на 270px влево): (821, 689)
                const FVector2D GridTopLeft = FVector2D(821.f, 689.f);

                if (UCanvasPanelSlot* GS = Cast<UCanvasPanelSlot>(RootCanvas->AddChildToCanvas(GridCanvas)))
                {
                    GS->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
                    GS->SetAlignment(FVector2D(0.f, 0.f));
                    // Размещаем грид рюкзака по абсолютным координатам экрана
                    GS->SetPosition(GridTopLeft);
                    GS->SetSize(FVector2D(60.f * GridX, 60.f * GridY));
                    GS->SetZOrder(15);
                }
                BackpackStorageGrid = nullptr;

                // Добавляем фон в стиле пояса для визуализации области грида
                if (UBorder* GridBackground = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("BackpackGridBackground")))
                {
                    GridBackground->SetBrushColor(FLinearColor(1.f,1.f,1.f,0.05f));
                    GridBackground->SetVisibility(ESlateVisibility::HitTestInvisible);
                    if (UCanvasPanelSlot* BgSlot = GridCanvas->AddChildToCanvas(GridBackground))
                    {
                        BgSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
                        BgSlot->SetOffsets(FMargin(0.0f));
                        BgSlot->SetZOrder(0);
                    }
                }

                // Регистрируем область грида для обработки dnd и снаппинга в координатах корня
                RegisterGrid(TEXT("рюкзак"), GridTopLeft, FVector2D(60.f * GridX, 60.f * GridY), GridX, GridY);

                // Рисуем видимую сетку ячеек 60x60 с 1px зазором между клетками, чтобы образовалась сетка
                for (int32 yCell = 0; yCell < GridY; ++yCell)
                {
                    for (int32 xCell = 0; xCell < GridX; ++xCell)
                    {
                        const FName CellName(*FString::Printf(TEXT("BackpackCell_r%d_c%d"), yCell, xCell));
                        UBorder* Cell = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), CellName);
                        if (Cell)
                        {
                            // Немного более светлый цвет, чтобы отличаться от синего фона
                            Cell->SetBrushColor(FLinearColor(1.f, 1.f, 1.f, 0.06f));
                            Cell->SetVisibility(ESlateVisibility::HitTestInvisible);
                            if (UCanvasPanelSlot* CellSlot = GridCanvas->AddChildToCanvas(Cell))
                            {
                                CellSlot->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
                                CellSlot->SetAlignment(FVector2D(0.f, 0.f));
                                // Делаем зазор 1px, чтобы сетка была визуально заметна
                                CellSlot->SetPosition(FVector2D(xCell * 60.f + 1.f, yCell * 60.f + 1.f));
                                CellSlot->SetSize(FVector2D(58.f, 58.f));
                                CellSlot->SetZOrder(1);
                            }
                        }
                    }
                }

                // Отрисовываем предметы из хранилища рюкзака
                TArray<UInventoryItemData*> Stored = InvComp->GetEquipmentStorageItems(EquippedBackpack);
                const FVector2D CellSize(60.f, 60.f);
                int32 curX = 0, curY = 0;
                for (UInventoryItemData* It : Stored)
                {
                    if (!It) continue;
                    UInventoryItemWidget* W = WidgetTree->ConstructWidget<UInventoryItemWidget>(UInventoryItemWidget::StaticClass());
                    W->Init(It, It->Icon, CellSize);
                    W->SetVisibility(ESlateVisibility::Visible);
                    W->SetIsEnabled(true);
                    if (UCanvasPanelSlot* S = Cast<UCanvasPanelSlot>(GridCanvas->AddChildToCanvas(W)))
                    {
                        S->SetAnchors(FAnchors(0.f,0.f,0.f,0.f));
                        S->SetAlignment(FVector2D(0.f,0.f));
                        // Восстанавливаем сохранённую позицию предмета, если она есть
                        if (UEquippableItemData* EquippedBackpackPtr = InvComp->GetEquippedItem(Backpack))
                        {
                            // Сначала пробуем PersistentCellByItem (после выброса/подбора)
                            if (EquippedBackpackPtr->PersistentCellByItem.Contains(It))
                            {
                                const FIntPoint Cell = EquippedBackpackPtr->PersistentCellByItem[It];
                                S->SetPosition(FVector2D(Cell.X * CellSize.X, Cell.Y * CellSize.Y));
                            }
                            else if (EquippedBackpackPtr->StoredCellByItem.Contains(It))
                            {
                                const FIntPoint Cell = EquippedBackpackPtr->StoredCellByItem[It];
                                S->SetPosition(FVector2D(Cell.X * CellSize.X, Cell.Y * CellSize.Y));
                            }
                            else
                            {
                                S->SetPosition(FVector2D(curX * CellSize.X, curY * CellSize.Y));
                            }
                        }
                        else
                        {
                            S->SetPosition(FVector2D(curX * CellSize.X, curY * CellSize.Y));
                        }
                        S->SetSize(FVector2D(CellSize.X * FMath::Max(1, It->SizeInCellsX), CellSize.Y * FMath::Max(1, It->SizeInCellsY)));
                        S->SetZOrder(2);
                    }
                    // Простейшее последовательное размещение
                    curX += FMath::Max(1, It->SizeInCellsX);
                    if (curX >= GridX)
                    {
                        curX = 0;
                        curY += 1;
                        if (curY >= GridY) break;
                    }
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
    if (!BackpackData) return false;
    const int32 GridX = FMath::Max(1, BackpackData->AdditionalGridSize.X);
    const int32 GridY = FMath::Max(1, BackpackData->AdditionalGridSize.Y);
    if (StartCellX < 0 || StartCellY < 0) return false;
    if (StartCellX + SizeX > GridX || StartCellY + SizeY > GridY) return false;

    // Проверяем пересечения с уже размещёнными элементами по StoredCellByItem
    for (const TPair<TObjectPtr<UInventoryItemData>, FIntPoint>& Pair : BackpackData->StoredCellByItem)
    {
        UInventoryItemData* Other = Pair.Key;
        if (!Other || Other == IgnoredItem) continue;
        const FIntPoint OtherCell = Pair.Value;
        const int32 OtherSizeX = FMath::Max(1, Other->SizeInCellsX);
        const int32 OtherSizeY = FMath::Max(1, Other->SizeInCellsY);
        const bool bOverlapX = !(StartCellX + SizeX <= OtherCell.X || OtherCell.X + OtherSizeX <= StartCellX);
        const bool bOverlapY = !(StartCellY + SizeY <= OtherCell.Y || OtherCell.Y + OtherSizeY <= StartCellY);
        if (bOverlapX && bOverlapY)
        {
    return false;
        }
    }
    return true;
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

    // Вычисляем координаты относительно канвы рюкзака
    const FVector2D ScreenPos = InDragDropEvent.GetScreenSpacePosition();
    UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetTree ? WidgetTree->RootWidget : nullptr);
    if (!RootCanvas) return false;

    // Находим зарегистрированную область грида в корневых координатах (для размера/кол-ва ячеек)
    const FVector2D RootLocal = RootCanvas->GetCachedGeometry().AbsoluteToLocal(ScreenPos);
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
                    if ((bIsPocket || bIsVestGrid) &&
                        Child->GetCachedGeometry().IsUnderLocation(ScreenPos))
                    {
                        // Проверка по реальному размеру виджета-ячейки (геометрия)
                        const FVector2D TargetSize = Child->GetCachedGeometry().GetLocalSize();
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
                                    InvComp->MoveItemToPocket(PocketIndex, DraggedWidget->ItemData);
                                }
                                else if (bIsVestGrid)
                                {
                                    InvComp->MoveItemToVest(DraggedWidget->ItemData);
                                }

                                // Reparent visual to target grid
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

    // Определяем зону дропа
    const FGridArea& DropArea = GridAreas[GridIdx];
    // Если дроп в рюкзак — репарентим в канву рюкзака, двигаем и сохраняем позицию
    if (DropArea.Name.Contains(TEXT("рюкзак")))
    {
        // Найти канву рюкзака в корневом канвасе
        UCanvasPanel* BackpackCanvas = nullptr;
        const int32 NumChildren = RootCanvas->GetChildrenCount();
        for (int32 i = 0; i < NumChildren; ++i)
        {
            if (UWidget* C = RootCanvas->GetChildAt(i))
            {
                if (C->GetFName() == TEXT("BackpackStorageCanvas"))
                {
                    BackpackCanvas = Cast<UCanvasPanel>(C);
                    break;
                }
            }
        }
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
            if (!IsAreaFreeInBackpack(EquippedBackpack, CellX, CellY, SX, SY, DraggedWidget->ItemData))
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
            EquippedBackpack->StoredCellByItem.Add(DraggedWidget->ItemData, FIntPoint(CellX, CellY));
            EquippedBackpack->PersistentCellByItem.Add(DraggedWidget->ItemData, FIntPoint(CellX, CellY));
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
    }
    
    if (APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(GetOwningPlayerPawn()))
    {
        if (UInventoryComponent* InvComp = PlayerChar->InventoryComponent)
        {
            if (DropArea.Name.Contains(TEXT("рюкзак")))
            {
                if (UEquippableItemData* EquippedBackpack = InvComp->GetEquippedItem(Backpack))
                {
                    // Удаляем из любых мест, затем добавляем в хранилище рюкзака
                    InvComp->RemoveFromAnyStorage(DraggedWidget->ItemData);
                    InvComp->AddToEquipmentStorage(EquippedBackpack, DraggedWidget->ItemData);
                    EquippedBackpack->StoredCellByItem.Add(DraggedWidget->ItemData, FIntPoint(CellX, CellY));
                    EquippedBackpack->PersistentCellByItem.Add(DraggedWidget->ItemData, FIntPoint(CellX, CellY));
                }
            }
            else if (DropArea.Name.StartsWith(TEXT("жилет")))
            {
                // Валидируем размер и выполняем автоповорот при необходимости
                int32 FitX = DraggedWidget->bRotated ? DraggedWidget->SizeY : DraggedWidget->SizeX;
                int32 FitY = DraggedWidget->bRotated ? DraggedWidget->SizeX : DraggedWidget->SizeY;
                bool bFits = (FitX <= A.CellsX && FitY <= A.CellsY);
                if (!bFits)
                {
                    // Пробуем автоповорот
                    int32 RotX = DraggedWidget->SizeY;
                    int32 RotY = DraggedWidget->SizeX;
                    if (RotX <= A.CellsX && RotY <= A.CellsY)
                    {
                        DraggedWidget->bRotated = !DraggedWidget->bRotated;
                        DraggedWidget->UpdateVisualSize(FVector2D(60.f, 60.f));
                        bFits = true;
                    }
                }
                if (!bFits)
                {
                    return false; // не влезает в грид жилета
                }
                InvComp->MoveItemToVest(DraggedWidget->ItemData);
                // Сохраняем секцию и ячейку в данных жилета (локальный индекс мини‑грида + внутренняя ячейка)
                if (UEquippableItemData* EquippedVest = InvComp->GetEquippedItem(Vest))
                {
                    // Преобразуем глобальный индекс GridIdx в локальный индекс жилета по имени области "жилетN"
                    int32 VestLocalIdx = INDEX_NONE;
                    {
                        FString AreaName = A.Name; // например, "жилет3"
                        if (AreaName.RemoveFromStart(TEXT("жилет")))
                        {
                            const int32 Parsed = FCString::Atoi(*AreaName);
                            if (Parsed > 0) { VestLocalIdx = Parsed - 1; }
                        }
                    }
                    if (VestLocalIdx == INDEX_NONE)
                    {
                        // fallback: если имя не парсится, ограничим глобальный индекс в диапазон мини‑гридов
                        VestLocalIdx = FMath::Clamp(GridIdx, 0, VestGrids.Num() - 1);
                    }

                    // Ячейка внутри мини‑грида (важно для 1x2 — нижняя ячейка Y=1)
                    const int32 InnerCellX = CellX; // для жилета A.CellsX обычно 1
                    const int32 InnerCellY = CellY; // 0 или 1 для секций 1x2

                    EquippedVest->StoredGridByItem.Add(DraggedWidget->ItemData, VestLocalIdx);
                    EquippedVest->PersistentGridByItem.Add(DraggedWidget->ItemData, VestLocalIdx);
                    EquippedVest->StoredCellByItem.Add(DraggedWidget->ItemData, FIntPoint(InnerCellX, InnerCellY));
                    EquippedVest->PersistentCellByItem.Add(DraggedWidget->ItemData, FIntPoint(InnerCellX, InnerCellY));
                }
                UpdateVestGrid();
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
                    // Пробуем автоповорот для 1x2 или 2x1
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

                // Логика хранения
                InvComp->MoveItemToPocket(PocketIndex, DraggedWidget->ItemData);

                    // Визуально: репарентим виджет в соответствующий карман без полного RefreshInventoryUI
                if (RightPanelRef)
                {
                    const FString PocketName = FString::Printf(TEXT("карман%d"), PocketIndex);
                    UCanvasPanel* TargetPocket = nullptr;
                    const int32 Count = RightPanelRef->GetChildrenCount();
                    for (int32 i = 0; i < Count; ++i)
                    {
                        if (UWidget* Child = RightPanelRef->GetChildAt(i))
                        {
                            if (Child->GetFName() == FName(*PocketName))
                            {
                                TargetPocket = Cast<UCanvasPanel>(Child);
                                break;
                            }
                        }
                    }
                    if (TargetPocket)
                    {
                        // Удаляем прежний виджет из любого контейнера, затем добавляем в карман
                        DraggedWidget->RemoveFromParent();
                        if (UCanvasPanelSlot* NewSlot = Cast<UCanvasPanelSlot>(TargetPocket->AddChildToCanvas(DraggedWidget)))
                        {
                            NewSlot->SetAnchors(FAnchors(0.f,0.f,0.f,0.f));
                            NewSlot->SetAlignment(FVector2D(0.f,0.f));
                            NewSlot->SetPosition(FVector2D::ZeroVector);
                            NewSlot->SetSize(FVector2D(60.f, 60.f));
                        }
                        // Сбрасываем подсветку после успешного дропа
                        DraggedWidget->SetTint(FLinearColor(1.f, 1.f, 1.f, 1.f));
                    }
                }
            }
        }
    }
            return true;
        }
    
bool UInventoryWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    // Координаты курсора переводим в систему корневого канваса, т.к. GridAreas зарегистрированы в ней
    UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetTree ? WidgetTree->RootWidget : nullptr);
    if (!RootCanvas) return false;
    const FVector2D RootLocal = RootCanvas->GetCachedGeometry().AbsoluteToLocal(InDragDropEvent.GetScreenSpacePosition());
    const int32 GridIdx = FindGridAtPoint(RootLocal);
    if (GridIdx != INDEX_NONE) return true;
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
    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UInventoryWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    // Legacy function - no longer needed
}

FReply UInventoryWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    const FVector2D ScreenPos = InMouseEvent.GetScreenSpacePosition();
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(12345, 0.f, FColor::Cyan,
            FString::Printf(TEXT("Cursor: (%.1f, %.1f)"), ScreenPos.X, ScreenPos.Y));
    }
    return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
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
    for (int32 i = 0; i < GridAreas.Num(); ++i)
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
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, TEXT("🔍 CreateVestGrid called"));
    }
    
    if (VestGrids.Num() > 0)
    {
        DestroyVestGrid();
    }
    
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
                GS->SetZOrder(5); // рендерим поверх
                // Регистрируем в координатах корневой канвы
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
    
    // Создаем 6 отдельных гридов жилета горизонтально в одну линию
    // Поднимаем грид жилета на 200px (было ~220)
    const float VestY = 20.f;
    // Размещаем внутри координат RightPanel (0..RightPanel.Width). Начнём от 70 как у других гридов
    const float VestXStart = 70.f;
    const float Step = 70.f;        // шаг между секциями (60 + 10)
    AddVestLabeledGrid(TEXT("жилет1"), VestXStart + Step * 0, VestY, FVector2D(60.f, 60.f), 1, 1, nullptr, false);
    AddVestLabeledGrid(TEXT("жилет2"), VestXStart + Step * 1, VestY, FVector2D(60.f, 60.f), 1, 1, nullptr, false);
    AddVestLabeledGrid(TEXT("жилет3"), VestXStart + Step * 2, VestY, FVector2D(60.f, 120.f), 1, 2, nullptr, false);
    AddVestLabeledGrid(TEXT("жилет4"), VestXStart + Step * 3, VestY, FVector2D(60.f, 120.f), 1, 2, nullptr, false);
    AddVestLabeledGrid(TEXT("жилет5"), VestXStart + Step * 4, VestY, FVector2D(60.f, 60.f), 1, 1, nullptr, false);
    AddVestLabeledGrid(TEXT("жилет6"), VestXStart + Step * 5, VestY, FVector2D(60.f, 60.f), 1, 1, nullptr, false);
    
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


void UInventoryWidget::UpdateVestGrid()
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, 
            FString::Printf(TEXT("🔍 UpdateVestGrid called, current grids: %d"), VestGrids.Num()));
    }
    
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
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
            FString::Printf(TEXT("🔍 EquippedVest: %s, HasStorage: %s, CurrentGrids: %d"), 
                EquippedVest ? TEXT("Found") : TEXT("Null"),
                EquippedVest && EquippedVest->bHasAdditionalStorage ? TEXT("Yes") : TEXT("No"),
                VestGrids.Num()));
    }
    
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
        return; // Если жилет не экипирован, выходим
    }
    
    // Получаем предметы из хранилища жилета
    TArray<UInventoryItemData*> VestItems = PlayerChar->InventoryComponent->GetEquipmentStorageItems(EquippedVest);
    
    // Инициализируем массив VestOccupiedCells если он пустой
    if (VestOccupiedCells.Num() == 0)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("🔧 Инициализируем VestOccupiedCells..."));
        }
        
        VestOccupiedCells.SetNum(6); // 6 гридов
        // Гриды 1-2: 1x1
        for (int32 GridIndex = 0; GridIndex < 2; ++GridIndex)
        {
            VestOccupiedCells[GridIndex].SetNum(1); // 1 ряд
            VestOccupiedCells[GridIndex][0].SetNum(1); // 1 колонка
            VestOccupiedCells[GridIndex][0][0] = false;
        }
        // Гриды 3-4: 1x2
        for (int32 GridIndex = 2; GridIndex < 4; ++GridIndex)
        {
            VestOccupiedCells[GridIndex].SetNum(2); // 2 ряда
            for (int32 Row = 0; Row < 2; ++Row)
            {
                VestOccupiedCells[GridIndex][Row].SetNum(1); // 1 колонка
                VestOccupiedCells[GridIndex][Row][0] = false;
            }
        }
        // Гриды 5-6: 1x1
        for (int32 GridIndex = 4; GridIndex < 6; ++GridIndex)
        {
            VestOccupiedCells[GridIndex].SetNum(1); // 1 ряд
            VestOccupiedCells[GridIndex][0].SetNum(1); // 1 колонка
            VestOccupiedCells[GridIndex][0][0] = false;
        }
    }
    
    // Очищаем занятые ячейки для всех 6 гридов (горизонтальная конфигурация)
    for (int32 GridIndex = 0; GridIndex < 6; ++GridIndex)
    {
        // Проверяем, что массив инициализирован
        if (GridIndex < VestOccupiedCells.Num() && VestOccupiedCells[GridIndex].Num() > 0)
        {
            if (GridIndex < 2 || GridIndex >= 4) // Гриды 1-2 и 5-6: 1x1
            {
                if (VestOccupiedCells[GridIndex][0].Num() > 0)
                {
                    VestOccupiedCells[GridIndex][0][0] = false;
                }
            }
            else // Гриды 3-4: 1x2
            {
                if (VestOccupiedCells[GridIndex][0].Num() > 0)
                {
                    VestOccupiedCells[GridIndex][0][0] = false;
                }
                if (VestOccupiedCells[GridIndex].Num() > 1 && VestOccupiedCells[GridIndex][1].Num() > 0)
                {
                    VestOccupiedCells[GridIndex][1][0] = false;
                }
            }
        }
    }
    
    // Перед повторным размещением очищаем все старые виджеты предметов из гридов жилета,
    // чтобы не было дубликатов после закрытия/открытия инвентаря
    for (UCanvasPanel* Grid : VestGrids)
    {
        if (!Grid) continue;
        TArray<UWidget*> Children = Grid->GetAllChildren();
        for (UWidget* W : Children)
        {
            if (UInventoryItemWidget* ItemW = Cast<UInventoryItemWidget>(W))
            {
                Grid->RemoveChild(ItemW);
            }
        }
    }
    // Также чистим маппинг для предметов, которые находились в гриде жилета
    for (auto It = ItemToWidget.CreateIterator(); It; ++It)
    {
        if (It.Value() && VestGrids.Contains(Cast<UCanvasPanel>(It.Value()->GetParent())))
        {
            It.RemoveCurrent();
        }
    }

    // Размещаем предметы: приоритетно по сохранённым координатам из StoredCellByItem/PersistentCellByItem,
    // иначе — последовательной раскладкой
    int32 FallbackIndex = 0;
    for (UInventoryItemData* Data : VestItems)
    {
        if (!Data) continue;
        RemoveExistingItemWidget(Data);
        bool bPlaced = false;
        if (EquippedVest)
        {
            int32* GridPtr = EquippedVest->PersistentGridByItem.Find(Data);
            if (!GridPtr) GridPtr = EquippedVest->StoredGridByItem.Find(Data);
            FIntPoint* CellPtr = EquippedVest->PersistentCellByItem.Find(Data);
            if (!CellPtr) CellPtr = EquippedVest->StoredCellByItem.Find(Data);
            if (GridPtr && *GridPtr >= 0 && *GridPtr < VestGrids.Num())
            {
                // Добавляем в нужный мини‑грид с позицией 0,0 (или будущей ячейкой)
                UInventoryItemWidget* ItemWidget = WidgetTree->ConstructWidget<UInventoryItemWidget>(UInventoryItemWidget::StaticClass());
                if (ItemWidget)
                {
                    ItemWidget->Init(Data, Data->Icon, FVector2D(60.f, 60.f));
                        if (UCanvasPanel* TargetGrid = VestGrids[*GridPtr])
                    {
                        if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(TargetGrid->AddChildToCanvas(ItemWidget)))
                        {
                            CanvasSlot->SetAnchors(FAnchors(0,0,0,0));
                            CanvasSlot->SetAlignment(FVector2D(0,0));
                            // Восстанавливаем внутреннюю ячейку (для 1x2 секций нижняя клетка Y=1)
                            const int32 InnerX = (CellPtr ? CellPtr->X : 0);
                            const int32 InnerY = (CellPtr ? CellPtr->Y : 0);
                            const FVector2D CellSize(60.f, 60.f);
                            CanvasSlot->SetPosition(FVector2D(InnerX * CellSize.X, InnerY * CellSize.Y));
                            CanvasSlot->SetSize(FVector2D(60.f, 60.f));
                        }
                        ItemToWidget.Add(Data, ItemWidget);
                        bPlaced = true;
                    }
                }
            }
        }
        if (!bPlaced)
        {
            AddVestGridItemIcon(Data, FallbackIndex++);
        }
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
    
    // Создаем виджет предмета
    UInventoryItemWidget* ItemWidget = WidgetTree->ConstructWidget<UInventoryItemWidget>(UInventoryItemWidget::StaticClass());
    if (!ItemWidget) return;
    
    // Инициализируем виджет
    ItemWidget->Init(ItemData, ItemData->Icon, FVector2D(60.f, 60.f));
    
    // Размещаем в гриде жилета (простое размещение по порядку)
    const int32 CellSize = 60;
    int32 CellX = Index % 6; // 6 колонок
    int32 CellY = Index / 6; // 2 ряда
    
    // Проверяем, что не выходим за границы
    if (CellY >= 2) return;
    
    // UCanvasPanelSlot* ItemSlot = VestGridRef->AddChildToCanvas(ItemWidget);
    // if (ItemSlot)
    // {
    //     FVector2D ItemSize = FVector2D(ItemData->SizeInCellsX * CellSize, ItemData->SizeInCellsY * CellSize);
    //     ItemSlot->SetPosition(FVector2D(CellX * CellSize, CellY * CellSize));
    //     ItemSlot->SetSize(ItemSize);
    // }
    
    // Отмечаем ячейки как занятые
    for (int32 dy = 0; dy < ItemData->SizeInCellsY; ++dy)
    {
        for (int32 dx = 0; dx < ItemData->SizeInCellsX; ++dx)
        {
            if (CellY + dy < 2 && CellX + dx < 6)
            {
                // VestOccupiedCells[CellY + dy][CellX + dx] = true;
            }
        }
    }
    
    // Сохраняем связь
    ItemToWidget.Add(ItemData, ItemWidget);

    // Обновляем StoredCellByItem для жилета минимально: по индексу кладём в PersistentCellByItem как последовательный слот,
    // чтобы при следующем открытии восстановить расположение хотя бы стабильно
    if (APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(GetOwningPlayerPawn()))
    {
        if (UInventoryComponent* Inv = PlayerChar->InventoryComponent)
        {
            if (UEquippableItemData* VestData = Cast<UEquippableItemData>(Inv->GetEquippedItem(Vest)))
            {
                VestData->StoredCellByItem.Add(ItemData, FIntPoint(CellX, CellY));
                VestData->PersistentCellByItem.Add(ItemData, FIntPoint(CellX, CellY));
            }
        }
    }
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

// Функции для работы с рюкзаком

