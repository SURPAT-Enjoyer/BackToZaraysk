#include "InventoryWidget.h"
#include "InventoryItemData.h"
#include "EquippableItemData.h"
#include "EquipmentSlotType.h"
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
                    RegisterGrid(Label, Pos, GridSize, GridWidth, GridHeight);
                }
            };
            // Убираем грид разгрузки из центральной части
            // AddLabeledGrid(TEXT("разгрузка"), 0.f, FVector2D(480.f, 240.f), 8, 4, TEXT("разгрузка"), false);
            AddLabeledGrid(TEXT("пояс"), 280.f, FVector2D(480.f, 120.f), 8, 2, TEXT("пояс"), false);
            AddLabeledGrid(TEXT("карманы"), 430.f, FVector2D(240.f, 60.f), 4, 1, nullptr, true);
            // Запомним позицию и размер рюкзака для размещения иконок
            BackpackGridPosition = FVector2D(70.f, 520.f);
            BackpackGridSize = FVector2D(480.f, 240.f); // Увеличиваем высоту для 4 строк по 60 пикселей
            AddLabeledGrid(TEXT("рюкзак"), 520.f, BackpackGridSize, 8, 4, TEXT("рюкзак"), false);
            
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
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, 
            FString::Printf(TEXT("🔍 AddBackpackItemIcon: Adding %s"), 
                ItemData ? *ItemData->DisplayName.ToString() : TEXT("Unknown Item")));
    }
    
    // Создаём виджет предмета, поддерживающий drag&drop
    class UInventoryItemWidget* ItemW = WidgetTree->ConstructWidget<UInventoryItemWidget>(UInventoryItemWidget::StaticClass());
    UTexture2D* Icon = (ItemData && ItemData->Icon) ? ItemData->Icon : LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture"));
    ItemW->Init(ItemData, Icon, BackpackCellSize);
    
    // Используем систему восстановления позиций из InventoryComponent
    FVector2D Position;
    int32 CellX = 0, CellY = 0;
    
    // Проверяем, есть ли сохраненная позиция для этого предмета
    APlayerController* PC = GetOwningPlayer();
    if (PC)
    {
        APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(PC->GetPawn());
        if (PlayerChar && PlayerChar->InventoryComponent)
        {
            UInventoryComponent* InvComp = PlayerChar->InventoryComponent;
            
            // Ищем предмет в массиве рюкзака для определения его индекса
            int32 ItemIndex = InvComp->BackpackItems.Find(ItemData);
            if (ItemIndex != INDEX_NONE)
            {
                // Используем индекс предмета для определения позиции
                const int32 Cols = FMath::Max(1, (int32)(BackpackGridSize.X / BackpackCellSize.X));
                CellX = ItemIndex % Cols;
                CellY = ItemIndex / Cols;
                
                Position = BackpackGridPosition + FVector2D(
                    BackpackCellSize.X * CellX, 
                    BackpackCellSize.Y * CellY
                );
                
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                        FString::Printf(TEXT("✅ Using restored position for %s: index=%d, cell=(%d,%d)"), 
                            *ItemData->DisplayName.ToString(), ItemIndex, CellX, CellY));
                }
            }
            else
            {
                // Если индекс не найден, используем поиск свободной позиции
                Position = FindValidPositionInBackpack(ItemData);
                FVector2D RelativePos = Position - BackpackGridPosition;
                CellX = FMath::RoundToInt(RelativePos.X / BackpackCellSize.X);
                CellY = FMath::RoundToInt(RelativePos.Y / BackpackCellSize.Y);
                
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
                        FString::Printf(TEXT("⚠️ Using fallback position for %s: cell=(%d,%d)"), 
                            *ItemData->DisplayName.ToString(), CellX, CellY));
                }
            }
        }
    }
    
    int32 SX = ItemData ? FMath::Max(1, ItemData->SizeInCellsX) : 1;
    int32 SY = ItemData ? FMath::Max(1, ItemData->SizeInCellsY) : 1;
    
    // Отмечаем ячейки как занятые
    MarkCellsAsOccupied(CellX, CellY, SX, SY);
    
    if (UCanvasPanelSlot* S = RightPanelRef->AddChildToCanvas(ItemW))
    {
        S->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
        S->SetAlignment(FVector2D(0.f, 0.f));
        S->SetPosition(Position);
        S->SetSize(FVector2D(BackpackCellSize.X * SX, BackpackCellSize.Y * SY));
        S->SetZOrder(5);
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                FString::Printf(TEXT("✅ Positioned %s at (%f, %f), cells (%d,%d) to (%d,%d)"), 
                    ItemData ? *ItemData->DisplayName.ToString() : TEXT("Unknown Item"), 
                    Position.X, Position.Y, CellX, CellY, CellX + SX - 1, CellY + SY - 1));
        }
    }
    
    BackpackIconWidgets.Add(ItemW);
    ItemToWidget.Add(ItemData, ItemW);
}

