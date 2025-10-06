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
#include "BackToZaraysk/Inventory/EquippableItemData.h"
#include "BackToZaraysk/Components/EquipmentComponent.h"
#include "BackToZaraysk/Characters/PlayerCharacter.h"
#include "BackToZaraysk/GameData/Items/TacticalVest.h"

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
                
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
                        FString::Printf(TEXT("🔍 Creating context menu for item: %s"), 
                            ItemData ? *ItemData->DisplayName.ToString() : TEXT("null")));
                }
                
                // Кнопки для экипируемых предметов
                UEquippableItemData* EquippableItem = Cast<UEquippableItemData>(ItemData);
                if (ItemData && EquippableItem)
                {
                    // Проверяем, экипирован ли предмет
                    if (EquippableItem->bIsEquipped)
                    {
                        // Кнопка "Снять" для экипированных предметов
                        UButton* UnequipBtn = Parent->WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
                        UTextBlock* UnequipTxt = Parent->WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
                        UnequipTxt->SetText(FText::FromString(TEXT("Снять")));
                        UnequipBtn->AddChild(UnequipTxt);
                        Menu->AddChild(UnequipBtn);
                        
                        // Привязываем функцию снятия экипировки
                        UnequipBtn->OnClicked.AddDynamic(this, &UInventoryItemWidget::OnUnequipClicked);
                    }
                    else
                    {
                        // Кнопка "Надеть" для неэкипированных предметов
                        UButton* EquipBtn = Parent->WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
                        UTextBlock* EquipTxt = Parent->WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
                        EquipTxt->SetText(FText::FromString(TEXT("Надеть")));
                        EquipBtn->AddChild(EquipTxt);
                        Menu->AddChild(EquipBtn);
                        
                        // Привязываем функцию экипировки
                        EquipBtn->OnClicked.AddDynamic(this, &UInventoryItemWidget::OnEquipClicked);
                    }
                }
                
                // Кнопка "Выбросить"
                UButton* DropBtn = Parent->WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
                UTextBlock* Txt = Parent->WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
                Txt->SetText(FText::FromString(TEXT("Выбросить")));
                Txt->SetColorAndOpacity(FLinearColor::White);
                DropBtn->AddChild(Txt);
                Menu->AddChild(DropBtn);
                
                // Привязываем функцию выброса
                DropBtn->OnClicked.AddDynamic(this, &UInventoryItemWidget::OnDropClicked);
                
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("🔍 Added Drop button to context menu"));
                }
                if (UCanvasPanelSlot* S = Root->AddChildToCanvas(Menu))
                {
                    const FVector2D ScreenPos = InMouseEvent.GetScreenSpacePosition();
                    const FVector2D Local = Root->GetCachedGeometry().AbsoluteToLocal(ScreenPos);
                    S->SetAnchors(FAnchors(0.f,0.f,0.f,0.f));
                    S->SetAlignment(FVector2D(0.f,0.f));
                    S->SetPosition(Local + FVector2D(6.f,6.f));
                    S->SetSize(FVector2D(140.f, 80.f));
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
                            else if (ItemData->SizeInCellsX == 3 && ItemData->SizeInCellsY == 3)
                            {
                                // Для тактического жилета 3x3
                                DropClass = ATacticalVest::StaticClass();
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

void UInventoryItemWidget::OnEquipClicked()
{
    if (!ItemData) 
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("❌ ItemData is null"));
        return;
    }
    
    UEquippableItemData* EquippableItem = Cast<UEquippableItemData>(ItemData);
    if (!EquippableItem) 
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
            FString::Printf(TEXT("❌ ItemData is not UEquippableItemData. Class: %s"), 
                ItemData ? *ItemData->GetClass()->GetName() : TEXT("null")));
        return;
    }
    
    // Получаем PlayerCharacter
    if (UInventoryWidget* Parent = GetTypedOuter<UInventoryWidget>())
    {
        APlayerController* PC = Parent->GetOwningPlayer();
        if (PC)
        {
            APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(PC->GetPawn());
            if (PlayerChar && PlayerChar->InventoryComponent)
            {
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                        FString::Printf(TEXT("✅ PlayerCharacter found, InventoryComponent: %s"), 
                            PlayerChar->InventoryComponent ? TEXT("OK") : TEXT("NULL")));
                }
                
                UInventoryComponent* InvComp = PlayerChar->InventoryComponent;
                
                // Экипируем предмет
                if (InvComp->EquipItemFromInventory(EquippableItem))
                {
                    if (GEngine)
                    {
                        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                            FString::Printf(TEXT("✅ Экипировано: %s"), *EquippableItem->DisplayName.ToString()));
                    }
                    
                    // Обновляем UI
                    Parent->SyncBackpack(InvComp->BackpackItems);
                    Parent->UpdateEquipmentSlots(); // Обновляем слоты экипировки
                }
                else
                {
                    if (GEngine)
                    {
                        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
                            TEXT("❌ Не удалось экипировать предмет"));
                    }
                }
            }
        }
    }
    
    // Закрыть меню
    if (UInventoryWidget* Parent = GetTypedOuter<UInventoryWidget>())
    {
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

void UInventoryItemWidget::OnUnequipClicked()
{
    if (!ItemData) 
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("❌ ItemData is null"));
        return;
    }
    
    UEquippableItemData* EquippableItem = Cast<UEquippableItemData>(ItemData);
    if (!EquippableItem) 
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
            FString::Printf(TEXT("❌ ItemData is not UEquippableItemData. Class: %s"), 
                ItemData ? *ItemData->GetClass()->GetName() : TEXT("null")));
        return;
    }
    
    // Получаем PlayerCharacter
    if (UInventoryWidget* Parent = GetTypedOuter<UInventoryWidget>())
    {
        APlayerController* PC = Parent->GetOwningPlayer();
        if (PC)
        {
            APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(PC->GetPawn());
            if (PlayerChar && PlayerChar->InventoryComponent)
            {
                UInventoryComponent* InvComp = PlayerChar->InventoryComponent;
                
                // Снимаем предмет
                if (InvComp->UnequipItemToInventory(EquippableItem->EquipmentSlot, false))
                {
                    if (GEngine)
                    {
                        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                            FString::Printf(TEXT("✅ Снято: %s"), *EquippableItem->DisplayName.ToString()));
                    }
                    
                    // Обновляем UI
                    Parent->SyncBackpack(InvComp->BackpackItems);
                    Parent->UpdateEquipmentSlots(); // Обновляем слоты экипировки
                }
                else
                {
                    if (GEngine)
                    {
                        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
                            TEXT("❌ Не удалось снять предмет"));
                    }
                }
            }
        }
    }
    
    // Закрыть меню
    if (UInventoryWidget* Parent = GetTypedOuter<UInventoryWidget>())
    {
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


