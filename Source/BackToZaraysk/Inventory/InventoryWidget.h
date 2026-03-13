#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BackToZaraysk/Inventory/EquipmentSlotType.h"
#include "InventoryWidget.generated.h"

class UInventoryItemWidget;
class UInventoryItemData;
class UEquipModBaseItemData;
class AArmorModPreviewActor;
class UTextureRenderTarget2D;

UCLASS()
class BACKTOZARAYSK_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetVisible(bool bIsVisible);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsInventoryVisible() const { return bShown; }

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;
    virtual void NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) override { Super::NativeOnAddedToFocusPath(InFocusEvent); }
    virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
    virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
    FReply HandleItemRotation(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent);
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual FReply NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override { return FReply::Unhandled(); }

public:
    virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
    
    // Функции для слотов экипировки
    void UpdateEquipmentSlots();
    void CreateEquipmentSlotWidget(const class UEquippableItemData* Item, int32 SlotIndex, const FString& SlotName, EEquipmentSlotType SlotType);
    void UpdateStaticEquipmentSlots(); // Обновляем статические слоты в левой панели
    
    
    
    // Функции для грида жилета
    void CreateVestGrid();
    void DestroyVestGrid();
    void UpdateVestGrid();

    // Гриды хранилищ модов брони (в зоне жилета: при броне с модами показываются здесь)
    void CreateArmorModGrids();
    void DestroyArmorModGrids();
    void UpdateArmorModGrids();
    void AddVestGridItemIcon(class UInventoryItemData* ItemData, int32 Index);
    void ForceClearVestGrids(); // Принудительная очистка всех гридов жилета
    
    // Drag&Drop для грида жилета
    bool CanDropOnVestGrid(const FGeometry& Geometry, const FVector2D& ScreenPosition) const;
    bool HandleVestGridDrop(class UInventoryItemData* ItemData, const FVector2D& Position);
    bool HandleVestGridItemDrag(class UInventoryItemData* ItemData);
    FVector2D GetVestGridCellPosition(int32 CellX, int32 CellY) const;

    // Окно модификации брони: вызов из InventoryItemWidget при ЛКМ по моду в режиме «установить в ячейку»
    bool TryInstallModOnArmorFromWaiting(class UInventoryItemData* ModItem);
    bool IsArmorModInstallWaitingForClick() const { return bArmorModInstallWaitingClick; }
    class UEquippableItemData* GetOpenedArmorForMods() const { return OpenedArmorForMods; }

private:
	bool bShown = false;
    bool bUIBuilt = false;

protected:
	UPROPERTY()
	class UInventoryGridWidget* GridWidget = nullptr;

    // Правый контейнер и параметры грида рюкзака для размещения иконок
    UPROPERTY(BlueprintReadOnly)
    class UCanvasPanel* RightPanelRef = nullptr;
    
    // Система отслеживания занятых ячеек
    TArray<TArray<bool>> OccupiedCells; // Двумерный массив занятых ячеек
    void InitializeOccupiedCells();
    void MarkCellsAsOccupied(int32 X, int32 Y, int32 SizeX, int32 SizeY);
    void MarkCellsAsFree(int32 X, int32 Y, int32 SizeX, int32 SizeY);
    bool AreCellsFree(int32 X, int32 Y, int32 SizeX, int32 SizeY) const;
    
    // Описание областей гридов справа
    struct FGridArea
    {
        FVector2D Position = FVector2D::ZeroVector; // в локалях RightPanel
        FVector2D Size = FVector2D::ZeroVector;
        int32 CellsX = 0;
        int32 CellsY = 0;
        FString Name;
    };
    TArray<FGridArea> GridAreas;
    void RegisterGrid(const FString& Name, const FVector2D& Pos, const FVector2D& Size, int32 CellsX, int32 CellsY);
    int32 FindGridAtPoint(const FVector2D& LocalPoint) const;
    FVector2D SnapToCellInGrid(int32 GridIndex, const FVector2D& LocalPoint) const;
    bool TryAutoPlaceInGrid(int32 GridIndex, int32 SizeX, int32 SizeY, int32& OutCellX, int32& OutCellY) const;
    
    // Размещение предметов и проверка занятости
    struct FPlacedItem
    {
        class UInventoryItemWidget* Widget = nullptr;
        int32 GridIndex = INDEX_NONE;
        int32 CellX = 0;
        int32 CellY = 0;
        int32 SizeX = 1;
        int32 SizeY = 1;
        bool bRotated = false;
    };
    TArray<FPlacedItem> Placed;
    bool IsAreaFree(int32 GridIndex, int32 CellX, int32 CellY, int32 SizeX, int32 SizeY, class UInventoryItemWidget* Ignore) const;
    void UpsertPlacement(class UInventoryItemWidget* Widget, int32 GridIndex, int32 CellX, int32 CellY);
    bool IsAreaFreeInEquipment(const class UEquippableItemData* EquipmentData,
                               int32 StartCellX,
                               int32 StartCellY,
                               int32 SizeX,
                               int32 SizeY,
                               const class UInventoryItemData* IgnoredItem) const;

    bool FindNearestFreeCellInEquipment(const class UEquippableItemData* EquipmentData,
                                        int32 SizeX,
                                        int32 SizeY,
                                        int32 StartX,
                                        int32 StartY,
                                        int32& OutX,
                                        int32& OutY,
                                        const class UInventoryItemData* IgnoredItem) const;
    
    // Текущий dnd
    UPROPERTY()
    class UBorder* Highlight = nullptr;
    UPROPERTY()
    class UInventoryItemWidget* DragItem = nullptr;
    bool bDragActive = false;
    FVector2D DragStartPos = FVector2D::ZeroVector;
    FVector2D DragStartSize = FVector2D::ZeroVector;
    int32 DragStartGridIndex = INDEX_NONE;
    int32 DragStartCellX = 0;
    int32 DragStartCellY = 0;
    
    // Связь данных предмета с его виджетом для сохранения положения между открытиями
    UPROPERTY(BlueprintReadOnly)
    TMap<TObjectPtr<class UInventoryItemData>, TObjectPtr<class UInventoryItemWidget>> ItemToWidget;
    