void UInventoryWidget::SyncBackpack(const TArray<UInventoryItemData*>& Items)
{
    if (!RightPanelRef) return;

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, 
            FString::Printf(TEXT("🔍 SyncBackpack called with %d items"), Items.Num()));
    }

    // Очищаем все занятые ячейки
    InitializeOccupiedCells();

    // Обновляем статические слоты экипировки
    UpdateStaticEquipmentSlots();
    
    // Обновляем грид жилета
    UpdateVestGrid();

    // 1) Удаляем виджеты тех предметов, которых больше нет в данных
    for (auto ItMap = ItemToWidget.CreateIterator(); ItMap; ++ItMap)
    {
        UInventoryItemData* Data = ItMap.Key();
        UInventoryItemWidget* W = ItMap.Value();
        if (!Items.Contains(Data))
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, 
                    FString::Printf(TEXT("🗑️ Removing widget for item: %s"), *Data->DisplayName.ToString()));
            }
            
            // Не удаляем виджеты из статических слотов экипировки
            if (W && RightPanelRef && !W->bIsStaticEquipmentSlot)
            {
                RightPanelRef->RemoveChild(W);
            }
            ItMap.RemoveCurrent();
        }
    }

    // 2) Создаем виджеты только для новых предметов. Сохраняем позицию ранее размещенных
    for (UInventoryItemData* It : Items)
    {
        // Пропускаем экипированные предметы - они не должны отображаться в инвентаре
        UEquippableItemData* EquippableItem = Cast<UEquippableItemData>(It);
        if (EquippableItem)
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, 
                    FString::Printf(TEXT("🔍 Item %s: bIsEquipped = %s"), *It->DisplayName.ToString(), EquippableItem->bIsEquipped ? TEXT("true") : TEXT("false")));
            }
            
            if (EquippableItem->bIsEquipped)
            {
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, 
                        FString::Printf(TEXT("⏭️ Skipping equipped item in inventory: %s"), *It->DisplayName.ToString()));
                }
                continue;
            }
        }
        
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
    
    // Проверяем, не пытаемся ли мы поместить предмет в грид жилета
    FVector2D ScreenPosition = InDragDropEvent.GetScreenSpacePosition();
    FVector2D LocalPosition = InGeometry.AbsoluteToLocal(ScreenPosition);
    FVector2D VestGridPosition = FVector2D(70.f, 300.f);
    FVector2D VestGridSizeLocal = FVector2D(360.f, 120.f);
    
    bool bOverVestGrid = LocalPosition.X >= VestGridPosition.X && 
                        LocalPosition.X <= VestGridPosition.X + VestGridSizeLocal.X &&
                        LocalPosition.Y >= VestGridPosition.Y && 
                        LocalPosition.Y <= VestGridPosition.Y + VestGridSizeLocal.Y;
    
    if (bOverVestGrid)
    {
        UWidget* W = Cast<UWidget>(InOperation->Payload);
        if (W)
        {
            UInventoryItemWidget* ItemWidget = Cast<UInventoryItemWidget>(W);
            if (ItemWidget && ItemWidget->ItemData)
            {
                bool bSuccess = HandleVestGridDrop(ItemWidget->ItemData, InDragDropEvent.GetScreenSpacePosition());
                if (bSuccess)
                {
                    // Удаляем предмет из рюкзака, если он был там
                    APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(GetOwningPlayerPawn());
                    if (PlayerChar && PlayerChar->InventoryComponent)
                    {
                        PlayerChar->InventoryComponent->RemoveSpecificFromBackpack(ItemWidget->ItemData);
                    }
                    
                    // Обновляем UI
                    SyncBackpack(PlayerChar->InventoryComponent->BackpackItems);
                }
                return bSuccess;
            }
        }
        return false;
    }
    
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
    if (!EquipmentPanelRef) 
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("❌ UpdateEquipmentSlots: EquipmentPanelRef is null"));
        return;
    }
    
    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, TEXT("🔧 UpdateEquipmentSlots called"));
    
    // Очищаем старые виджеты слотов экипировки
    if (EquipmentPanelRef)
    {
        // Удаляем все дочерние элементы панели экипировки
        TArray<UWidget*> Children = EquipmentPanelRef->GetAllChildren();
        for (UWidget* Child : Children)
        {
            EquipmentPanelRef->RemoveChild(Child);
        }
    }
    EquipmentSlotWidgets.Empty();
    
    // Получаем данные экипировки от персонажа
    APlayerController* PC = GetOwningPlayer();
    if (!PC) return;
    
    APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(PC->GetPawn());
    if (!PlayerChar || !PlayerChar->InventoryComponent) return;
    
    UInventoryComponent* InvComp = PlayerChar->InventoryComponent;
    
    // Создаем виджеты для слотов экипировки
    TArray<TEnumAsByte<EEquipmentSlotType>> SlotTypes = {
        Helmet,
        Vest,
        Backpack,
        PrimaryWeapon,
        SecondaryWeapon,
        Melee
    };
    
    TArray<FString> SlotNames = {
        TEXT("Голова"),
        TEXT("Разгрузка"),
        TEXT("Рюкзак"),
        TEXT("Основное оружие"),
        TEXT("Вторичное оружие"),
        TEXT("Холодное оружие")
    };
    
    // Отображаем только слот "Разгрузка" для жилета
    for (int32 i = 0; i < SlotTypes.Num(); ++i)
    {
        EEquipmentSlotType SlotType = SlotTypes[i];
        
        // Показываем только слот Vest (Разгрузка)
        if (SlotType != Vest) continue;
        
        UEquippableItemData* EquippedItem = InvComp->GetEquippedItem(SlotType);
        
        // Создаем виджет слота экипировки
        CreateEquipmentSlotWidget(EquippedItem, i, SlotNames[i], SlotType);
        
        if (EquippedItem)
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, 
                    FString::Printf(TEXT("✅ Slot %d (%s): %s"), (int32)SlotType, *SlotNames[i], *EquippedItem->DisplayName.ToString()));
            }
        }
        else
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Silver, 
                    FString::Printf(TEXT("⚪ Slot %d (%s): Empty"), (int32)SlotType, *SlotNames[i]));
            }
        }
    }
}

