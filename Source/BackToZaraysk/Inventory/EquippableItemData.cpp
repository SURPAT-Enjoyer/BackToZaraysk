#include "EquippableItemData.h"

UEquippableItemData::UEquippableItemData()
{
    ItemType = EInventoryItemType::Generic;
    bRotatable = true;
}

UTacticalVestItemData::UTacticalVestItemData()
{
    DisplayName = FText::FromString(TEXT("Тактический жилет"));
    SizeInCellsX = 3;
    SizeInCellsY = 3;
    EquipmentSlot = EEquipmentSlotType::Vest;
    AttachSocketName = FName(TEXT("spine_03")); // Прикрепляем к позвоночнику
    bRotatable = false; // Жилет нельзя вращать
    
    // Устанавливаем относительный трансформ (будет настроен в Blueprint)
    RelativeTransform = FTransform::Identity;
}




