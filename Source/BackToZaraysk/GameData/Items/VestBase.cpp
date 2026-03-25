#include "VestBase.h"
#include "Components/StaticMeshComponent.h"
#include "BackToZaraysk/Inventory/EquippableItemData.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "Engine/SkeletalMesh.h"

AVestBase::AVestBase()
{
	EnsureStorageGridSizes();

	// World pickup: по умолчанию показываем SkeletalMesh (как просили — SK_BackpackSmall)
	if (Mesh)
	{
		if (!Mesh->GetStaticMesh())
		{
			if (UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
			{
				Mesh->SetStaticMesh(Cube);
			}
		}
		Mesh->SetVisibility(false, true);
		Mesh->SetHiddenInGame(true, true);
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Mesh->SetWorldScale3D(WorldVisualScale);
	}

	if (SkeletalMesh)
	{
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

	ItemClass = UVestBaseItemData::StaticClass();
	AttachSocketName = AttachBoneName;
	AttachScale = FVector(1.0f, 1.0f, 1.0f);
}

void AVestBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	EnsureStorageGridSizes();

	if (Mesh)
	{
		Mesh->SetWorldScale3D(WorldVisualScale);
	}

	ApplyEditorOverridesToItemInstance();
}

void AVestBase::BeginPlay()
{
	Super::BeginPlay();

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

	// Если это placed pickup (создали ItemInstance тут), подхватываем SkeletalMesh из BP в ItemData,
	// чтобы при выбросе в мир сохранялся визуал из SkeletalMesh.
	if (UEquippableItemData* Eq = Cast<UEquippableItemData>(ItemInstance))
	{
		if (bCreatedItemInstanceHere && SkeletalMesh && SkeletalMesh->GetSkeletalMeshAsset())
		{
			Eq->EquippedMesh = SkeletalMesh->GetSkeletalMeshAsset();
		}
	}

	if (ItemInstance)
	{
		ApplyItemInstanceVisuals();
	}
}

#if WITH_EDITOR
void AVestBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	EnsureStorageGridSizes();
	ApplyEditorOverridesToItemInstance();
}
#endif

void AVestBase::EnsureStorageGridSizes()
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
	for (FIntPoint& P : StorageGridSizes)
	{
		P.X = FMath::Max(1, P.X);
		P.Y = FMath::Max(1, P.Y);
	}
	// Смещения мини‑гридов теперь могут быть отрицательными (для сдвига
	// отдельных ячеек ниже/выше базовой строки), поэтому не клэмпим их к 0.
}

void AVestBase::ApplyEditorOverridesToItemInstance()
{
	if (!ItemInstance) return;

	ItemInstance->SizeInCellsX = FMath::Max(1, InventorySizeX);
	ItemInstance->SizeInCellsY = FMath::Max(1, InventorySizeY);

	if (UEquippableItemData* Eq = Cast<UEquippableItemData>(ItemInstance))
	{
		Eq->EquipmentSlot = Vest;
		// Модификации: такой же флаг и сетки, как у ArmorBase, но теперь на общем базовом классе.
		Eq->bIsModdable = bIsModdable;
		Eq->ModSideGrids = ModSideGrids;

		Eq->bHasAdditionalStorage = true;
		Eq->AdditionalGrids = StorageGridSizes;
		Eq->AdditionalGridOffsets = StorageGridOffsets;
		// Для совместимости оставляем AdditionalGridSize как (sum widths, max height)
		int32 SumW = 0;
		int32 MaxH = 0;
		for (const FIntPoint& G : Eq->AdditionalGrids)
		{
			SumW += FMath::Max(1, G.X);
			MaxH = FMath::Max(MaxH, FMath::Max(1, G.Y));
		}
		Eq->AdditionalGridSize = FIntPoint(FMath::Max(1, SumW), FMath::Max(1, MaxH));

		// Единая точка правды для сокета аттача:
		// 1) Если указан Vest|Attachment::AttachBoneName — используем его.
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

void AVestBase::ApplyInventoryIconOverride()
{
	if (!ItemInstance) return;

	if (InventoryIconTexture && InventoryIconTexture->IsValidLowLevel())
	{
		ItemInstance->Icon = InventoryIconTexture;
	}
	else
	{
		ItemInstance->Icon = nullptr;
	}
}

