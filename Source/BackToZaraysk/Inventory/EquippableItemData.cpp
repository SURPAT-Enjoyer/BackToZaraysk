#include "EquippableItemData.h"
#include "Engine/Engine.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "UObject/ObjectRedirector.h"
#include "BackToZaraysk/GameData/Items/HelmetBase.h"
#include "BackToZaraysk/GameData/Items/TacticalVest.h"
#include "BackToZaraysk/GameData/Items/VestBase.h"
#include "BackToZaraysk/GameData/Items/Test/PickupBackpack.h"
#include "BackToZaraysk/GameData/Items/BackpackBase.h"
#include "BackToZaraysk/GameData/Items/ArmorBase.h"
#include "BackToZaraysk/GameData/Items/BulletproofVestBege.h"

static UTexture2D* LoadTexture2DWithRedirect(const TCHAR* Path)
{
    if (!Path || !*Path) return nullptr;

    if (UTexture2D* Direct = LoadObject<UTexture2D>(nullptr, Path))
    {
        return Direct;
    }

    UObject* AnyObj = StaticLoadObject(UObject::StaticClass(), nullptr, Path);
    if (!AnyObj) return nullptr;

    if (UObjectRedirector* Redirector = Cast<UObjectRedirector>(AnyObj))
    {
        AnyObj = Redirector->DestinationObject;
    }

    return Cast<UTexture2D>(AnyObj);
}

static UTexture2D* LoadCap01IconTexture()
{
    const TCHAR* IconPaths[] = {
        // стандартная форма
        TEXT("/Game/BackToZaraysk/Core/Items/Equipment/cap01.cap01"),
        TEXT("/Game/BackToZaraysk/Core/Items/Equipment/cap01.Cap01"),
        // иногда помогает форма без ".AssetName"
        TEXT("/Game/BackToZaraysk/Core/Items/Equipment/cap01"),
        // форма с префиксом класса
        TEXT("Texture2D'/Game/BackToZaraysk/Core/Items/Equipment/cap01.cap01'"),
        TEXT("Texture2D'/Game/BackToZaraysk/Core/Items/Equipment/cap01.Cap01'")
    };

    for (const TCHAR* P : IconPaths)
    {
        if (UTexture2D* Tex = LoadTexture2DWithRedirect(P))
        {
            return Tex;
        }
    }

    return nullptr;
}

UEquippableItemData::UEquippableItemData()
{
    // Значения по умолчанию уже установлены в заголовочном файле
    // ItemType наследуется от UInventoryItemData
    // EquipmentSlot = None по умолчанию
    PickupActorClass = nullptr;
}

UHelmetBaseItemData::UHelmetBaseItemData()
{
    DisplayName = FText::FromString(TEXT("Шлем"));
    SizeInCellsX = 1;
    SizeInCellsY = 1;
    EquipmentSlot = Helmet;
    bRotatable = false;

    bHasAdditionalStorage = false;
    AdditionalGridSize = FIntPoint(0, 0);

    // Визуал — небольшой параллелепипед (куб с масштабом), используем стандартный куб Engine
    const FString MeshPath = TEXT("/Engine/BasicShapes/Cube.Cube");
    EquippedMesh = LoadObject<UStaticMesh>(nullptr, *MeshPath);
    if (!EquippedMesh || !EquippedMesh->IsValidLowLevel())
    {
        UE_LOG(LogTemp, Error, TEXT("UHelmetBaseItemData: Failed to load EquippedMesh from path: %s"), *MeshPath);
        EquippedMesh = nullptr;
    }

    // Аттачим к head (bone name тоже ок для DoesSocketExist)
    AttachSocketName = FName(TEXT("head"));

    // Иконка (как у брони — test)
    {
        const FString IconPath = TEXT("/Game/BackToZaraysk/Core/Items/Equipment/test.test");
        Icon = LoadObject<UTexture2D>(nullptr, *IconPath);
        if (!Icon || !Icon->IsValidLowLevel())
        {
            UE_LOG(LogTemp, Warning, TEXT("UHelmetBaseItemData: Failed to load Icon from path: %s"), *IconPath);
            Icon = nullptr;
        }
    }

    // Класс pickup-актора по умолчанию — AHelmetBase (может быть переопределён в наследниках DataAsset)
    PickupActorClass = AHelmetBase::StaticClass();

    // Позиционирование на голове. Масштаб экипированного предмета = 1x1x1
    RelativeTransform = FTransform(
        FRotator(270.0f, 0.0f, 0.0f), // +180° + 90° вокруг оси Y (Pitch)
        FVector(0.0f, 0.0f, 6.0f),
        FVector(1.0f, 1.0f, 1.0f)
    );
}

