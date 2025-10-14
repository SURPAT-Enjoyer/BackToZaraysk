#include "EquippableItemData.h"
#include "Engine/Engine.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"

UEquippableItemData::UEquippableItemData()
{
    // Значения по умолчанию уже установлены в заголовочном файле
    // ItemType наследуется от UInventoryItemData
    // EquipmentSlot = None по умолчанию
}

UTacticalVestItemData::UTacticalVestItemData()
{
    DisplayName = FText::FromString(TEXT("Тактический жилет"));
    SizeInCellsX = 3;
    SizeInCellsY = 3;
    EquipmentSlot = Vest;
    AttachSocketName = FName(TEXT("spine_02")); // Прикрепляем к кости груди
    bRotatable = false; // Жилет нельзя вращать
    
    // Настройки дополнительного хранилища (грид как на картинке: 1+2+2+2+2+1)
    bHasAdditionalStorage = true;
    AdditionalGridSize = FIntPoint(6, 2); // 6 колонок, 2 ряда
    
    // Загружаем меш тактического жилета
    FString MeshPath = TEXT("/Game/Insurgent_2/Mesh/Separate_Parts/SK_ChestRigSmall.SK_ChestRigSmall");
    EquippedMesh = LoadObject<USkeletalMesh>(nullptr, *MeshPath);
    
    if (EquippedMesh && EquippedMesh->IsValidLowLevel())
    {
        UE_LOG(LogTemp, Log, TEXT("UTacticalVestItemData: Successfully loaded EquippedMesh: %s"), *EquippedMesh->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("UTacticalVestItemData: Failed to load EquippedMesh from path: %s"), *MeshPath);
        EquippedMesh = nullptr; // Убеждаемся, что указатель null
    }
    
    // Устанавливаем относительный трансформ для тактического жилета
    // Поворачиваем жилет вертикально и на 90 градусов
    RelativeTransform = FTransform(
        FRotator(0.0f, 90.0f, 90.0f),   // Поворот: вертикально (90°) + поворот на 90°
        FVector(5.0f, 0.0f, 0.0f),      // Небольшое смещение вперед
        FVector(1.0f, 1.0f, 1.0f)       // Масштаб
    );
}

UBackpackItemData::UBackpackItemData()
{
    DisplayName = FText::FromString(TEXT("Рюкзак"));
    SizeInCellsX = 8;
    SizeInCellsY = 6;
    EquipmentSlot = Backpack;
    AttachSocketName = FName(TEXT("spine_03")); // Прикрепляем к спине
    bRotatable = false; // Рюкзак нельзя вращать
    
    // Настройки дополнительного хранилища (грид 8x6)
    bHasAdditionalStorage = true;
    AdditionalGridSize = FIntPoint(8, 6); // 8 колонок, 6 рядов
    
    // Задаём меш рюкзака: используем гарантированный статический куб из Engine
    FString MeshPath = TEXT("/Engine/BasicShapes/Cube.Cube");
    EquippedMesh = LoadObject<UStaticMesh>(nullptr, *MeshPath);
    
    if (EquippedMesh && EquippedMesh->IsValidLowLevel())
    {
        UE_LOG(LogTemp, Log, TEXT("UBackpackItemData: Successfully loaded EquippedMesh: %s"), *EquippedMesh->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("UBackpackItemData: Failed to load EquippedMesh from path: %s"), *MeshPath);
        EquippedMesh = nullptr; // Убеждаемся, что указатель null
    }
    
    // Относительный трансформ — визуально как рюкзак на спине
    RelativeTransform = FTransform(
        FRotator(0.0f, 0.0f, 0.0f),      // без поворота
        FVector(-12.0f, 0.0f, -4.0f),    // слегка назад и вниз
        FVector(0.28f, 0.16f, 0.36f)     // похожие пропорции на рюкзак
    );
}




