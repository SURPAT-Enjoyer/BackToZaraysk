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
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
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
    SetVisibility(ESlateVisibility::Visible);
    SetIsEnabled(true);
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
                Menu->SetPadding(FMargin(8.f));
                
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
                        FString::Printf(TEXT("🔍 Creating context menu for item: %s"), 
                            ItemData ? *ItemData->DisplayName.ToString() : TEXT("null")));
                }
                
                // Контейнер для вертикального стека кнопок
                UVerticalBox* VBox = Parent->WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ContextMenuVBox"));
                Menu->SetContent(VBox);

                // Кнопки для экипируемых предметов
                UEquippableItemData* EquippableItem = Cast<UEquippableItemData>(ItemData);
                if (ItemData && EquippableItem)
                {
                    // Проверяем, экипирован ли предмет
                    if (EquippableItem->bIsEquipped)
                    {
                        // Снять
                        UButton* UnequipBtn = Parent->WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
                        UTextBlock* UnequipTxt = Parent->WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
                        UnequipTxt->SetText(FText::FromString(TEXT("Снять")));
                        UnequipBtn->AddChild(UnequipTxt);
                        VBox->AddChildToVerticalBox(UnequipBtn);
                        // Для рюкзака в слоте делаем кнопку неактивной и визуально тусклой
                        if (EquippableItem->EquipmentSlot == Backpack)
                        {
                            UnequipBtn->SetIsEnabled(false);
                            UnequipBtn->SetRenderOpacity(0.5f);
                            UnequipTxt->SetColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 0.7f));
                        }
                        UnequipBtn->OnClicked.Clear();
                        UnequipBtn->OnClicked.AddDynamic(this, &UInventoryItemWidget::OnUnequipClicked);
                    }
                    else
                    {
                        // Надеть
                        UButton* EquipBtn = Parent->WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
                        UTextBlock* EquipTxt = Parent->WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
                        EquipTxt->SetText(FText::FromString(TEXT("Надеть")));
                        EquipBtn->AddChild(EquipTxt);
                        VBox->AddChildToVerticalBox(EquipBtn);
                        EquipBtn->OnClicked.Clear();
                        EquipBtn->OnClicked.AddDynamic(this, &UInventoryItemWidget::OnEquipClicked);
                    }
                }
                
                // Кнопка "Выбросить"
                {
                UButton* DropBtn = Parent->WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
                UTextBlock* Txt = Parent->WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
                    Txt->SetText(FText::FromString(TEXT("Выбросить")));
                    Txt->SetColorAndOpacity(FLinearColor::White);
                DropBtn->AddChild(Txt);
                    VBox->AddChildToVerticalBox(DropBtn);
                    DropBtn->OnClicked.Clear();
                    DropBtn->OnClicked.AddDynamic(this, &UInventoryItemWidget::OnDropClicked);
                }
                
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
                    S->SetSize(FVector2D(220.f, 200.f));
                    S->SetZOrder(9999);
                }
            }
        }
        return FReply::Handled();
    }
    if (Button == EKeys::LeftMouseButton)
    {
        // Запускаем drag через DetectDragIfPressed (он вызовет NativeOnDragDetected)
        return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
    }
    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UInventoryItemWidget::OnDropClicked()
{
    if (UInventoryWidget* Parent = GetTypedOuter<UInventoryWidget>())
    {
        if (!ItemData) return;
        
        APlayerController* PC = Parent->GetOwningPlayer();
        if (!PC) return;
        
        APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(PC->GetPawn());
        if (!PlayerChar || !PlayerChar->InventoryComponent) return;
        
        UInventoryComponent* InvComp = PlayerChar->InventoryComponent;
        bool bItemRemoved = false;

        // Если предмет экипирован и это рюкзак — снимаем и выбрасываем в мир
        if (UEquippableItemData* EquippableItem = Cast<UEquippableItemData>(ItemData))
        {
            if (EquippableItem->bIsEquipped && EquippableItem->EquipmentSlot == Backpack)
            {
                if (InvComp->UnequipItemToInventory(Backpack, true))
                {
                    bItemRemoved = true;
                    // Обновляем UI экипировки и грид рюкзака
                    Parent->UpdateEquipmentSlots();
                    Parent->UpdateBackpackStorageGrid();
                }
            }
        }
        
        // Проверяем, экипирован ли предмет
        UEquippableItemData* EquippableItem = Cast<UEquippableItemData>(ItemData);
        if (EquippableItem && EquippableItem->bIsEquipped)
        {
            // Снимаем экипированный предмет и выбрасываем его
            if (InvComp->UnequipItemToInventory(EquippableItem->EquipmentSlot, true))
            {
                bItemRemoved = true;
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange, 
                        FString::Printf(TEXT("🗑️ Экипированный предмет %s снят и выброшен"), *ItemData->DisplayName.ToString()));
                }
            }
        }
        else
        {
            // Обычный предмет из инвентаря
            if (InvComp->RemoveSpecificFromBackpack(ItemData))
            {
                bItemRemoved = true;
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange, 
                        FString::Printf(TEXT("🗑️ Предмет %s удален из инвентаря и выброшен"), *ItemData->DisplayName.ToString()));
                }
            }
        }
        
        if (bItemRemoved)
        {
            // Спавним предмет перед игроком
            FVector ViewLoc; FRotator ViewRot; PC->GetPlayerViewPoint(ViewLoc, ViewRot);
                            const FVector SpawnLoc = ViewLoc + ViewRot.Vector() * 80.f;
            FActorSpawnParameters S; 
            S.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
            
                            // Определяем Pickup-класс через единый маппер
                            extern TSubclassOf<AActor> GetPickupClassForItem_Internal(const UInventoryItemData* ItemData);
                            TSubclassOf<AActor> DropClass = GetPickupClassForItem_Internal(ItemData);
            
                            if (UWorld* World = GetWorld())
                            {
                                World->SpawnActor<AActor>(DropClass, SpawnLoc, ViewRot, S);
            }
            
            // Обновляем UI (принудительно)
            Parent->RefreshInventoryUI();
            
            // Удаляем виджет предмета
            Parent->RemoveItemWidget(this);
            Parent->RemoveItemMapping(ItemData);
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
                    
                    // Очищаем позицию предмета из системы drag & drop
                    Parent->ClearItemPosition(EquippableItem);
                    
                    // Обновляем UI
                    Parent->SyncBackpack(InvComp->BackpackItems);
                    
                    // Принудительно обновляем гриды после экипировки (с небольшой задержкой)
                    FTimerHandle TimerHandleEquip;
                    UInventoryWidget* ParentWidgetEquip = Parent;
                    GetWorld()->GetTimerManager().SetTimer(TimerHandleEquip, [ParentWidgetEquip, EquippableItem]()
                    {
                        if (ParentWidgetEquip)
                        {
                            if (EquippableItem->EquipmentSlot == Vest)
                            {
                                ParentWidgetEquip->UpdateVestGrid();
                                if (GEngine)
                                {
                                    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, TEXT("🔄 Принудительно обновлен грид жилета после экипировки"));
                                }
                            }
                            else if (EquippableItem->EquipmentSlot == Backpack)
                            {
                                // Backpack grid removed - no longer needed
                                if (GEngine)
                                {
                                    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, TEXT("🔄 Рюкзак экипирован (грид удален)"));
                                }
                            }
                        }
                    }, 0.1f, false);
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
                        
                        // Диагностика размера рюкзака перед обновлением UI
                        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, 
                            FString::Printf(TEXT("🔍 Backpack size before UI update: %d"), InvComp->BackpackItems.Num()));
                        
                        // Проверяем, есть ли предмет в рюкзаке
                        if (InvComp->BackpackItems.Contains(EquippableItem))
                        {
                            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                                TEXT("✅ Item found in backpack after unequip"));
                        }
                        else
                        {
                            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
                                TEXT("❌ Item NOT found in backpack after unequip"));
                        }
                        
                        // Проверяем флаг bIsEquipped
                        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, 
                            FString::Printf(TEXT("🔍 Item bIsEquipped flag: %s"), EquippableItem->bIsEquipped ? TEXT("true") : TEXT("false")));
                    }
                    
                    // Принудительно очищаем соответствующие UI зоны
                    if (EquippableItem->EquipmentSlot == Vest)
                    {
                        if (GEngine)
                        {
                            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("🗑️ Снят жилет - принудительно очищаем гриды"));
                        }
                        Parent->ForceClearVestGrids();
                    }
                    else if (EquippableItem->EquipmentSlot == Backpack)
                    {
                        // Для рюкзака: принудительно прячем/удаляем его грид
                        if (GEngine)
                        {
                            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("🗑️ Снят рюкзак - скрываем грид хранилища"));
                        }
                        Parent->UpdateEquipmentSlots();
                        Parent->UpdateBackpackStorageGrid();
                    }
                    
                    // Принудительно очищаем грид рюкзака при снятии рюкзака
                    if (EquippableItem->EquipmentSlot == Backpack)
                    {
                        if (GEngine)
                        {
                            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("🗑️ Снят рюкзак - принудительно очищаем грид"));
                        }
                        
                        // Backpack grid removed - no longer needed
                    }
                    
                    // Обновляем UI
                    Parent->RefreshInventoryUI();
                    
                    // Для других предметов (не жилет и не рюкзак) просто обновляем гриды
                    if (EquippableItem->EquipmentSlot != Vest && EquippableItem->EquipmentSlot != Backpack)
                    {
                        // Для других предметов просто обновляем гриды
                        FTimerHandle TimerHandle;
                        UInventoryWidget* ParentWidget = Parent;
                        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [ParentWidget]()
                        {
                            if (ParentWidget)
                            {
                                ParentWidget->UpdateVestGrid();
                                // Backpack grid removed - no longer needed
                                if (GEngine)
                                {
                                    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, TEXT("🔄 Принудительно обновлен грид жилета после снятия"));
                                }
                            }
                        }, 0.1f, false);
                    }
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
    // Разрешаем нормальный drag даже если предмет находится в гриде —
    // перенос/удаление обработаем на стадии Drop
    UDragDropOperation* Op = UWidgetBlueprintLibrary::CreateDragDropOperation(UDragDropOperation::StaticClass());
    if (Op)
    {
        UInventoryItemWidget* DragVisual = CreateWidget<UInventoryItemWidget>(GetOwningPlayer(), UInventoryItemWidget::StaticClass());
        if (DragVisual)
        {
            UTexture2D* IconTex = (ItemData && ItemData->Icon)
                ? ItemData->Icon
                : LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture"));
            DragVisual->Init(ItemData, IconTex, FVector2D(60.f, 60.f));
            DragVisual->bRotated = bRotated;
            DragVisual->UpdateVisualSize(FVector2D(60.f, 60.f));
            DragVisual->SetTint(FLinearColor(1.f, 1.f, 1.f, 0.85f));
            DragVisual->SetVisibility(ESlateVisibility::HitTestInvisible);
            Op->DefaultDragVisual = DragVisual;
        }
        Op->Payload = this;
        Op->Pivot = EDragPivot::MouseDown;
        OutOperation = Op;
    }
    // Подсветка исходного виджета
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


