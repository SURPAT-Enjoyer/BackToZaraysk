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
#include "BackToZaraysk/GameData/Items/Test/PickupBase.h"
#include "BackToZaraysk/GameData/Items/EquipmentBase.h"
#include "BackToZaraysk/Inventory/EquippableItemData.h"
#include "BackToZaraysk/Inventory/EquipmentSlotType.h"
#include "BackToZaraysk/Inventory/InventoryBlueprintLibrary.h"
#include "BackToZaraysk/Components/EquipmentComponent.h"
#include "BackToZaraysk/Characters/PlayerCharacter.h"
#include "BackToZaraysk/GameData/Items/TacticalVest.h"
#include "Components/SkeletalMeshComponent.h"

namespace
{
    static void EnsureDroppedActorHasPhysics(AActor* SpawnedActor)
    {
        if (!SpawnedActor) return;
        if (AEquipmentBase* Equip = Cast<AEquipmentBase>(SpawnedActor))
        {
            Equip->SetActorHiddenInGame(false);

            // ВАЖНО: даже если визуал — SkeletalMesh, оставляем коллизию/физику на статическом Mesh (может быть скрыт),
            // иначе предметы с SkeletalMesh без PhysicsAsset могут проваливаться/исчезать.
            if (Equip->Mesh)
            {
                Equip->Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                Equip->Mesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
                Equip->Mesh->SetSimulatePhysics(true);
                Equip->Mesh->SetEnableGravity(true);
                Equip->Mesh->SetUseCCD(true);
                Equip->Mesh->AddImpulse(FVector(0.f, 0.f, 150.f), NAME_None, true);
            }
            if (Equip->SkeletalMesh)
            {
                Equip->SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                Equip->SkeletalMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
                Equip->SkeletalMesh->SetUseCCD(true);
                // SimulatePhysics для SkeletalMesh включаем только если есть PhysicsAsset
                if (Equip->SkeletalMesh->GetPhysicsAsset())
                {
                    Equip->SkeletalMesh->SetSimulatePhysics(true);
                    Equip->SkeletalMesh->SetEnableGravity(true);
                }
            }
            return;
        }
        if (APickupBase* PB = Cast<APickupBase>(SpawnedActor))
        {
            if (PB->Mesh)
            {
                PB->Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                PB->Mesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
                PB->Mesh->SetSimulatePhysics(true);
                PB->Mesh->SetEnableGravity(true);
                PB->Mesh->SetUseCCD(true);
                PB->Mesh->AddImpulse(FVector(0.f, 0.f, 150.f), NAME_None, true);
            }
        }
    }

    static void ForceApplyWorldVisualsForDroppedItem(AActor* SpawnedActor, UInventoryItemData* ItemData)
    {
        if (!SpawnedActor || !ItemData) return;

        if (AEquipmentBase* Equip = Cast<AEquipmentBase>(SpawnedActor))
        {
            UEquipmentComponent::SetupDroppedEquipmentVisual(Equip, ItemData);
            return;
        }

        if (APickupBase* Pickup = Cast<APickupBase>(SpawnedActor))
        {
            Pickup->ItemInstance = ItemData;
            Pickup->ApplyItemInstanceVisuals();
            EnsureDroppedActorHasPhysics(SpawnedActor);
            return;
        }
    }
}

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
        UTexture2D* IconToUse = InIcon;
        if (!IconToUse)
        {
            // Если в данных задана иконка – используем её.
            if (InData && InData->Icon)
            {
                IconToUse = InData->Icon;
            }
            else
            {
                // Иначе подставляем квадрат по умолчанию в зависимости от типа данных.
                if (Cast<UEquipModBaseItemData>(InData))
                {
                    // Красный квадрат для модов.
                    IconToUse = LoadObject<UTexture2D>(nullptr, TEXT("/Game/UI/T_RedSquare.T_RedSquare"));
                }
                else
                {
                    // Синий квадрат по умолчанию для ItemBase/прочих UInventoryItemData.
                    IconToUse = LoadObject<UTexture2D>(nullptr, TEXT("/Game/UI/T_BlueSquare.T_BlueSquare"));
                }
            }
        }

        if (!IconToUse)
        {
            IconToUse = LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture"));
        }

        FSlateBrush Brush;
        Brush.SetResourceObject(IconToUse);
        Brush.ImageSize = FVector2D(CellSize.X * SizeX, CellSize.Y * SizeY);
        Image->SetBrush(Brush);
    }
}

