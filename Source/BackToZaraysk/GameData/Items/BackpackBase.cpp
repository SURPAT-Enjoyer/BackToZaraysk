#include "BackpackBase.h"
#include "Components/StaticMeshComponent.h"
#include "BackToZaraysk/Inventory/EquippableItemData.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "Engine/SkeletalMesh.h"

ABackpackBase::ABackpackBase()
{
	EnsureStorageGrids();

	// World pickup: по умолчанию показываем SkeletalMesh рюкзака, чтобы BP-наследникам было проще
	if (Mesh)
	{
		if (!Mesh->GetStaticMesh())
		{
			if (UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
			{
				Mesh->SetStaticMesh(Cube);
			}
		}
		// Если у нас есть SkeletalMesh — статический меш прячем (чтобы не было двух визуалов)
		Mesh->SetVisibility(false, true);
		Mesh->SetHiddenInGame(true, true);
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Mesh->SetWorldScale3D(WorldVisualScale);
	}

	if (SkeletalMesh)
	{
		// Дефолтный меш для BackpackBase: SK_BackpackSmall
		// Файл: Content/BackToZaraysk/Core/Items/Equipment/SK_BackpackSmall.uasset
		if (USkeletalMesh* Sk = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/BackToZaraysk/Core/Items/Equipment/SK_BackpackSmall.SK_BackpackSmall")))
		{
			SkeletalMesh->SetSkeletalMesh(Sk);
		}
		SkeletalMesh->SetVisibility(true, true);
		SkeletalMesh->SetHiddenInGame(false, true);
		SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		SkeletalMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
		SkeletalMesh->SetUseCCD(true);
		SkeletalMesh->SetSimulatePhysics(bEnablePhysics);
	}

	// Базовые данные предмета
	ItemClass = UBackpackBaseItemData::StaticClass();

	// Аттачим к грудному отделу спины
	AttachSocketName = AttachBoneName;
	AttachScale = FVector(1.0f, 1.0f, 1.0f); // по умолчанию 1х1х1
}

void ABackpackBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	EnsureStorageGrids();

	if (Mesh)
	{
		Mesh->SetWorldScale3D(WorldVisualScale);
	}

	ApplyEditorOverridesToItemInstance();
}

void ABackpackBase::BeginPlay()
{
	Super::BeginPlay();

	EnsureStorageGrids();

	// Если это pickup, размещённый в уровне: создаём ItemInstance и применяем параметры из Details
	bool bCreatedItemInstanceHere = false;
	if (!ItemInstance && ItemClass)
	{
		if (UInventoryItemData* NewData = NewObject<UInventoryItemData>(this, ItemClass))
		{
			ItemInstance = NewData;
			bCreatedItemInstanceHere = true;
		}
	}

	ApplyEditorOverridesToItemInstance();

	// Фикс для BP-наследников (как у HelmetBase):
	// если в BP на компоненте Mesh выставлен конкретный StaticMesh — подхватываем его в ItemData,
	// чтобы ApplyItemInstanceVisuals не перетёр меш на куб из ItemData.
	if (UEquippableItemData* Eq = Cast<UEquippableItemData>(ItemInstance))
	{
		// Если мы создали ItemInstance на этом акторе (то есть pickup размещён в уровне),
		// и в BP выбрали SkeletalMesh — подхватим его в ItemData, чтобы при выбросе в мир сохранялся визуал.
		// ВАЖНО: если ItemInstance пришёл извне (дроп из инвентаря), НЕ перетираем его EquippedMesh.
		if (bCreatedItemInstanceHere && SkeletalMesh && SkeletalMesh->GetSkeletalMeshAsset())
		{
			Eq->EquippedMesh = SkeletalMesh->GetSkeletalMeshAsset();
		}

		if (Mesh && Mesh->GetStaticMesh())
		{
			UStaticMesh* CurrentMesh = Mesh->GetStaticMesh();
			UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
			const bool bIsCube = (Cube && CurrentMesh == Cube) || CurrentMesh->GetPathName().Contains(TEXT("/Engine/BasicShapes/Cube"));
			if (!bIsCube)
			{
				Eq->EquippedMesh = CurrentMesh;
			}
		}
	}

	if (ItemInstance)
	{
		ApplyItemInstanceVisuals();
	}
}

 
#if WITH_EDITOR
void ABackpackBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	EnsureStorageGrids();
	ApplyEditorOverridesToItemInstance();
}
#endif

