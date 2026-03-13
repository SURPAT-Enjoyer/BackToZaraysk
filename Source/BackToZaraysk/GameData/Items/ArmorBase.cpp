#include "ArmorBase.h"
#include "Components/StaticMeshComponent.h"
#include "BackToZaraysk/Inventory/EquippableItemData.h"
#include "Engine/Texture2D.h"
#include "Engine/SkeletalMesh.h"

AArmorBase::AArmorBase()
{
	// Делаем поведение максимально похожим на AVestBase:
	// - мировой pickup показывает SkeletalMesh (если он задан в BP)
	// - статический Mesh служит только плейсхолдером и по умолчанию скрыт.
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
	}

	if (SkeletalMesh)
	{
		if (!SkeletalMesh->GetSkeletalMeshAsset())
		{
			if (USkeletalMesh* Sk = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/BackToZaraysk/Core/Items/Equipment/SK_BackpackSmall.SK_BackpackSmall")))
			{
				SkeletalMesh->SetSkeletalMesh(Sk);
			}
		}
		SkeletalMesh->SetVisibility(true, true);
		SkeletalMesh->SetHiddenInGame(false, true);
		SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		SkeletalMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
		SkeletalMesh->SetUseCCD(true);
		SkeletalMesh->SetSimulatePhysics(bEnablePhysics);
	}

	ItemClass = UArmorBaseItemData::StaticClass();
	AttachSocketName = AttachBoneName.IsNone() ? FName(TEXT("spine_02")) : AttachBoneName;
}

void AArmorBase::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

	// Применяем настройки из BP (InventorySizeX/Y, иконка, модифицируемость и т.п.)
	ApplyEditorOverridesToItemInstance();
	ApplyInventoryIconOverride();

	// Пересчитываем сетки модификаций (по сторонам) при каждом изменении в редакторе
	RebuildModSideGrids();
}

void AArmorBase::BeginPlay()
{
	Super::BeginPlay();

	// Если это pickup, размещённый в уровне: создаём ItemInstance из ItemClass (как у VestBase).
	bool bCreatedItemInstanceHere = false;
	if (!ItemInstance && ItemClass)
	{
		if (UInventoryItemData* NewData = NewObject<UInventoryItemData>(this, ItemClass))
		{
			ItemInstance = NewData;
			bCreatedItemInstanceHere = true;
		}
	}

	// Применяем настройки из BP (размер в инвентаре, модифицируемость и т.д.)
	ApplyEditorOverridesToItemInstance();

	// Если это placed pickup и у него в BP задан SkeletalMesh, подхватываем его в EquippedMesh,
	// чтобы при выбросе/экипировке использовался именно этот меш (ровно как у VestBase).
	if (UEquippableItemData* Eq = Cast<UEquippableItemData>(ItemInstance))
	{
		if (bCreatedItemInstanceHere && SkeletalMesh && SkeletalMesh->GetSkeletalMeshAsset())
		{
			Eq->EquippedMesh = SkeletalMesh->GetSkeletalMeshAsset();
		}
	}

	// Применяем override иконки (или сбрасываем на белую заливку через nullptr)
	ApplyInventoryIconOverride();

	// Применяем визуал из ItemInstance (SkeletalMesh/StaticMesh) – логика та же, что и у VestBase через AEquipmentBase.
	if (ItemInstance)
	{
		ApplyItemInstanceVisuals();
	}
}

void AArmorBase::ApplyInventoryIconOverride()
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

void AArmorBase::ApplyEditorOverridesToItemInstance()
{
	if (!ItemInstance)
	{
		return;
	}

	// Размер предмета в инвентаре берём из полей Blueprint'а (InventorySizeX/Y)
	ItemInstance->SizeInCellsX = FMath::Max(1, InventorySizeX);
	ItemInstance->SizeInCellsY = FMath::Max(1, InventorySizeY);

	if (UEquippableItemData* Eq = Cast<UEquippableItemData>(ItemInstance))
	{
		// Слот — бронежилет
		Eq->EquipmentSlot = Armor;

		// Кость/сокет аттача:
		// 1) Если задан Armor|Attachment::AttachBoneName — используем его.
		// 2) Иначе, если в Equipment|Attachment::AttachSocketName выбран сокет (не "None") — берём его.
		// 3) Иначе fallback на spine_02.
		FName EffectiveSocket = AttachBoneName;
		if (EffectiveSocket.IsNone())
		{
			if (!AttachSocketName.IsNone() && AttachSocketName != TEXT("None"))
			{
				EffectiveSocket = AttachSocketName;
			}
			else
			{
				EffectiveSocket = FName(TEXT("spine_02"));
			}
		}

		Eq->AttachSocketName = EffectiveSocket;

		// Масштаб при экипировке 1x1x1 (чтобы модель не была крошечной)
		FVector Loc = Eq->RelativeTransform.GetLocation();
		FRotator Rot = Eq->RelativeTransform.Rotator();
		Eq->RelativeTransform = FTransform(Rot, Loc, FVector(1.0f, 1.0f, 1.0f));

		// Флаг модифицируемости из BP
		Eq->bIsModdable = bIsModdable;

		// Копируем сетки модификаций для превью и UI
		Eq->ModSideGrids = ModSideGrids;

		// Здесь дополнительных хранилищ нет (бронеплита без карманов),
		// поэтому явно фиксируем соответствующие флаги.
		Eq->bHasAdditionalStorage = false;
		Eq->AdditionalGridSize = FIntPoint(0, 0);
		Eq->AdditionalGrids.Reset();
		Eq->AdditionalGridOffsets.Reset();

		// Синхронизируем оба поля на акторе, чтобы в Details не было расхождений.
		AttachBoneName = EffectiveSocket;
		AttachSocketName = EffectiveSocket;
	}
	else
	{
		// На всякий случай: если ItemInstance не UEquippableItemData,
		// синхронизируем только поля актора по AttachBoneName / AttachSocketName.
		FName EffectiveSocket = AttachBoneName.IsNone() ? FName(TEXT("spine_02")) : AttachBoneName;
		if (EffectiveSocket.IsNone() && !AttachSocketName.IsNone() && AttachSocketName != TEXT("None"))
		{
			EffectiveSocket = AttachSocketName;
		}
		AttachBoneName = EffectiveSocket;
		AttachSocketName = EffectiveSocket;
	}
}


