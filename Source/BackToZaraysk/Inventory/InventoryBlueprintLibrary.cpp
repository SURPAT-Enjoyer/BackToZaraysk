#include "InventoryBlueprintLibrary.h"
#include "InventoryComponent.h"
#include "EquippableItemData.h"
#include "BackToZaraysk/Characters/PlayerCharacter.h"
#include "BackToZaraysk/Components/EquipmentComponent.h"
#include "BackToZaraysk/GameData/Items/TacticalVest.h"
#include "Kismet/GameplayStatics.h"

bool UInventoryBlueprintLibrary::EquipItemOnPlayer(UObject* WorldContextObject, UEquippableItemData* Item)
{
    if (!Item)
    {
        UE_LOG(LogTemp, Warning, TEXT("InventoryBlueprintLibrary: Item is null"));
        return false;
    }
    
    APlayerCharacter* PlayerChar = GetPlayerCharacter(WorldContextObject);
    if (!PlayerChar || !PlayerChar->InventoryComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("InventoryBlueprintLibrary: PlayerCharacter or InventoryComponent not found"));
        return false;
    }
    
    return PlayerChar->InventoryComponent->EquipItemFromInventory(Item);
}

bool UInventoryBlueprintLibrary::UnequipItemFromSlot(UObject* WorldContextObject, EEquipmentSlotType SlotType)
{
    APlayerCharacter* PlayerChar = GetPlayerCharacter(WorldContextObject);
    if (!PlayerChar || !PlayerChar->InventoryComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("InventoryBlueprintLibrary: PlayerCharacter or InventoryComponent not found"));
        return false;
    }
    
    return PlayerChar->InventoryComponent->UnequipItemToInventory(SlotType);
}

UEquippableItemData* UInventoryBlueprintLibrary::GetEquippedItemFromSlot(UObject* WorldContextObject, EEquipmentSlotType SlotType)
{
    APlayerCharacter* PlayerChar = GetPlayerCharacter(WorldContextObject);
    if (!PlayerChar || !PlayerChar->InventoryComponent)
    {
        return nullptr;
    }
    
    return PlayerChar->InventoryComponent->GetEquippedItem(SlotType);
}

bool UInventoryBlueprintLibrary::IsEquipmentSlotOccupied(UObject* WorldContextObject, EEquipmentSlotType SlotType)
{
    APlayerCharacter* PlayerChar = GetPlayerCharacter(WorldContextObject);
    if (!PlayerChar || !PlayerChar->EquipmentComponent)
    {
        return false;
    }
    
    return PlayerChar->EquipmentComponent->IsSlotOccupied(SlotType);
}

FText UInventoryBlueprintLibrary::GetEquipmentSlotName(EEquipmentSlotType SlotType)
{
    switch (SlotType)
    {
        case None:
            return FText::FromString(TEXT("Нет"));
        case Helmet:
            return FText::FromString(TEXT("Шлем"));
        case Vest:
            return FText::FromString(TEXT("Разгрузка"));
        case Backpack:
            return FText::FromString(TEXT("Рюкзак"));
        case PrimaryWeapon:
            return FText::FromString(TEXT("Основное оружие"));
        case SecondaryWeapon:
            return FText::FromString(TEXT("Запасное оружие"));
        case Melee:
            return FText::FromString(TEXT("Холодное оружие"));
        default:
            return FText::FromString(TEXT("Неизвестно"));
    }
}

AActor* UInventoryBlueprintLibrary::SpawnTestTacticalVest(UObject* WorldContextObject, FVector SpawnLocation)
{
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("InventoryBlueprintLibrary: World not found"));
        return nullptr;
    }
    
    // Спавним тактический жилет
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    ATacticalVest* Vest = World->SpawnActor<ATacticalVest>(
        ATacticalVest::StaticClass(),
        SpawnLocation,
        FRotator::ZeroRotator,
        SpawnParams
    );
    
    if (Vest)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
                TEXT("✅ Тестовый тактический жилет создан!"));
        }
    }
    else
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, 
                TEXT("❌ Ошибка создания тактического жилета!"));
        }
    }
    
    return Vest;
}

APlayerCharacter* UInventoryBlueprintLibrary::GetPlayerCharacter(UObject* WorldContextObject)
{
    if (!WorldContextObject)
    {
        UE_LOG(LogTemp, Error, TEXT("InventoryBlueprintLibrary: WorldContextObject is null"));
        return nullptr;
    }
    
    APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0);
    if (!PC)
    {
        UE_LOG(LogTemp, Error, TEXT("InventoryBlueprintLibrary: PlayerController not found"));
        return nullptr;
    }
    
    return Cast<APlayerCharacter>(PC->GetPawn());
}