public:
    // Функции для работы с рюкзаком (legacy - kept for compatibility)
    void RemoveItemWidget(class UInventoryItemWidget* Widget);
    void RemoveItemMapping(class UInventoryItemData* ItemData);
    void ClearItemPosition(class UInventoryItemData* ItemData);
    void SyncBackpack(const TArray<class UInventoryItemData*>& Items);
    void AddBackpackItemIcon(class UInventoryItemData* ItemData);

    

    // Слоты экипировки
    UPROPERTY(BlueprintReadOnly)
    class UCanvasPanel* EquipmentPanelRef = nullptr;
    FVector2D EquipmentSlotSize = FVector2D(80.f, 80.f);
    UPROPERTY()
    TArray<TObjectPtr<class UWidget>> EquipmentSlotWidgets;
    
    // Ссылки на статические слоты экипировки в левой панели
    UPROPERTY()
    class UBorder* VestSlotRef = nullptr; // Ссылка на слот "разгрузка"
    UPROPERTY()
    class UInventoryItemWidget* VestItemWidgetRef = nullptr; // Ссылка на виджет предмета в слоте разгрузки
    
    // Слот бронежилета
    UPROPERTY()
    class UBorder* ArmorSlotRef = nullptr; // Ссылка на слот "бронежилет"
    UPROPERTY()
    class UInventoryItemWidget* ArmorItemWidgetRef = nullptr; // Ссылка на виджет предмета в слоте бронежилета

    // Слот головы (шлем)
    UPROPERTY()
    class UBorder* HelmetSlotRef = nullptr; // Ссылка на слот "голова"
    UPROPERTY()
    class UInventoryItemWidget* HelmetItemWidgetRef = nullptr; // Ссылка на виджет предмета в слоте головы
    
    // Гриды жилета (6 отдельных гридов)
    UPROPERTY()
    TArray<class UCanvasPanel*> VestGrids;
    
    // Размеры гридов жилета
    UPROPERTY()
    TArray<FVector2D> VestGridSizes;
    
    // Занятые ячейки гридов жилета (6 гридов)
    TArray<TArray<TArray<bool>>> VestOccupiedCells;

    // Гриды хранилищ модов брони (одна панель на каждый мини‑грид каждого мода с bHasAdditionalStorage)
    UPROPERTY()
    TArray<class UCanvasPanel*> ArmorModGrids;
    UPROPERTY()
    TArray<FVector2D> ArmorModGridSizes;
    // Для каждого элемента ArmorModGrids храним, к какому модулю он относится и индекс мини‑грида внутри этого модуля.
    TArray<class UEquipModBaseItemData*> ArmorModGridModData;
    TArray<int32> ArmorModGridMiniIndices;
    
    // Ссылки на слот рюкзака
    UPROPERTY()
    class UBorder* BackpackSlotRef = nullptr; // Ссылка на слот "рюкзак"
    UPROPERTY()
    class UInventoryItemWidget* BackpackItemWidgetRef = nullptr; // Ссылка на виджет предмета в слоте рюкзака

    // Мини‑гриды хранилища рюкзака (в новом формате)
    UPROPERTY()
    TArray<class UCanvasPanel*> BackpackGrids;

    UPROPERTY()
    TArray<FVector2D> BackpackGridSizes;

    // Кеш лэйаута, чтобы не пересоздавать гриды каждый апдейт (иначе начинает лагать)
    uint32 CachedVestLayoutHash = 0;
    uint32 CachedBackpackLayoutHash = 0;
    
    

public:

public:




    // Hover
    UPROPERTY()
    class UInventoryItemWidget* HoverItem = nullptr;


