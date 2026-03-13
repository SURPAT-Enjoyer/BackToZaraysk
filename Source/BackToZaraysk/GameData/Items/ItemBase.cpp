#include "ItemBase.h"

#include "BackToZaraysk/Inventory/InventoryItemData.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInterface.h"

AItemBase::AItemBase()
{
	// Добавляем SkeletalMesh-компонент (опциональный визуал)
	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	if (SkeletalMesh)
	{
		SkeletalMesh->SetupAttachment(RootComponent);
		SkeletalMesh->SetVisibility(false);
		SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SkeletalMesh->SetSimulatePhysics(false);
	}

	// Базовый тип данных предмета
	ItemClass = UItemBaseItemData::StaticClass();

	// Placeholder-визуал: маленький куб, если пользователь не назначил меши
	ApplyDefaultVisualIfNeeded();
	SyncVisibleMeshAndScale();
}

void AItemBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	ApplyDefaultVisualIfNeeded();
	SyncVisibleMeshAndScale();

	if (ItemInstance)
	{
		ApplyEditorOverridesToItemInstance();
	}
}

void AItemBase::BeginPlay()
{
	Super::BeginPlay();

	// Если это pickup, размещённый в уровне: создаём ItemInstance и применяем параметры из Details
	if (!ItemInstance && ItemClass)
	{
		if (UInventoryItemData* NewData = NewObject<UInventoryItemData>(this, ItemClass))
		{
			ItemInstance = NewData;
		}
	}

	ApplyEditorOverridesToItemInstance();

	// Для BP-наследников: сохраняем выбранный меш в ItemInstance,
	// чтобы при выбросе из инвентаря в мир можно было восстановить визуал.
	if (ItemInstance && !ItemInstance->WorldMesh)
	{
		const bool bHasSk = (SkeletalMesh && SkeletalMesh->GetSkeletalMeshAsset() != nullptr);
		if (bHasSk)
		{
			ItemInstance->WorldMesh = SkeletalMesh->GetSkeletalMeshAsset();
		}
		else if (Mesh && Mesh->GetStaticMesh())
		{
			UStaticMesh* CurrentMesh = Mesh->GetStaticMesh();
			UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
			const bool bIsCube = (Cube && CurrentMesh == Cube) || CurrentMesh->GetPathName().Contains(TEXT("/Engine/BasicShapes/Cube"));
			if (!bIsCube)
			{
				ItemInstance->WorldMesh = CurrentMesh;
			}
		}
	}

	// При выбрасывании/спавне из инвентаря ItemInstance уже может быть выставлен — тогда просто применим визуал
	ApplyItemInstanceVisuals();
}

#if WITH_EDITOR
void AItemBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	ApplyDefaultVisualIfNeeded();
	SyncVisibleMeshAndScale();

	if (ItemInstance)
	{
		ApplyEditorOverridesToItemInstance();
	}
}
#endif

void AItemBase::ApplyItemInstanceVisuals()
{
	// Поддержка и StaticMesh, и SkeletalMesh: визуал из ItemInstance->WorldMesh.
	// При смене типа меша второй компонент сбрасываем, чтобы отображался только выбранный.
	if (ItemInstance && ItemInstance->WorldMesh)
	{
		if (USkeletalMesh* SK = Cast<USkeletalMesh>(ItemInstance->WorldMesh))
		{
			if (SkeletalMesh)
			{
				SkeletalMesh->SetSkeletalMesh(SK);
			}
			if (Mesh)
			{
				Mesh->SetStaticMesh(nullptr);
				Mesh->SetVisibility(false);
				Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				Mesh->SetSimulatePhysics(false);
			}
		}
		else if (UStaticMesh* SM = Cast<UStaticMesh>(ItemInstance->WorldMesh))
		{
			if (Mesh)
			{
				Mesh->SetStaticMesh(SM);
			}
			if (SkeletalMesh)
			{
				SkeletalMesh->SetSkeletalMesh(nullptr);
				SkeletalMesh->SetVisibility(false);
				SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				SkeletalMesh->SetSimulatePhysics(false);
			}
		}
	}

	ApplyDefaultVisualIfNeeded();
	SyncVisibleMeshAndScale();
}

void AItemBase::ApplyEditorOverridesToItemInstance()
{
	if (!ItemInstance) return;

	ItemInstance->SizeInCellsX = FMath::Max(1, InventorySizeX);
	ItemInstance->SizeInCellsY = FMath::Max(1, InventorySizeY);
	ItemInstance->WeightKg = WeightKg;

	ApplyInventoryIconOverride();
}

void AItemBase::ApplyInventoryIconOverride()
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

void AItemBase::ApplyDefaultVisualIfNeeded()
{
	// Если есть меши, то placeholder не нужен.
	const bool bHasSk = (SkeletalMesh && SkeletalMesh->GetSkeletalMeshAsset() != nullptr);
	const bool bHasSt = (Mesh && Mesh->GetStaticMesh() != nullptr);
	if (bHasSk || bHasSt)
	{
		return;
	}

	if (!Mesh) return;

	if (UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
	{
		Mesh->SetStaticMesh(Cube);
	}
	if (UMaterialInterface* M = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/EngineMaterials/DefaultWhiteGrid.DefaultWhiteGrid")))
	{
		Mesh->SetMaterial(0, M);
	}
}

void AItemBase::SyncVisibleMeshAndScale()
{
	const bool bHasSk = (SkeletalMesh && SkeletalMesh->GetSkeletalMeshAsset() != nullptr);

	if (bHasSk)
	{
		// Показываем SkeletalMesh, скрываем StaticMesh
		if (SkeletalMesh)
		{
			SkeletalMesh->SetVisibility(true);
			SkeletalMesh->SetRelativeScale3D(WorldVisualScale);
		}
		if (Mesh)
		{
			Mesh->SetVisibility(false);
			Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Mesh->SetSimulatePhysics(false);
		}
		return;
	}

	// Иначе показываем StaticMesh (placeholder — маленький кубик).
	if (Mesh)
	{
		Mesh->SetVisibility(true);
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Mesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
		Mesh->SetSimulatePhysics(bEnablePhysics);
		Mesh->SetEnableGravity(bEnablePhysics);

		const bool bIsEngineCube =
			(Mesh->GetStaticMesh() == nullptr) ||
			(Mesh->GetStaticMesh()->GetPathName().Contains(TEXT("/Engine/BasicShapes/Cube")));

		if (bIsEngineCube)
		{
			// Требование: по умолчанию маленький куб 0.1х0.1 (умножаем на WorldVisualScale)
			Mesh->SetRelativeScale3D(FVector(0.1f, 0.1f, 0.1f) * WorldVisualScale);
		}
		else
		{
			// Для назначенных пользователем статических мешей — масштаб 1x1x1 (умножаем на WorldVisualScale)
			Mesh->SetRelativeScale3D(WorldVisualScale);
		}
	}

	if (SkeletalMesh)
	{
		SkeletalMesh->SetVisibility(false);
		SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SkeletalMesh->SetSimulatePhysics(false);
	}
}