void UInventoryItemWidget::NativeConstruct()
{
    Super::NativeConstruct();
    SetIsFocusable(true); // позволяем ловить R на виджете предмета
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
                                // Разрешаем "Снять" для слотов, которые реально поддерживаем в UI/логике
                                const bool bSupportedUnequip =
                                    (EquippableItem->EquipmentSlot == Helmet) ||
                                    (EquippableItem->EquipmentSlot == Vest) ||
                                    (EquippableItem->EquipmentSlot == Belt) ||
                                    (EquippableItem->EquipmentSlot == Backpack) ||
                                    (EquippableItem->EquipmentSlot == Armor);
                                UnequipBtn->SetIsEnabled(bSupportedUnequip);
                                if (!bSupportedUnequip)
                                {
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

                        // По умолчанию — можно экипировать
                        bool bCanEquip = true;

                        // Дополнительные ограничения: бронежилет/разгрузка + моды
                        APlayerController* PC = Parent->GetOwningPlayer();
                        APlayerCharacter* PlayerChar = PC ? Cast<APlayerCharacter>(PC->GetPawn()) : nullptr;
                        UInventoryComponent* InvComp = PlayerChar ? PlayerChar->InventoryComponent : nullptr;
                        if (InvComp)
                        {
                            // Нельзя надеть разгрузку, если на броне есть моды
                            if (EquippableItem->EquipmentSlot == Vest)
                            {
                                if (UEquippableItemData* EquippedArmor = InvComp->GetEquippedItem(Armor))
                                {
                                    if (EquippedArmor->InstalledMods.Num() > 0)
                                    {
                                        bCanEquip = false;
                                    }
                                }
                            }

                            // Нельзя надеть бронежилет с модами, если уже надета разгрузка
                            if (EquippableItem->EquipmentSlot == Armor && EquippableItem->InstalledMods.Num() > 0)
                            {
                                if (UEquippableItemData* EquippedVest = InvComp->GetEquippedItem(Vest))
                                {
                                    bCanEquip = false;
                                }
                            }
                        }

                        EquipBtn->SetIsEnabled(bCanEquip);
                        if (!bCanEquip)
                        {
                            EquipBtn->SetRenderOpacity(0.5f);
                            EquipTxt->SetColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 0.7f));
                        }

                        EquipBtn->OnClicked.Clear();
                        EquipBtn->OnClicked.AddDynamic(this, &UInventoryItemWidget::OnEquipClicked);
                    }
                }

                // Кнопка "Модифицировать" для любого модифицируемого элемента экипировки.
                if (UEquippableItemData* EqArmor = Cast<UEquippableItemData>(ItemData))
                {
                    if (EqArmor->bIsModdable)
                    {
                        UButton* ModifyBtn = Parent->WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
                        UTextBlock* ModifyTxt = Parent->WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
                        ModifyTxt->SetText(FText::FromString(TEXT("Модифицировать")));
                        ModifyBtn->AddChild(ModifyTxt);
                        VBox->AddChildToVerticalBox(ModifyBtn);
                        ModifyBtn->OnClicked.Clear();
                        ModifyBtn->OnClicked.AddDynamic(this, &UInventoryItemWidget::OnModifyArmorClicked);
                    }
                }

                // Кнопка "Открыть" (только для модулей с внутренним гридом)
                if (UEquipModBaseItemData* Mod = Cast<UEquipModBaseItemData>(ItemData))
                {
                    if (Mod->bHasAdditionalStorage)
                    {
                        UButton* OpenBtn = Parent->WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
                        UTextBlock* OpenTxt = Parent->WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
                        OpenTxt->SetText(FText::FromString(TEXT("Открыть")));
                        OpenBtn->AddChild(OpenTxt);
                        VBox->AddChildToVerticalBox(OpenBtn);
                        OpenBtn->OnClicked.Clear();
                        OpenBtn->OnClicked.AddDynamic(this, &UInventoryItemWidget::OnOpenClicked);
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
        if (UInventoryWidget* Parent = GetTypedOuter<UInventoryWidget>())
        {
            if (Parent->IsArmorModInstallWaitingForClick() && Parent->GetOpenedArmorForMods() && ItemData &&
                Parent->TryInstallModOnArmorFromWaiting(ItemData))
            {
                return FReply::Handled();
            }
        }
        // Запускаем drag через DetectDragIfPressed (он вызовет NativeOnDragDetected)
        return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
    }
    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UInventoryItemWidget::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        if (UInventoryWidget* Parent = GetTypedOuter<UInventoryWidget>())
        {
            if (UEquipModBaseItemData* Mod = Cast<UEquipModBaseItemData>(ItemData))
            {
                if (Mod->bHasAdditionalStorage)
                {
                    Parent->OpenEquipModStorage(Mod);
                    return FReply::Handled();
                }
            }
        }
    }
    return Super::NativeOnMouseButtonDoubleClick(InGeometry, InMouseEvent);
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
                    // Pickup рюкзака уже заспавнен в EquipmentComponent с переносом ItemInstance.
                    // Здесь НИЧЕГО дополнительно не спавним, чтобы не было дублей и потери содержимого.
                    // Обновляем UI и выходим.
                    Parent->UpdateEquipmentSlots();
                    Parent->UpdateBackpackStorageGrid();
                    Parent->RefreshInventoryUI();

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
                    return; // критично: не выполнять дальнейший код спавна
                }
            }
        }
        
        // Проверяем, экипирован ли предмет
        UEquippableItemData* EquippableItem = Cast<UEquippableItemData>(ItemData);
        if (EquippableItem && EquippableItem->bIsEquipped)
        {
            // ВАЖНО: UnequipItemToInventory(..., true) уже спавнит объект в мире через EquipmentComponent.
            // Если мы пойдём дальше в общий путь bItemRemoved -> SpawnActor, получим дубль.
            if (InvComp->UnequipItemToInventory(EquippableItem->EquipmentSlot, true))
            {
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange,
                        FString::Printf(TEXT("🗑️ Экипированный предмет %s снят и выброшен (спавн выполнен Unequip)"), *ItemData->DisplayName.ToString()));
                }

                // Обновляем UI и выходим — НЕ спавним вручную второй раз
                Parent->UpdateEquipmentSlots();
                Parent->UpdateBackpackStorageGrid();
                Parent->UpdateVestGrid();
                Parent->RefreshInventoryUI();

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
                return;
            }

            // Fallback: если по какой-то причине снять+выбросить не удалось (несинхрон между bIsEquipped и EquipmentSlots),
            // принудительно удаляем предмет из слота/хранилищ и спавним его в мир вручную, чтобы он НЕ оставался в слоте.
            if (InvComp->RemoveFromAnyStorage(ItemData))
            {
                bItemRemoved = true;
            }
        }
        else
        {
            // Удаляем предмет из любого хранения (рюкзак, хранилища, пояс, карманы)
            if (InvComp->RemoveFromAnyStorage(ItemData))
            {
                bItemRemoved = true;
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange, 
                        FString::Printf(TEXT("🗑️ Предмет %s удален из хранения и выброшен"), *ItemData->DisplayName.ToString()));
                }
            }
        }
        
        if (bItemRemoved)
        {
            // Спавним предмет перед игроком (для НЕ-рюкзака). Для рюкзака спавн уже сделан выше и сюда мы не попадём.
            FVector ViewLoc; FRotator ViewRot; PC->GetPlayerViewPoint(ViewLoc, ViewRot);
                            FVector SpawnLoc = ViewLoc + ViewRot.Vector() * 80.f + FVector(0.f, 0.f, 100.f);
                            // Для бронежилета — спавним от позиции root на меше (иначе может улетать вверх из-за вьюпоинта/камеры)
                            if (UEquippableItemData* Eq = Cast<UEquippableItemData>(ItemData))
                            {
                                if (Eq->EquipmentSlot == Armor)
                                {
                                    if (APlayerCharacter* PlayerChar2 = Cast<APlayerCharacter>(PC->GetPawn()))
                                    {
                                        if (USkeletalMeshComponent* M = PlayerChar2->GetMesh())
                                        {
                                            const FName RootSocket(TEXT("root"));
                                            if (M->DoesSocketExist(RootSocket))
                                            {
                                                SpawnLoc = M->GetSocketLocation(RootSocket) + PlayerChar2->GetActorForwardVector() * 80.f;
                                            }
                                            else
                                            {
                                                SpawnLoc = PlayerChar2->GetActorLocation() + PlayerChar2->GetActorForwardVector() * 80.f;
                                            }
                                        }
                                    }
                                }
                            }
            FActorSpawnParameters S; 
            S.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
            
                            // Определяем Pickup-класс через единый маппер
                            TSubclassOf<AActor> DropClass = GetPickupClassForItem_Internal(ItemData);
            
            if (UWorld* World = GetWorld())
            {
                if (AActor* SpawnedActor = World->SpawnActor<AActor>(DropClass, SpawnLoc, ViewRot, S))
                {
                    // Передаём текущий экземпляр данных и принудительно применяем визуал (включая фолбэк для жилета)
                    ForceApplyWorldVisualsForDroppedItem(SpawnedActor, ItemData);
                }
            }
            
            // Обновляем UI (принудительно)
            Parent->RefreshInventoryUI();
            Parent->RefreshOpenedEquipModStorage();
            
            // Удаляем виджет предмета (важно: этот виджет может быть в окне модуля, а не в RightPanelRef)
            RemoveFromParent();
            Parent->ClearItemPosition(ItemData);
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

void UInventoryItemWidget::OnOpenClicked()
{
    if (UInventoryWidget* Parent = GetTypedOuter<UInventoryWidget>())
    {
        if (!ItemData) return;

        if (UEquipModBaseItemData* Mod = Cast<UEquipModBaseItemData>(ItemData))
        {
            Parent->OpenEquipModStorage(Mod);
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

void UInventoryItemWidget::OnModifyArmorClicked()
{
    if (!ItemData) return;

    if (UInventoryWidget* Parent = GetTypedOuter<UInventoryWidget>())
    {
        if (UEquippableItemData* ArmorData = Cast<UEquippableItemData>(ItemData))
        {
            if (ArmorData->bIsModdable)
            {
                Parent->OpenArmorModWindow(ArmorData);
            }
        }

        // Закрыть контекстное меню
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
                
                // ВАЖНО: EquipItemFromInventory требует, чтобы предмет был в BackpackItems.
                // Если предмет лежит в хранилище рюкзака/жилета/карманах, он не будет найден и экипировка провалится,
                // хотя кнопка контекстного меню активна.
                // Поэтому: удаляем предмет из любого хранения и временно добавляем в BackpackItems перед экипировкой.
                InvComp->RemoveFromAnyStorage(EquippableItem);
                InvComp->BackpackItems.AddUnique(EquippableItem);

                // Экипируем предмет
                const bool bEquipped = InvComp->EquipItemFromInventory(EquippableItem);
                if (bEquipped)
                {
                    if (GEngine)
                    {
                        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                            FString::Printf(TEXT("✅ Экипировано: %s"), *EquippableItem->DisplayName.ToString()));
                    }
                    
                    // Очищаем позицию предмета из системы drag & drop
                    Parent->ClearItemPosition(EquippableItem);
                    
                    // Обновляем UI
                    Parent->RefreshInventoryUI();
                    
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
                    // Если экипировка не удалась — убираем добавленный в BackpackItems предмет обратно
                    InvComp->RemoveSpecificFromBackpack(EquippableItem);
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
                
                // Снимаем жилет: если есть рюкзак — кладём в инвентарь, иначе — выбрасываем в мир
                bool bUnequipped = false;
                if (EquippableItem->EquipmentSlot == Vest)
                {
                    const bool bHasBackpack = (InvComp->GetEquippedItem(Backpack) != nullptr);
                    bUnequipped = InvComp->UnequipItemToInventory(Vest, bHasBackpack ? false : true);
                }
                else
                {
                    bUnequipped = InvComp->UnequipItemToInventory(EquippableItem->EquipmentSlot, false);
                }
                if (bUnequipped)
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
                    
                    // Принудительно очищаем соответствующие UI зоны и обновляем
                    if (EquippableItem->EquipmentSlot == Vest)
                    {
                        Parent->ForceClearVestGrids();
                        Parent->UpdateEquipmentSlots();
                        Parent->UpdateBackpackStorageGrid();
                        Parent->RefreshInventoryUI();
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
        // Локальная обработка вращения и обновление DragVisual, если активен drag
        bRotated = !bRotated;
        UpdateVisualSize(FVector2D(60.f, 60.f));
        if (UDragDropOperation* Op = UWidgetBlueprintLibrary::GetDragDroppingContent())
        {
            if (UInventoryItemWidget* DragVisual = Cast<UInventoryItemWidget>(Op->DefaultDragVisual))
            {
                DragVisual->bRotated = bRotated;
                DragVisual->UpdateVisualSize(FVector2D(60.f, 60.f));
            }
        }
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
            UTexture2D* IconTex = nullptr;
            if (ItemData && ItemData->Icon)
            {
                IconTex = ItemData->Icon;
            }
            else if (Cast<UEquipModBaseItemData>(ItemData))
            {
                IconTex = LoadObject<UTexture2D>(nullptr, TEXT("/Game/UI/T_RedSquare.T_RedSquare"));
            }
            else
            {
                IconTex = LoadObject<UTexture2D>(nullptr, TEXT("/Game/UI/T_BlueSquare.T_BlueSquare"));
            }
            if (!IconTex)
            {
                IconTex = LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture"));
            }
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