public:
    // Полный рефреш UI
    void RefreshInventoryUI();

    // Hover helpers from item widgets
    void SetHoveredItem(class UInventoryItemWidget* Item) { HoverItem = Item; }
    void ClearHoveredItem(class UInventoryItemWidget* Item) { if (HoverItem == Item) HoverItem = nullptr; }

    // Обновление отображения грида карманов
    void UpdatePocketsGrid();
    
    // Обновление отображения грида хранилища рюкзака
    void UpdateBackpackStorageGrid();

    // Проверка свободной области для размещения предмета в гриде рюкзака
    bool IsAreaFreeInBackpack(const class UEquippableItemData* BackpackData,
                              int32 StartCellX,
                              int32 StartCellY,
                              int32 SizeX,
                              int32 SizeY,
                              const class UInventoryItemData* IgnoredItem) const;

    // Удаляет ранее созданный виджет данного ItemData из любых контейнеров UI
    void RemoveExistingItemWidget(class UInventoryItemData* ItemData);

    // === Окно хранилища модуля (EquipModBase) ===
    void OpenEquipModStorage(UEquipModBaseItemData* ModItem);
    void CloseEquipModStorage();
    void RefreshOpenedEquipModStorage();

    // === Окно модификации бронежилета (ArmorBase и его наследников) ===
    void OpenArmorModWindow(class UEquippableItemData* ArmorItem);
    UFUNCTION()
    void CloseArmorModWindow();

private:
    // Текущий открытый модуль
    UPROPERTY()
    TObjectPtr<UEquipModBaseItemData> OpenedEquipMod = nullptr;

    // Виджеты модального окна
    UPROPERTY()
    TObjectPtr<class UImage> EquipModBackdrop = nullptr;
    UPROPERTY()
    TObjectPtr<class UBorder> EquipModWindowBorder = nullptr;
    UPROPERTY()
    TObjectPtr<class UCanvasPanel> EquipModWindowCanvas = nullptr;

    // Мини‑гриды окна модуля
    UPROPERTY()
    TArray<class UCanvasPanel*> EquipModGrids;
    UPROPERTY()
    TArray<FVector2D> EquipModGridSizes;
    uint32 CachedEquipModLayoutHash = 0;

    void UpdateEquipModStorageGrid();

    UFUNCTION()
    void HandleEquipModCloseClicked();

    // Drag window
    bool bDraggingEquipModWindow = false;
    FVector2D EquipModDragStartMouseScreen = FVector2D::ZeroVector;
    FVector2D EquipModDragStartWindowPos = FVector2D::ZeroVector;

    // Состояние окна модификации бронежилета
    UPROPERTY()
    TObjectPtr<class UEquippableItemData> OpenedArmorForMods = nullptr;

    UPROPERTY()
    TObjectPtr<class UBorder> ArmorModWindowBorder = nullptr;
    UPROPERTY()
    TObjectPtr<class UCanvasPanel> ArmorModWindowCanvas = nullptr;
    UPROPERTY()
    TObjectPtr<class UImage> ArmorModImage = nullptr;

    /** Актор превью 3D-модели в окне модификации. */
    UPROPERTY()
    TObjectPtr<class AArmorModPreviewActor> ArmorModPreviewActor = nullptr;
    /** Render target для отображения 3D-превью в UImage. */
    UPROPERTY()
    TObjectPtr<class UTextureRenderTarget2D> ArmorModRenderTarget = nullptr;

    bool bDraggingArmorModWindow = false;
    FVector2D ArmorModDragStartMouseScreen = FVector2D::ZeroVector;
    FVector2D ArmorModDragStartWindowPos = FVector2D::ZeroVector;

    /** Масштаб изображения превью (колесо мыши). */
    float ArmorModPreviewScale = 1.f;

    /** Ресайз окна модификации за рамку. */
    bool bResizingArmorModWindow = false;
    FVector2D ArmorModResizeStartMouseScreen = FVector2D::ZeroVector;
    FVector2D ArmorModResizeStartSize = FVector2D::ZeroVector;
    FVector2D ArmorModResizeStartPosition = FVector2D::ZeroVector;
    bool bArmorModResizeRight = false;
    bool bArmorModResizeBottom = false;

    // Вращение превью (пока 2D‑вращение картинки как заглушка)
    bool bRotatingArmorPreview = false;
    FVector2D ArmorPreviewLastMouse = FVector2D::ZeroVector;

    /** Ширина зоны ресайза по краю окна (пиксели). */
    static constexpr float ArmorModResizeEdgePx = 8.f;
    static constexpr float ArmorModMinWidth = 320.f;
    static constexpr float ArmorModMinHeight = 240.f;

    /** Режим «выберите мод для установки» после ПКМ → Установить: следующий ЛКМ по моду в инвентаре установит его в сохранённую ячейку. */
    bool bArmorModInstallWaitingClick = false;
    int32 ArmorModInstallSide = 0;
    int32 ArmorModInstallCellX = 0;
    int32 ArmorModInstallCellY = 0;
    /** Индекс мода в InstalledMods для контекстного меню «Снять». */
    int32 ArmorModContextMenuModIndex = -1;

    bool AnyModCanInstallAtArmorCell(int32 Side, int32 CellX, int32 CellY) const;
    void TryInstallModOnArmor(UInventoryItemData* ModItem, int32 Side, int32 CellX, int32 CellY);
    void ClearArmorModHighlight();
    UFUNCTION()
    void OnArmorModContextInstallClicked();
    UFUNCTION()
    void OnArmorModContextUninstallClicked();
};
