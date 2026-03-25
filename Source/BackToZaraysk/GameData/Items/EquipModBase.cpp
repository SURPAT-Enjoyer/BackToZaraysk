#include "EquipModBase.h"

#include "BackToZaraysk/Inventory/InventoryItemData.h"

AEquipModBase::AEquipModBase()
{
	// ВАЖНО: "поля по умолчанию" от ItemBase не меняем.
	// Здесь только фиксируем тип данных предмета.
	ItemClass = UEquipModBaseItemData::StaticClass();

	EnsureStorageGrids();
}

void AEquipModBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	EnsureStorageGrids();

	if (ItemInstance)
	{
		ApplyEditorOverridesToItemInstance();
	}
}

void AEquipModBase::BeginPlay()
{
	Super::BeginPlay();

	// Для pickup, размещённого в мире, НАДЁЖНО гарантируем, что ItemInstance имеет тип UEquipModBaseItemData.
	// Так AdditionalGrids/Offsets и флаг bHasAdditionalStorage корректно попадают в данные предмета,
	// и мод в инвентаре не "превращается" в обычный ItemBase 1x1 без хранилища.
	if (!ItemInstance)
	{
		if (UEquipModBaseItemData* NewData = NewObject<UEquipModBaseItemData>(this, UEquipModBaseItemData::StaticClass()))
		{
			ItemInstance = NewData;
		}
	}
	else if (!Cast<UEquipModBaseItemData>(ItemInstance))
	{
		// Если по какой‑то причине в блюпринте выставлен другой класс данных —
		// создаём корректный экземпляр UEquipModBaseItemData и копируем базовые поля.
		if (UEquipModBaseItemData* NewData = NewObject<UEquipModBaseItemData>(this, UEquipModBaseItemData::StaticClass()))
		{
			if (UInventoryItemData* Old = Cast<UInventoryItemData>(ItemInstance))
			{
				NewData->DisplayName   = Old->DisplayName;
				NewData->Icon          = Old->Icon;
				NewData->WeightKg      = Old->WeightKg;
				NewData->SizeInCellsX  = Old->SizeInCellsX;
				NewData->SizeInCellsY  = Old->SizeInCellsY;
				NewData->bRotatable    = Old->bRotatable;
			}
			ItemInstance = NewData;
		}
	}

	if (ItemInstance)
	{
		ApplyEditorOverridesToItemInstance();
	}
}

#if WITH_EDITOR
void AEquipModBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	EnsureStorageGrids();

	if (ItemInstance)
	{
		ApplyEditorOverridesToItemInstance();
	}
}
#endif

void AEquipModBase::SyncAttachmentGridOriginToItemInstance()
{
	if (UEquipModBaseItemData* Mod = Cast<UEquipModBaseItemData>(ItemInstance))
		Mod->AttachmentGridOrigin = AttachmentGridOrigin;
}

void AEquipModBase::EnsureStorageGrids()
{
	StorageGridCount = FMath::Max(1, StorageGridCount);
	if (StorageGridSizes.Num() != StorageGridCount)
	{
		StorageGridSizes.SetNum(StorageGridCount);
	}
	if (StorageGridOffsets.Num() != StorageGridCount)
	{
		StorageGridOffsets.SetNum(StorageGridCount);
	}

	for (int32 i = 0; i < StorageGridCount; ++i)
	{
		FIntPoint& Sz = StorageGridSizes[i];
		if (Sz.X <= 0) Sz.X = 1;
		if (Sz.Y <= 0) Sz.Y = 1;

		// Offsets могут быть любыми (в т.ч. отрицательными) — но чтобы табличная раскладка была предсказуемой,
		// держим их как индексы столбца/строки. Не клэмпим.
	}
}

void AEquipModBase::ApplyEditorOverridesToItemInstance()
{
	if (!ItemInstance) return;

	UEquipModBaseItemData* Mod = Cast<UEquipModBaseItemData>(ItemInstance);
	if (!Mod) return;

	Mod->bHasAdditionalStorage = bIsHasGrid;
	Mod->ModSlotSize = FIntPoint(FMath::Max(1, ModSlotSize.X), FMath::Max(1, ModSlotSize.Y));
	Mod->AttachmentGridOrigin = AttachmentGridOrigin;

	if (bIsHasGrid)
	{
		// === Источник правды — настройки грида у актора (StorageGridSizes/StorageGridOffsets). ===
		// Они транслируются в ItemData, чтобы:
		// 1) окно хранилища мода,
		// 2) мини‑гриды мода на бронежилете
		// выглядели идентично.
		Mod->AdditionalGrids = StorageGridSizes;
		Mod->AdditionalGridOffsets = StorageGridOffsets;

		// Если размеров нет вообще — дефолт 1x1, чтобы окно не разваливалось.
		if (Mod->AdditionalGrids.Num() == 0)
		{
			Mod->AdditionalGrids.Add(FIntPoint(1, 1));
		}

		// Если оффсеты не заданы или длина не совпадает —
		// раскладываем мини‑гриды в линию слева направо по ширине (как в CreateVestGrid/UpdateBackpackStorageGrid).
		// Если же в StorageGridOffsets ты задал осмысленные значения (как на скрине 3), мы их НЕ трогаем.
		if (Mod->AdditionalGridOffsets.Num() != Mod->AdditionalGrids.Num())
		{
			Mod->AdditionalGridOffsets.SetNum(Mod->AdditionalGrids.Num());
			float XCells = 0.f;
			for (int32 i = 0; i < Mod->AdditionalGrids.Num(); ++i)
			{
				const int32 W = FMath::Max(1, Mod->AdditionalGrids[i].X);
				Mod->AdditionalGridOffsets[i] = FIntPoint(FMath::RoundToInt(XCells), 0);
				XCells += (float)W;
			}
		}

		// Bounding box: максимальные экстенты по Col/Row (как у рюкзака)
		int32 MaxCol = 0;
		int32 MaxRow = 0;
		for (int32 i = 0; i < Mod->AdditionalGrids.Num(); ++i)
		{
			const FIntPoint Sz(FMath::Max(1, Mod->AdditionalGrids[i].X), FMath::Max(1, Mod->AdditionalGrids[i].Y));
			const FIntPoint Off = Mod->AdditionalGridOffsets.IsValidIndex(i) ? Mod->AdditionalGridOffsets[i] : FIntPoint(i, 0);
			MaxCol = FMath::Max(MaxCol, Off.X + Sz.X);
			MaxRow = FMath::Max(MaxRow, Off.Y + Sz.Y);
		}
		Mod->AdditionalGridSize = FIntPoint(FMath::Max(1, MaxCol), FMath::Max(1, MaxRow));
	}
	else
	{
		// Если грида нет — оставляем конфиг, но UI его не показывает.
		Mod->AdditionalGridSize = FIntPoint(0, 0);
	}
}

