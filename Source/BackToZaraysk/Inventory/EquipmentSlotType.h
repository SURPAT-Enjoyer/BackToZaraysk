#pragma once

#include "CoreMinimal.h"
#include "EquipmentSlotType.generated.h"

/**
 * Типы слотов экипировки
 */
UENUM(BlueprintType)
enum EEquipmentSlotType : uint8
{
    None            UMETA(DisplayName = "Нет"),
    Helmet          UMETA(DisplayName = "Шлем"),
    Vest            UMETA(DisplayName = "Разгрузка"),  // Разгрузочный жилет
    Backpack        UMETA(DisplayName = "Рюкзак"),
    PrimaryWeapon   UMETA(DisplayName = "Основное оружие"),
    SecondaryWeapon UMETA(DisplayName = "Запасное оружие"),
    Melee           UMETA(DisplayName = "Холодное оружие"),
    Armor           UMETA(DisplayName = "Бронежилет") // Бронежилет (без доп. хранилищ)
};