void UInventoryWidget::CreateEquipmentSlotWidget(const UEquippableItemData* Item, int32 SlotIndex, const FString& SlotName, EEquipmentSlotType SlotType)
{
    if (!EquipmentPanelRef) return;
    
    // Создаем контейнер для слота
    UBorder* SlotContainer = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
    SlotContainer->SetBrushColor(Item ? FLinearColor(0.3f, 0.7f, 0.3f, 0.8f) : FLinearColor(0.2f, 0.2f, 0.2f, 0.8f));
    
    // Добавляем текст с названием слота
    UTextBlock* SlotLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    SlotLabel->SetText(FText::FromString(SlotName));
    SlotLabel->SetColorAndOpacity(FLinearColor::White);
    SlotLabel->SetJustification(ETextJustify::Center);
    SlotContainer->AddChild(SlotLabel);
    
    // Если есть предмет в слоте, добавляем его иконку и создаем интерактивный виджет
    if (Item)
    {
        // Создаем виджет предмета для слота экипировки
        UInventoryItemWidget* ItemWidget = WidgetTree->ConstructWidget<UInventoryItemWidget>(UInventoryItemWidget::StaticClass());
        UTexture2D* Icon = Item->Icon ? Item->Icon : LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture"));
        ItemWidget->Init(const_cast<UEquippableItemData*>(Item), Icon, EquipmentSlotSize);
        
        // Добавляем виджет предмета в контейнер
        SlotContainer->AddChild(ItemWidget);
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, 
                FString::Printf(TEXT("✅ Created equipment slot widget for: %s"), *Item->DisplayName.ToString()));
        }
    }
    else
    {
        // Создаем пустой слот с текстом "Пусто"
        UTextBlock* EmptyLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
        EmptyLabel->SetText(FText::FromString(TEXT("Пусто")));
        EmptyLabel->SetColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f));
        EmptyLabel->SetJustification(ETextJustify::Center);
        SlotContainer->AddChild(EmptyLabel);
    }
    
    // Добавляем слот в панель
    if (UCanvasPanelSlot* CanvasSlot = EquipmentPanelRef->AddChildToCanvas(SlotContainer))
    {
        CanvasSlot->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
        CanvasSlot->SetAlignment(FVector2D(0.f, 0.f));
        // Размещаем слоты слева от основного интерфейса
        CanvasSlot->SetPosition(FVector2D(20.f, 10.f + SlotIndex * (EquipmentSlotSize.Y + 10.f)));
        CanvasSlot->SetSize(EquipmentSlotSize);
        CanvasSlot->SetZOrder(10);
    }
    
    EquipmentSlotWidgets.Add(SlotContainer);
}