void ABackpackBase::EnsureStorageGrids()
{
	StorageGridCount = FMath::Max(1, StorageGridCount);

	// Если пользователь не настроил новый формат — подхватим legacy X/Y в один грид.
	if (StorageGridSizes.Num() == 0 && StorageGridOffsets.Num() == 0)
	{
		StorageGridSizes = { FIntPoint(FMath::Max(1, StorageGridSizeX), FMath::Max(1, StorageGridSizeY)) };
		StorageGridOffsets = { FIntPoint(0, 0) };
		StorageGridCount = 1;
	}

	if (StorageGridSizes.Num() != StorageGridCount)
	{
		StorageGridSizes.SetNum(StorageGridCount);
	}
	if (StorageGridOffsets.Num() != StorageGridCount)
	{
		StorageGridOffsets.SetNum(StorageGridCount);
	}

	for (FIntPoint& P : StorageGridSizes)
	{
		P.X = FMath::Max(1, P.X);
		P.Y = FMath::Max(1, P.Y);
	}
	// Смещения мини‑гридов теперь могут быть отрицательными (для сдвига
	// отдельных ячеек ниже/выше базовой строки), поэтому не клэмпим их к 0.

	// Синхронизируем legacy поля из первого грида, чтобы старые части кода продолжали работать.
	if (StorageGridSizes.Num() > 0)
	{
		StorageGridSizeX = FMath::Max(1, StorageGridSizes[0].X);
		StorageGridSizeY = FMath::Max(1, StorageGridSizes[0].Y);
	}
}

void ABackpackBase::ApplyEditorOverridesToItemInstance()
{
	if (!ItemInstance) return;

	// 1) Размер предмета в инвентаре
	ItemInstance->SizeInCellsX = FMath::Max(1, InventorySizeX);
	ItemInstance->SizeInCellsY = FMath::Max(1, InventorySizeY);

	// 2) Настройки хранилища (для рюкзака всегда дополнительное хранилище)
	if (UEquippableItemData* Eq = Cast<UEquippableItemData>(ItemInstance))
	{
		Eq->EquipmentSlot = Backpack;
		// Модификации: общий флаг и сетки, как у ArmorBase.
		Eq->bIsModdable = bIsModdable;
		Eq->ModSideGrids = ModSideGrids;

		Eq->bHasAdditionalStorage = true;
		Eq->AdditionalGrids = StorageGridSizes;
		Eq->AdditionalGridOffsets = StorageGridOffsets;
		// Для совместимости оставляем AdditionalGridSize как bounding box (max extents)
		int32 MaxX = 0;
		int32 MaxY = 0;
		for (int32 i = 0; i < Eq->AdditionalGrids.Num(); ++i)
		{
			const FIntPoint Sz = Eq->AdditionalGrids[i];
			const FIntPoint Off = Eq->AdditionalGridOffsets.IsValidIndex(i) ? Eq->AdditionalGridOffsets[i] : FIntPoint(0, 0);
			MaxX = FMath::Max(MaxX, FMath::Max(1, Off.X) + FMath::Max(1, Sz.X));
			MaxY = FMath::Max(MaxY, FMath::Max(0, Off.Y) + FMath::Max(1, Sz.Y));
		}
		Eq->AdditionalGridSize = FIntPoint(FMath::Max(1, MaxX), FMath::Max(1, MaxY));

		// Единая точка правды для сокета аттача:
		// 1) Если указан Backpack|Attachment::AttachBoneName — используем его.
		// 2) Иначе, если в Equipment|Attachment::AttachSocketName выбран сокет (не "None") — берём его.
		// 3) Иначе fallback на spine_03.
		FName EffectiveSocket = AttachBoneName;
		if (EffectiveSocket.IsNone())
		{
			if (!AttachSocketName.IsNone() && AttachSocketName != TEXT("None"))
			{
				EffectiveSocket = AttachSocketName;
			}
			else
			{
				EffectiveSocket = FName(TEXT("spine_03"));
			}
		}

		Eq->AttachSocketName = EffectiveSocket;

		// Синхронизируем оба поля на акторе, чтобы в Details не было расхождений.
		AttachBoneName = EffectiveSocket;
		AttachSocketName = EffectiveSocket;
	}

	ApplyInventoryIconOverride();
}

void ABackpackBase::ApplyInventoryIconOverride()
{
	if (!ItemInstance) return;

	if (InventoryIconTexture && InventoryIconTexture->IsValidLowLevel())
	{
		ItemInstance->Icon = InventoryIconTexture;
	}
	else
	{
		// ВАЖНО: оставляем nullptr, чтобы UI использовал WhiteSquareTexture нужного размера.
		ItemInstance->Icon = nullptr;
	}
}

