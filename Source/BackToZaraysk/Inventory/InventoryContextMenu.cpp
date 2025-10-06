#include "InventoryContextMenu.h"
#include "InventoryItemData.h"
#include "EquippableItemData.h"
#include "InventoryComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

void UInventoryContextMenu::ShowMenuForItem(UInventoryItemData* Item, FVector2D ScreenPosition)
{
    if (!Item)
    {
        UE_LOG(LogTemp, Warning, TEXT("InventoryContextMenu: Item is null"));
        return;
    }
    
    CurrentItem = Item;
    UpdateMenuState();
    
    // Позиционируем меню на экране
    SetPositionInViewport(ScreenPosition);
    SetVisibility(ESlateVisibility::Visible);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, 
            FString::Printf(TEXT("📋 Контекстное меню: %s"), *Item->DisplayName.ToString()));
    }
}

void UInventoryContextMenu::HideMenu()
{
    CurrentItem = nullptr;
    bIsEquippableItem = false;
    bIsItemEquipped = false;
    SetVisibility(ESlateVisibility::Collapsed);
}

void UInventoryContextMenu::EquipItem()
{
    if (!CurrentItem)
    {
        UE_LOG(LogTemp, Warning, TEXT("InventoryContextMenu: No item selected"));
        return;
    }
    
    UEquippableItemData* EquippableItem = Cast<UEquippableItemData>(CurrentItem);
    if (!EquippableItem)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
                TEXT("❌ Этот предмет нельзя экипировать!"));
        }
        return;
    }
    
    // Получаем компонент инвентаря игрока
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC) return;
    
    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn) return;
    
    UInventoryComponent* InventoryComp = PlayerPawn->FindComponentByClass<UInventoryComponent>();
    if (!InventoryComp)
    {
        UE_LOG(LogTemp, Error, TEXT("InventoryContextMenu: InventoryComponent not found!"));
        return;
    }
    
    // Экипируем предмет
    if (InventoryComp->EquipItemFromInventory(EquippableItem))
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                FString::Printf(TEXT("✅ Экипировано: %s"), *EquippableItem->DisplayName.ToString()));
        }
        HideMenu();
    }
}

void UInventoryContextMenu::UnequipItem()
{
    if (!CurrentItem)
    {
        UE_LOG(LogTemp, Warning, TEXT("InventoryContextMenu: No item selected"));
        return;
    }
    
    UEquippableItemData* EquippableItem = Cast<UEquippableItemData>(CurrentItem);
    if (!EquippableItem || !EquippableItem->bIsEquipped)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
                TEXT("❌ Предмет не экипирован!"));
        }
        return;
    }
    
    // Получаем компонент инвентаря игрока
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC) return;
    
    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn) return;
    
    UInventoryComponent* InventoryComp = PlayerPawn->FindComponentByClass<UInventoryComponent>();
    if (!InventoryComp)
    {
        UE_LOG(LogTemp, Error, TEXT("InventoryContextMenu: InventoryComponent not found!"));
        return;
    }
    
    // Снимаем предмет
    if (InventoryComp->UnequipItemToInventory(EquippableItem->EquipmentSlot))
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                FString::Printf(TEXT("✅ Снято: %s"), *EquippableItem->DisplayName.ToString()));
        }
        HideMenu();
    }
}

void UInventoryContextMenu::UseItem()
{
    if (!CurrentItem)
    {
        UE_LOG(LogTemp, Warning, TEXT("InventoryContextMenu: No item selected"));
        return;
    }
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
            TEXT("⚠️ UseItem - функционал еще не реализован"));
    }
    
    HideMenu();
}

void UInventoryContextMenu::DropItem()
{
    if (!CurrentItem)
    {
        UE_LOG(LogTemp, Warning, TEXT("InventoryContextMenu: No item selected"));
        return;
    }
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
            TEXT("⚠️ DropItem - функционал еще не реализован"));
    }
    
    HideMenu();
}

void UInventoryContextMenu::SplitStack()
{
    if (!CurrentItem)
    {
        UE_LOG(LogTemp, Warning, TEXT("InventoryContextMenu: No item selected"));
        return;
    }
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
            TEXT("⚠️ SplitStack - функционал еще не реализован"));
    }
    
    HideMenu();
}

bool UInventoryContextMenu::CanEquipItem() const
{
    if (!CurrentItem) return false;
    
    UEquippableItemData* EquippableItem = Cast<UEquippableItemData>(CurrentItem);
    if (!EquippableItem) return false;
    
    // Нельзя экипировать уже экипированный предмет
    return !EquippableItem->bIsEquipped;
}

bool UInventoryContextMenu::CanUnequipItem() const
{
    if (!CurrentItem) return false;
    
    UEquippableItemData* EquippableItem = Cast<UEquippableItemData>(CurrentItem);
    if (!EquippableItem) return false;
    
    // Можно снять только экипированный предмет
    return EquippableItem->bIsEquipped;
}

bool UInventoryContextMenu::CanUseItem() const
{
    // Пока все предметы можно "использовать" (заглушка)
    return CurrentItem != nullptr;
}

void UInventoryContextMenu::UpdateMenuState()
{
    if (!CurrentItem)
    {
        bIsEquippableItem = false;
        bIsItemEquipped = false;
        return;
    }
    
    UEquippableItemData* EquippableItem = Cast<UEquippableItemData>(CurrentItem);
    bIsEquippableItem = (EquippableItem != nullptr);
    bIsItemEquipped = bIsEquippableItem && EquippableItem->bIsEquipped;
    
    UE_LOG(LogTemp, Log, TEXT("InventoryContextMenu: Item=%s, Equippable=%d, Equipped=%d"),
        *CurrentItem->DisplayName.ToString(), bIsEquippableItem, bIsItemEquipped);
}