UCap01ItemData::UCap01ItemData()
{
    DisplayName = FText::FromString(TEXT("Кепка"));
    // Размер по умолчанию 1x1, но оставляем редактируемым через DataAsset
    SizeInCellsX = 1;
    SizeInCellsY = 1;
    EquipmentSlot = Helmet;
    bRotatable = false;

    bHasAdditionalStorage = false;
    AdditionalGridSize = FIntPoint(0, 0);

    // Модель кепки
    // Файл: Content/BackToZaraysk/Core/Items/Equipment/SM_Cap_Bege.uasset
    const FString MeshPath = TEXT("/Game/BackToZaraysk/Core/Items/Equipment/SM_Cap_Bege.SM_Cap_Bege");
    EquippedMesh = LoadObject<UStaticMesh>(nullptr, *MeshPath);
    if (!EquippedMesh || !EquippedMesh->IsValidLowLevel())
    {
        UE_LOG(LogTemp, Error, TEXT("UCap01ItemData: Failed to load EquippedMesh from path: %s"), *MeshPath);
        EquippedMesh = nullptr;
    }

    AttachSocketName = FName(TEXT("head"));

    // Иконка (можно заменить позже)
    {
        // Иконка для BP_Cap01 из ассета Content/BackToZaraysk/Core/Items/Equipment/cap01.uasset
        Icon = LoadCap01IconTexture();

        if (!Icon || !Icon->IsValidLowLevel())
        {
            UE_LOG(LogTemp, Warning, TEXT("UCap01ItemData: Failed to load Icon from cap01"));
            Icon = nullptr;
        }
    }

    // Трансформ на голове — дефолтный, можно подкрутить в редакторе на DataAsset
    RelativeTransform = FTransform(
        FRotator(0.0f, 0.0f, 0.0f),
        FVector(0.0f, 0.0f, 0.0f),
        FVector(1.0f, 1.0f, 1.0f)
    );
}

void UCap01ItemData::PostLoad()
{
    Super::PostLoad();

    const bool bIconIsMissing = (Icon == nullptr) || !Icon->IsValidLowLevel();
    const bool bIconIsOldTest = (!bIconIsMissing && Icon && Icon->GetPathName().Contains(TEXT("/test.")));

    if (bIconIsMissing || bIconIsOldTest)
    {
        if (UTexture2D* NewIcon = LoadCap01IconTexture())
        {
            Icon = NewIcon;
        }
    }
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
    // Мульти-гриды (6 мини‑гридов) слева направо: 1x1,1x1,1x2,1x2,1x1,1x1
    AdditionalGrids = {
        FIntPoint(1, 1),
        FIntPoint(1, 1),
        FIntPoint(1, 2),
        FIntPoint(1, 2),
        FIntPoint(1, 1),
        FIntPoint(1, 1)
    };
    // Оффсеты по умолчанию: в ряд слева направо с зазором 10px (~0.17 клетки). UI всё равно округляет до клеток,
    // поэтому здесь задаём целочисленные клеточные оффсеты без зазоров.
    AdditionalGridOffsets = {
        FIntPoint(0, 0),
        FIntPoint(1, 0),
        FIntPoint(2, 0),
        FIntPoint(3, 0),
        FIntPoint(4, 0),
        FIntPoint(5, 0),
    };
    
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
    
    // Класс pickup-актора по умолчанию — ATacticalVest
    PickupActorClass = ATacticalVest::StaticClass();

    // Устанавливаем относительный трансформ для тактического жилета
    // Поворачиваем жилет вертикально и на 90 градусов
    RelativeTransform = FTransform(
        FRotator(0.0f, 90.0f, 90.0f),   // Поворот: вертикально (90°) + поворот на 90°
        FVector(5.0f, 0.0f, 0.0f),      // Небольшое смещение вперед
        FVector(1.0f, 1.0f, 1.0f)       // Масштаб
    );
}