void UInventoryWidget::UpdateStaticEquipmentSlots()
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, TEXT("🔍 UpdateStaticEquipmentSlots called"));
    }
    
    // Обновляем слот "разгрузка"
    if (VestSlotRef)
    {
        // Получаем данные экипировки от персонажа
        APlayerController* PC = GetOwningPlayer();
        if (PC)
        {
            APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(PC->GetPawn());
            if (PlayerChar && PlayerChar->InventoryComponent)
            {
                UInventoryComponent* InvComp = PlayerChar->InventoryComponent;
                UEquippableItemData* EquippedVest = InvComp->GetEquippedItem(Vest);
                
                if (EquippedVest)
                {
                    // Если виджет уже существует, обновляем его данные
                    if (VestItemWidgetRef && VestItemWidgetRef->IsValidLowLevel())
                    {
                        // Обновляем данные существующего виджета
                        UTexture2D* Icon = EquippedVest->Icon ? EquippedVest->Icon : LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture"));
                        VestItemWidgetRef->Init(const_cast<UEquippableItemData*>(EquippedVest), Icon, FVector2D(80.f, 80.f));
                        
                        // Убеждаемся, что флаг статического слота установлен
                        VestItemWidgetRef->SetStaticEquipmentSlot(true);
                        
                        if (GEngine)
                        {
                            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, 
                                FString::Printf(TEXT("✅ Vest slot updated existing widget: %s"), *EquippedVest->DisplayName.ToString()));
                        }
                    }
                    else
                    {
                        // Создаем новый виджет предмета для слота
                        VestItemWidgetRef = WidgetTree->ConstructWidget<UInventoryItemWidget>(UInventoryItemWidget::StaticClass());
                        UTexture2D* Icon = EquippedVest->Icon ? EquippedVest->Icon : LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture"));
                        VestItemWidgetRef->Init(const_cast<UEquippableItemData*>(EquippedVest), Icon, FVector2D(80.f, 80.f));
                        
                        // Устанавливаем флаг статического слота экипировки
                        VestItemWidgetRef->SetStaticEquipmentSlot(true);
                        
                        // Добавляем виджет в слот с правильным позиционированием
                        VestSlotRef->AddChild(VestItemWidgetRef);
                        
                        if (GEngine)
                        {
                            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, 
                                FString::Printf(TEXT("✅ Vest slot created new widget: %s"), *EquippedVest->DisplayName.ToString()));
                        }
                    }
                }
                else
                {
                    // Удаляем виджет, если жилет не экипирован
                    if (VestItemWidgetRef && VestItemWidgetRef->IsValidLowLevel())
                    {
                        VestSlotRef->RemoveChild(VestItemWidgetRef);
                        VestItemWidgetRef = nullptr;
                        
                        if (GEngine)
                        {
                            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Silver, TEXT("⚪ Vest slot cleared"));
                        }
                    }
                }
            }
        }
    }
}

