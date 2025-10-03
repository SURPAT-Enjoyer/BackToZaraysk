#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryItemData.h"
#include "InventoryItemWidget.generated.h"

UCLASS()
class BACKTOZARAYSK_API UInventoryItemWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    UPROPERTY()
    TObjectPtr<class UImage> Image = nullptr;

    UPROPERTY()
    TObjectPtr<class UInventoryItemData> ItemData = nullptr;

    UPROPERTY()
    int32 CellX = 0;

    UPROPERTY()
    int32 CellY = 0;

    UPROPERTY()
    int32 SizeX = 1;

    UPROPERTY()
    int32 SizeY = 1;

    UPROPERTY()
    bool bRotated = false;

    void Init(UInventoryItemData* InData, UTexture2D* InIcon, const FVector2D& CellSize);
    void UpdateVisualSize(const FVector2D& CellSize);
    void SetTint(const FLinearColor& Color);

protected:
    virtual void NativeConstruct() override;
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
    virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

    UFUNCTION()
    void OnDropClicked();

    UFUNCTION()
    void OnEquipClicked();
};