UVestBaseItemData::UVestBaseItemData()
{
    DisplayName = FText::FromString(TEXT("Разгрузка"));
    // Размер предмета и конфигурация хранилища переопределяются из AVestBase (в редакторе).
    SizeInCellsX = 1;
    SizeInCellsY = 1;
    EquipmentSlot = Vest;
    bRotatable = false;

    bHasAdditionalStorage = true;
    AdditionalGridSize = FIntPoint(1, 1);
    AdditionalGrids = { FIntPoint(1, 1) };
    AdditionalGridOffsets = { FIntPoint(0, 0) };

    // Дефолтный SkeletalMesh (как просили)
    const FString MeshPath = TEXT("/Game/BackToZaraysk/Core/Items/Equipment/SK_BackpackSmall.SK_BackpackSmall");
    EquippedMesh = LoadObject<USkeletalMesh>(nullptr, *MeshPath);
    if (!EquippedMesh || !EquippedMesh->IsValidLowLevel())
    {
        UE_LOG(LogTemp, Warning, TEXT("UVestBaseItemData: Failed to load EquippedMesh from path: %s"), *MeshPath);
        EquippedMesh = nullptr;
    }

    // Аттач по умолчанию — spine_03 (можно изменить в AVestBase)
    AttachSocketName = FName(TEXT("spine_03"));

    // Класс pickup-актора по умолчанию — AVestBase (может быть переопределён в наследниках DataAsset)
    PickupActorClass = AVestBase::StaticClass();

    Icon = nullptr;

    RelativeTransform = FTransform(
        FRotator(0.0f, 0.0f, 0.0f),
        FVector(0.0f, 0.0f, 0.0f),
        FVector(1.0f, 1.0f, 1.0f)
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
    
    // Класс pickup-актора по умолчанию — APickupBackpack (может быть переопределён в наследниках DataAsset)
    PickupActorClass = APickupBackpack::StaticClass();

    // Относительный трансформ — визуально как рюкзак на спине
    RelativeTransform = FTransform(
        FRotator(0.0f, 0.0f, 0.0f),      // без поворота
        FVector(-12.0f, 0.0f, -4.0f),    // слегка назад и вниз
        FVector(0.28f, 0.16f, 0.36f)     // похожие пропорции на рюкзак
    );
}

UBackpackBaseItemData::UBackpackBaseItemData()
{
    DisplayName = FText::FromString(TEXT("Рюкзак"));
    // Размер предмета и размер хранилища будут переопределены из ABackpackBase (в редакторе).
    SizeInCellsX = 1;
    SizeInCellsY = 1;
    EquipmentSlot = Backpack;
    bRotatable = false;

    bHasAdditionalStorage = true;
    AdditionalGridSize = FIntPoint(1, 1);

    // Визуал по умолчанию — SK_BackpackSmall
    // Файл: Content/BackToZaraysk/Core/Items/Equipment/SK_BackpackSmall.uasset
    const FString MeshPath = TEXT("/Game/BackToZaraysk/Core/Items/Equipment/SK_BackpackSmall.SK_BackpackSmall");
    EquippedMesh = LoadObject<USkeletalMesh>(nullptr, *MeshPath);
    if (!EquippedMesh || !EquippedMesh->IsValidLowLevel())
    {
        UE_LOG(LogTemp, Warning, TEXT("UBackpackBaseItemData: Failed to load EquippedMesh from path: %s"), *MeshPath);
        EquippedMesh = nullptr;
    }

    // Аттачим к грудному отделу спины
    AttachSocketName = FName(TEXT("spine_03"));

    // Класс pickup-актора по умолчанию — ABackpackBase (для настраиваемых рюкзаков)
    PickupActorClass = ABackpackBase::StaticClass();

    // Иконку по умолчанию не задаём: если не выбрана в BP — UI покажет белую заливку.
    Icon = nullptr;

    RelativeTransform = FTransform(
        FRotator(0.0f, 0.0f, 0.0f),
        FVector(0.0f, 0.0f, 0.0f),
        FVector(1.0f, 1.0f, 1.0f)
    );
}

UArmorBaseItemData::UArmorBaseItemData()
{
    DisplayName = FText::FromString(TEXT("Бронежилет"));
    SizeInCellsX = 3;
    SizeInCellsY = 3;
    EquipmentSlot = Armor;
    bRotatable = false;

    // Бронежилет НЕ имеет дополнительных гридов для хранения
    bHasAdditionalStorage = false;
    AdditionalGridSize = FIntPoint(0, 0);

    // Для теста используем статический куб из Engine (видимый на персонаже как плита)
    FString MeshPath = TEXT("/Engine/BasicShapes/Cube.Cube");
    EquippedMesh = LoadObject<UStaticMesh>(nullptr, *MeshPath);
    if (!EquippedMesh || !EquippedMesh->IsValidLowLevel())
    {
        UE_LOG(LogTemp, Error, TEXT("UArmorBaseItemData: Failed to load EquippedMesh from path: %s"), *MeshPath);
        EquippedMesh = nullptr;
    }

    // Экипировка: аттачим к кости груди (spine_02), как у VestBase
    AttachSocketName = FName(TEXT("spine_02"));

    // Иконка (вместо белого квадрата) — из файла test
    // Файл: Content/BackToZaraysk/Core/Items/Equipment/test.uasset (и/или test.tga)
    {
        const FString IconPath = TEXT("/Game/BackToZaraysk/Core/Items/Equipment/test.test");
        Icon = LoadObject<UTexture2D>(nullptr, *IconPath);
        if (!Icon || !Icon->IsValidLowLevel())
        {
            UE_LOG(LogTemp, Warning, TEXT("UArmorBaseItemData: Failed to load Icon from path: %s"), *IconPath);
            Icon = nullptr;
        }
    }

    // Класс pickup-актора по умолчанию — AArmorBase (может быть переопределён в наследниках DataAsset и/или BP)
    PickupActorClass = AArmorBase::StaticClass();

    // Позиционирование и масштаб 1x1x1 при экипировке (смещение под сокет spine_02)
    RelativeTransform = FTransform(
        FRotator(0.0f, 0.0f, 0.0f),
        FVector(-2.0f, -4.0f, 4.0f),
        FVector(1.0f, 1.0f, 1.0f)
    );
}

UBulletproofVestBegeItemData::UBulletproofVestBegeItemData()
{
    // Все свойства идентичны ArmorBase, кроме модели
    DisplayName = FText::FromString(TEXT("Бронежилет"));
    SizeInCellsX = 3;
    SizeInCellsY = 3;
    EquipmentSlot = Armor;
    bRotatable = false;

    bHasAdditionalStorage = false;
    AdditionalGridSize = FIntPoint(0, 0);

    // Класс pickup-актора по умолчанию — ABulletproofVestBege
    PickupActorClass = ABulletproofVestBege::StaticClass();

    // Модель бронежилета
    // Файл: Content/BackToZaraysk/Core/Items/Equipment/SK_Bulletproof_Bege.uasset
    const FString MeshPath = TEXT("/Game/BackToZaraysk/Core/Items/Equipment/SK_Bulletproof_Bege.SK_Bulletproof_Bege");
    EquippedMesh = LoadObject<USkeletalMesh>(nullptr, *MeshPath);
    if (!EquippedMesh || !EquippedMesh->IsValidLowLevel())
    {
        UE_LOG(LogTemp, Error, TEXT("UBulletproofVestBegeItemData: Failed to load EquippedMesh from path: %s"), *MeshPath);
        EquippedMesh = nullptr;
    }

    // Экипировка: аттачим к кости груди (spine_02)
    AttachSocketName = FName(TEXT("spine_02"));

    // Иконка — из файла test (как просили)
    {
        const FString IconPath = TEXT("/Game/BackToZaraysk/Core/Items/Equipment/test.test");
        Icon = LoadObject<UTexture2D>(nullptr, *IconPath);
        if (!Icon || !Icon->IsValidLowLevel())
        {
            UE_LOG(LogTemp, Warning, TEXT("UBulletproofVestBegeItemData: Failed to load Icon from path: %s"), *IconPath);
            Icon = nullptr;
        }
    }

    RelativeTransform = FTransform(
        FRotator(0.0f, 0.0f, 0.0f),
        FVector(-2.0f, -4.0f, 4.0f),
        FVector(1.0f, 1.0f, 1.0f)
    );
}

// --- Установленные модули на броне ---

void UEquippableItemData::GetOccupiedCellsForSide(int32 SideIndex, TSet<FIntPoint>& OutOccupied) const
{
    OutOccupied.Reset();
    for (const FInstalledArmorMod& Inst : InstalledMods)
    {
        if (Inst.SideIndex != SideIndex || !Inst.ModItemData) continue;
        FIntPoint Foot = GetModFootprint(Inst.ModItemData);
        for (int32 dy = 0; dy < Foot.Y; ++dy)
            for (int32 dx = 0; dx < Foot.X; ++dx)
                OutOccupied.Add(FIntPoint(Inst.CellX + dx, Inst.CellY + dy));
    }
}

bool UEquippableItemData::CanInstallModAt(int32 SideIndex, int32 CellX, int32 CellY, FIntPoint Footprint) const
{
    if (SideIndex < 0 || SideIndex >= ModSideGrids.Num()) return false;
    const FArmorModSideGrid& G = ModSideGrids[SideIndex];
    if (CellX < 0 || CellY < 0 || CellX + Footprint.X > G.CellsX || CellY + Footprint.Y > G.CellsY) return false;
    TSet<FIntPoint> Occupied;
    GetOccupiedCellsForSide(SideIndex, Occupied);
    for (int32 dy = 0; dy < Footprint.Y; ++dy)
        for (int32 dx = 0; dx < Footprint.X; ++dx)
            if (Occupied.Contains(FIntPoint(CellX + dx, CellY + dy))) return false;
    return true;
}

FIntPoint UEquippableItemData::GetModFootprint(UInventoryItemData* ModItemData)
{
    if (!ModItemData) return FIntPoint(1, 1);
    if (UEquipModBaseItemData* Mod = Cast<UEquipModBaseItemData>(ModItemData))
        return FIntPoint(FMath::Max(1, Mod->ModSlotSize.X), FMath::Max(1, Mod->ModSlotSize.Y));
    return FIntPoint(1, 1);
}

void UEquippableItemData::GetModStorageGrids(TArray<FIntPoint>& OutGridSizes, TArray<UInventoryItemData*>& OutModItemData) const
{
    OutGridSizes.Reset();
    OutModItemData.Reset();
    for (const FInstalledArmorMod& Inst : InstalledMods)
    {
        if (!Inst.ModItemData) continue;
        UEquipModBaseItemData* Mod = Cast<UEquipModBaseItemData>(Inst.ModItemData);
        if (!Mod || !Mod->bHasAdditionalStorage) continue;

		// Для модов с несколькими гридами берём их "объединённый" размер, но с разумным ограничением,
		// чтобы избежать гигантских UI-гридов и долгих переборов.
		const int32 MaxCellsPerDim = 8;
		FIntPoint Size = Mod->AdditionalGridSize;
		if (Size.X <= 0 || Size.Y <= 0)
		{
			// Если по каким-то причинам BoundingBox не посчитан — считаем его заново по AdditionalGrids/Offsets.
			if (Mod->AdditionalGrids.Num() > 0)
			{
				int32 MaxCol = 0;
				int32 MaxRow = 0;
				for (int32 i = 0; i < Mod->AdditionalGrids.Num(); ++i)
				{
					const FIntPoint Sz = Mod->AdditionalGrids[i];
					const FIntPoint Off = Mod->AdditionalGridOffsets.IsValidIndex(i) ? Mod->AdditionalGridOffsets[i] : FIntPoint(i, 0);
					MaxCol = FMath::Max(MaxCol, Off.X + FMath::Max(1, Sz.X));
					MaxRow = FMath::Max(MaxRow, Off.Y + FMath::Max(1, Sz.Y));
				}
				Size = FIntPoint(FMath::Max(1, MaxCol), FMath::Max(1, MaxRow));
			}
			else
			{
				Size = FIntPoint(1, 1);
			}
		}

		Size.X = FMath::Clamp(Size.X, 1, MaxCellsPerDim);
		Size.Y = FMath::Clamp(Size.Y, 1, MaxCellsPerDim);

        OutGridSizes.Add(Size);
        OutModItemData.Add(Inst.ModItemData);
    }
}

bool UEquippableItemData::InstallMod(UInventoryItemData* ModItemData, int32 SideIndex, int32 CellX, int32 CellY)
{
    if (!ModItemData || !bIsModdable) return false;
    FIntPoint Foot = GetModFootprint(ModItemData);
    if (!CanInstallModAt(SideIndex, CellX, CellY, Foot)) return false;
    FInstalledArmorMod Inst;
    Inst.ModItemData = ModItemData;
    Inst.SideIndex = SideIndex;
    Inst.CellX = CellX;
    Inst.CellY = CellY;
    InstalledMods.Add(Inst);
    return true;
}

UInventoryItemData* UEquippableItemData::UninstallMod(int32 InstalledModIndex)
{
    if (InstalledModIndex < 0 || InstalledModIndex >= InstalledMods.Num()) return nullptr;
    UInventoryItemData* Removed = InstalledMods[InstalledModIndex].ModItemData;
    InstalledMods.RemoveAt(InstalledModIndex);
    return Removed;
}

int32 UEquippableItemData::GetInstalledModIndexAt(int32 SideIndex, int32 CellX, int32 CellY) const
{
    for (int32 i = 0; i < InstalledMods.Num(); ++i)
    {
        const FInstalledArmorMod& Inst = InstalledMods[i];
        if (Inst.SideIndex != SideIndex || !Inst.ModItemData) continue;
        FIntPoint Foot = GetModFootprint(Inst.ModItemData);
        if (CellX >= Inst.CellX && CellX < Inst.CellX + Foot.X && CellY >= Inst.CellY && CellY < Inst.CellY + Foot.Y)
            return i;
    }
    return -1;
}