bool UInventoryWidget::IsPositionValidInBackpack(int32 X, int32 Y, int32 SizeX, int32 SizeY) const
{
    const int32 MaxCols = FMath::Max(1, (int32)(BackpackGridSize.X / BackpackCellSize.X));
    const int32 MaxRows = FMath::Max(1, (int32)(BackpackGridSize.Y / BackpackCellSize.Y));
    
    // Проверяем, что предмет помещается в границы грида
    if (X < 0 || Y < 0 || X + SizeX > MaxCols || Y + SizeY > MaxRows)
    {
        return false;
    }
    
    return true;
}

FVector2D UInventoryWidget::FindValidPositionInBackpack(UInventoryItemData* ItemData) const
{
    if (!ItemData) return BackpackGridPosition;
    
    const int32 SizeX = FMath::Max(1, ItemData->SizeInCellsX);
    const int32 SizeY = FMath::Max(1, ItemData->SizeInCellsY);
    const int32 MaxCols = OccupiedCells.Num() > 0 ? OccupiedCells[0].Num() : 0;
    const int32 MaxRows = OccupiedCells.Num();
    
    if (MaxCols == 0 || MaxRows == 0)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
                TEXT("❌ OccupiedCells not initialized"));
        }
        return BackpackGridPosition;
    }
    
    // Ищем первую подходящую позицию
    for (int32 Y = 0; Y <= MaxRows - SizeY; ++Y)
    {
        for (int32 X = 0; X <= MaxCols - SizeX; ++X)
        {
            if (AreCellsFree(X, Y, SizeX, SizeY))
            {
                FVector2D Position = BackpackGridPosition + FVector2D(
                    BackpackCellSize.X * X, 
                    BackpackCellSize.Y * Y
                );
                
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                        FString::Printf(TEXT("✅ Found valid position for %s: X=%d, Y=%d"), 
                            *ItemData->DisplayName.ToString(), X, Y));
                }
                
                return Position;
            }
        }
    }
    
    // Если не найдена подходящая позиция, возвращаем позицию за пределами грида
    FVector2D InvalidPosition = BackpackGridPosition + FVector2D(
        BackpackGridSize.X + 10.f, 
        BackpackGridSize.Y + 10.f
    );
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
            FString::Printf(TEXT("❌ No valid position found for %s, placing outside grid"), 
                *ItemData->DisplayName.ToString()));
    }
    
    return InvalidPosition;
}

void UInventoryWidget::InitializeOccupiedCells()
{
    const int32 MaxCols = FMath::Max(1, (int32)(BackpackGridSize.X / BackpackCellSize.X));
    const int32 MaxRows = FMath::Max(1, (int32)(BackpackGridSize.Y / BackpackCellSize.Y));
    
    OccupiedCells.Empty();
    OccupiedCells.SetNum(MaxRows);
    
    for (int32 Y = 0; Y < MaxRows; ++Y)
    {
        OccupiedCells[Y].SetNum(MaxCols);
        for (int32 X = 0; X < MaxCols; ++X)
        {
            OccupiedCells[Y][X] = false;
        }
    }
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, 
            FString::Printf(TEXT("🔧 Initialized occupied cells: %dx%d"), MaxCols, MaxRows));
    }
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

