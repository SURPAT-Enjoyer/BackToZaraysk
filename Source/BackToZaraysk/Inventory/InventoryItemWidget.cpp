#include "InventoryItemWidget.h"
#include "Components/Image.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/DragDropOperation.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "InventoryWidget.h"
#include "InventoryComponent.h"
#include "BackToZaraysk/Characters/BTZBaseCharacter.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "BackToZaraysk/GameData/Items/Test/PickupCube.h"
#include "BackToZaraysk/GameData/Items/Test/PickupParallelepiped.h"

void UInventoryItemWidget::Init(UInventoryItemData* InData, UTexture2D* InIcon, const FVector2D& CellSize)
{
    ItemData = InData;
    SizeX = InData ? FMath::Max(1, InData->SizeInCellsX) : 1;
    SizeY = InData ? FMath::Max(1, InData->SizeInCellsY) : 1;
    if (!Image && WidgetTree)
    {
        Image = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("Icon"));
        this->SetClipping(EWidgetClipping::ClipToBounds);
        this->WidgetTree->RootWidget = Image;
    }
    if (Image)
    {
        UTexture2D* Icon = InIcon ? InIcon : ((InData && InData->Icon) ? InData->Icon : LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture")));
        FSlateBrush Brush; Brush.SetResourceObject(Icon); Brush.ImageSize = FVector2D(CellSize.X * SizeX, CellSize.Y * SizeY);
        Image->SetBrush(Brush);
    }
}

void UInventoryItemWidget::NativeConstruct()
{
    Super::NativeConstruct();
    bIsFocusable = true; // позволяем ловить R на виджете предмета
}

FReply UInventoryItemWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    const FKey Button = InMouseEvent.GetEffectingButton();
    if (Button == EKeys::RightMouseButton)
    {
        if (UInventoryWidget* Parent = GetTypedOuter<UInventoryWidget>())
        {
            if (UCanvasPanel* Root = Cast<UCanvasPanel>(Parent->WidgetTree->RootWidget))
            {
                // Удаляем предыдущее меню
                TArray<UWidget*> Children = Root->GetAllChildren();
                for (UWidget* W : Children)
                {
                    if (W && W->GetFName() == TEXT("ContextMenu"))
                    {
                        Root->RemoveChild(W);
                        break;
                    }
                }
                UBorder* Menu = Parent->WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("ContextMenu"));
                Menu->SetBrushColor(FLinearColor(0.f,0.f,0.f,0.9f));
                UButton* DropBtn = Parent->WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
                UTextBlock* Txt = Parent->WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
                Txt->SetText(FText::FromString(TEXT("выбросить")));
                DropBtn->AddChild(Txt);
                Menu->AddChild(DropBtn);
                if (UCanvasPanelSlot* S = Root->AddChildToCanvas(Menu))
                {
                    const FVector2D ScreenPos = InMouseEvent.GetScreenSpacePosition();
                    const FVector2D Local = Root->GetCachedGeometry().AbsoluteToLocal(ScreenPos);
                    S->SetAnchors(FAnchors(0.f,0.f,0.f,0.f));
                    S->SetAlignment(FVector2D(0.f,0.f));
                    S->SetPosition(Local + FVector2D(6.f,6.f));
                    S->SetSize(FVector2D(140.f, 40.f));
                    S->SetZOrder(9999);
                }
                DropBtn->OnClicked.AddDynamic(this, &UInventoryItemWidget::OnDropClicked);
            }
        }
        return FReply::Handled();
    }
    if (Button == EKeys::LeftMouseButton)
    {
        return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
    }
    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UInventoryItemWidget::OnDropClicked()
{
    if (UInventoryWidget* Parent = GetTypedOuter<UInventoryWidget>())
    {
        if (!ItemData) return;
        if (APlayerController* PC = Parent->GetOwningPlayer())
        {
                        if (ABTZBaseCharacter* Ch = Cast<ABTZBaseCharacter>(PC->GetPawn()))
            {
                if (UInventoryComponent* Inv = Ch->FindComponentByClass<UInventoryComponent>())
                {
                    if (Inv->RemoveSpecificFromBackpack(ItemData))
                    {
                        // Спавним предмет перед игроком, как при обычном Drop
                        if (APlayerController* PC2 = Parent->GetOwningPlayer())
                        {
                            FVector ViewLoc; FRotator ViewRot; PC2->GetPlayerViewPoint(ViewLoc, ViewRot);
                            const FVector SpawnLoc = ViewLoc + ViewRot.Vector() * 80.f;
                            FActorSpawnParameters S; S.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
                            TSubclassOf<AActor> DropClass = APickupCube::StaticClass();
                            if (ItemData->SizeInCellsX == 2 && ItemData->SizeInCellsY == 1)
                            {
                                DropClass = APickupParallelepiped::StaticClass();
                            }
                            if (UWorld* World = GetWorld())
                            {
                                World->SpawnActor<AActor>(DropClass, SpawnLoc, ViewRot, S);
                            }
                        }
                                    Parent->RemoveItemWidget(this);
                                    Parent->RemoveItemMapping(ItemData);
                    }
                }
            }
        }
        // Закрыть меню
        if (UCanvasPanel* RootLocal = Cast<UCanvasPanel>(Parent->WidgetTree->RootWidget))
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
    }
}

FReply UInventoryItemWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    if (InKeyEvent.GetKey() == EKeys::R && ItemData && ItemData->bRotatable)
    {
        // Передаем обработку вращения родительскому виджету инвентаря
        if (UInventoryWidget* ParentInv = GetTypedOuter<UInventoryWidget>())
        {
            // Устанавливаем себя как hovered item для корректной обработки
            ParentInv->SetHoveredItem(this);
            return ParentInv->HandleItemRotation(InGeometry, InKeyEvent);
        }
        
        // Fallback: локальная обработка если родитель не найден
        bRotated = !bRotated;
        UpdateVisualSize(FVector2D(60.f, 60.f));
        return FReply::Handled();
    }
    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UInventoryItemWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
    UDragDropOperation* Op = UWidgetBlueprintLibrary::CreateDragDropOperation(UDragDropOperation::StaticClass());
    Op->DefaultDragVisual = this;
    Op->Payload = this;
    Op->Pivot = EDragPivot::MouseDown;
    OutOperation = Op;
    SetTint(FLinearColor(1.f, 1.f, 0.f, 1.f));
}

void UInventoryItemWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
    if (UInventoryWidget* ParentInv = GetTypedOuter<UInventoryWidget>())
    {
        ParentInv->SetHoveredItem(this);
    }
}

void UInventoryItemWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseLeave(InMouseEvent);
    if (UInventoryWidget* ParentInv = GetTypedOuter<UInventoryWidget>())
    {
        ParentInv->ClearHoveredItem(this);
    }
}

void UInventoryItemWidget::UpdateVisualSize(const FVector2D& CellSize)
{
    if (!Image) return;
    const int32 VX = bRotated ? SizeY : SizeX;
    const int32 VY = bRotated ? SizeX : SizeY;
    FSlateBrush Brush = Image->GetBrush();
    Brush.ImageSize = FVector2D(CellSize.X * VX, CellSize.Y * VY);
    Image->SetBrush(Brush);
}

void UInventoryItemWidget::SetTint(const FLinearColor& Color)
{
    if (Image)
    {
        Image->SetColorAndOpacity(Color);
    }
}


