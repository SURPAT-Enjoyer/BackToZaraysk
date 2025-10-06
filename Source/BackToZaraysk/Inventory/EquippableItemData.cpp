#include "EquippableItemData.h"

UEquippableItemData::UEquippableItemData()
{
    // Значения по умолчанию уже установлены в заголовочном файле
    // ItemType наследуется от UInventoryItemData
    // EquipmentSlot = EEquipmentSlotType::None по умолчанию
}

UTacticalVestItemData::UTacticalVestItemData()
{
    DisplayName = FText::FromString(TEXT("Тактический жилет"));
    SizeInCellsX = 3;
    SizeInCellsY = 3;
    EquipmentSlot = EEquipmentSlotType::Vest;
    AttachSocketName = FName(TEXT("spine_02")); // Прикрепляем к кости груди
    bRotatable = false; // Жилет нельзя вращать
    
    // Устанавливаем относительный трансформ для тактического жилета
    // Поворачиваем жилет вертикально и на 90 градусов
    RelativeTransform = FTransform(
        FRotator(0.0f, 90.0f, 90.0f),   // Поворот: вертикально (90°) + поворот на 90°
        FVector(5.0f, 0.0f, 0.0f),      // Небольшое смещение вперед
        FVector(1.0f, 1.0f, 1.0f)       // Масштаб
    );
}