void UInventoryWidget::ClearItemPosition(UInventoryItemData* ItemData)
{
    if (!ItemData) return;
    
    // Удаляем из системы drag & drop
    for (int32 i = Placed.Num() - 1; i >= 0; --i)
    {
        if (Placed[i].Widget && Placed[i].Widget->ItemData == ItemData)
        {
            Placed.RemoveAt(i);
            
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, 
                    FString::Printf(TEXT("🧹 Cleared drag&drop position for %s"), *ItemData->DisplayName.ToString()));
            }
            break;
        }
    }
    
    // Удаляем из системы занятых ячеек (если есть)
    // Это будет сделано автоматически при следующем SyncBackpack
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
    
    // Используем ту же логику, что и для других гридов в инвентаре
    UCanvasPanel* RightPanel = Cast<UCanvasPanel>(WidgetTree->RootWidget);
    if (!RightPanel)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("❌ RightPanel not found"));
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
        UCanvasPanel* Grid = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("VestGrid"));
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
                RegisterGrid(Label, Pos, GridSize, GridWidth, GridHeight);
            }
            
            // Добавляем фон грида (видимый)
            UBorder* GridBackground = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
            if (GridBackground)
            {
                GridBackground->SetBrushColor(FLinearColor(0.2f, 0.8f, 0.2f, 0.3f)); // Зеленый фон для видимости
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
                    UCanvasPanel* CellContainer = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass());
                    if (CellContainer)
                    {
                        // Добавляем фон ячейки
                        UBorder* CellBackground = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
                        if (CellBackground)
                        {
                            CellBackground->SetBrushColor(FLinearColor(0.0f, 0.0f, 1.0f, 0.2f)); // Синий фон
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
                    FString::Printf(TEXT("✅ Vest grid created: Pos(%.0f,%.0f), Size(%.0fx%.0f)"), 
                        Pos.X, Pos.Y, GridSize.X, GridSize.Y));
            }
        }
    };
    
    // Создаем 6 отдельных гридов жилета горизонтально в одну линию
    // Все гриды на одной высоте Y=200.f, но с разными X позициями
    AddVestLabeledGrid(TEXT("жилет1"), 70.f, 200.f, FVector2D(60.f, 60.f), 1, 1, nullptr, false);     // 1x1 (левый)
    AddVestLabeledGrid(TEXT("жилет2"), 140.f, 200.f, FVector2D(60.f, 60.f), 1, 1, nullptr, false);     // 1x1 (левый-центр)
    AddVestLabeledGrid(TEXT("жилет3"), 210.f, 200.f, FVector2D(60.f, 120.f), 1, 2, nullptr, false);    // 1x2 (центр-левый)
    AddVestLabeledGrid(TEXT("жилет4"), 280.f, 200.f, FVector2D(60.f, 120.f), 1, 2, nullptr, false);    // 1x2 (центр-правый)
    AddVestLabeledGrid(TEXT("жилет5"), 350.f, 200.f, FVector2D(60.f, 60.f), 1, 1, nullptr, false);     // 1x1 (правый-центр)
    AddVestLabeledGrid(TEXT("жилет6"), 420.f, 200.f, FVector2D(60.f, 60.f), 1, 1, nullptr, false);     // 1x1 (правый)
    
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
    for (UCanvasPanel* Grid : VestGrids)
    {
        if (Grid)
        {
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
            
            Grid->RemoveFromParent();
        }
    }
    
    VestGrids.Empty();
    VestGridSizes.Empty();
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("🗑️ Уничтожены все гриды жилета"));
    }
}

void UInventoryWidget::UpdateVestGrid()
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, TEXT("🔍 UpdateVestGrid called"));
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
            FString::Printf(TEXT("🔍 EquippedVest: %s, HasStorage: %s"), 
                EquippedVest ? TEXT("Found") : TEXT("Null"),
                EquippedVest && EquippedVest->bHasAdditionalStorage ? TEXT("Yes") : TEXT("No")));
    }
    
    // Управляем гридом жилета
    if (EquippedVest && EquippedVest->bHasAdditionalStorage)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("✅ Жилет найден с дополнительным хранилищем"));
        }
        
        // Всегда пересоздаем грид для обновления позиции
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("🔧 Пересоздаем грид жилета..."));
        }
        CreateVestGrid();
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
    
    // Очищаем занятые ячейки для всех 6 гридов (горизонтальная конфигурация)
    for (int32 GridIndex = 0; GridIndex < 6; ++GridIndex)
    {
        if (GridIndex < 2 || GridIndex >= 4) // Гриды 1-2 и 5-6: 1x1
        {
            VestOccupiedCells[GridIndex][0][0] = false;
        }
        else // Гриды 3-4: 1x2
        {
            VestOccupiedCells[GridIndex][0][0] = false;
            VestOccupiedCells[GridIndex][1][0] = false;
        }
    }
    
    // Размещаем предметы в гридах (пока отключено для 6 гридов)
    // for (int32 i = 0; i < VestItems.Num(); ++i)
    // {
    //     AddVestGridItemIcon(VestItems[i], i);
    // }
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
