#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BackToZaraysk/Inventory/EquipmentSlotType.h"
#include "InventoryWidget.generated.h"

class UInventoryItemWidget;
class UInventoryItemData;

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
    virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
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
    void AddVestGridItemIcon(class UInventoryItemData* ItemData, int32 Index);
    void ForceClearVestGrids(); // Принудительная очистка всех гридов жилета
    
    // Drag&Drop для грида жилета
    bool CanDropOnVestGrid(const FGeometry& Geometry, const FVector2D& ScreenPosition) const;
    bool HandleVestGridDrop(class UInventoryItemData* ItemData, const FVector2D& Position);
    bool HandleVestGridItemDrag(class UInventoryItemData* ItemData);
    FVector2D GetVestGridCellPosition(int32 CellX, int32 CellY) const;

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
    
    // Гриды жилета (6 отдельных гридов)
    UPROPERTY()
    TArray<class UCanvasPanel*> VestGrids;
    
    // Размеры гридов жилета
    UPROPERTY()
    TArray<FVector2D> VestGridSizes;
    
    // Занятые ячейки гридов жилета (6 гридов)
    TArray<TArray<TArray<bool>>> VestOccupiedCells;
    
    // Ссылки на слот рюкзака
    UPROPERTY()
    class UBorder* BackpackSlotRef = nullptr; // Ссылка на слот "рюкзак"
    UPROPERTY()
    class UInventoryItemWidget* BackpackItemWidgetRef = nullptr; // Ссылка на виджет предмета в слоте рюкзака
    UPROPERTY()
    class UInventoryGridWidget* BackpackStorageGrid = nullptr; // Грид хранилища рюкзака
    
    

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

    // Обновление отображения грида хранилища рюкзака
    void UpdateBackpackStorageGrid();
};
