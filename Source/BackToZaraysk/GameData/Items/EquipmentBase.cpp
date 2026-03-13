// Fill out your copyright notice in the Description page of Project Settings.

#include "EquipmentBase.h"
#include "Components/StaticMeshComponent.h"
#include "BackToZaraysk/Inventory/EquippableItemData.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"

AEquipmentBase::AEquipmentBase()
{
	// Создаем скелетный меш компонент
	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(RootComponent);

	// Скелетный меш по умолчанию скрыт
	SkeletalMesh->SetVisibility(false);
	SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEquipmentBase::BeginPlay()
{
	Super::BeginPlay();

	// Применяем настройки физики из параметров
	if (Mesh)
	{
		Mesh->SetSimulatePhysics(bEnablePhysics);
		Mesh->SetEnableGravity(true);
	}
}

void AEquipmentBase::RebuildModSideGrids()
{
	ModSideGrids.Reset();

	if (!bIsModdable)
	{
		return;
	}

	// Выбираем основной компонент для вычисления габаритов (SkeletalMesh приоритетнее, иначе StaticMesh)
	UPrimitiveComponent* VisualComp = nullptr;
	if (SkeletalMesh && SkeletalMesh->GetSkeletalMeshAsset())
	{
		VisualComp = SkeletalMesh;
	}
	else if (Mesh && Mesh->GetStaticMesh())
	{
		VisualComp = Mesh;
	}

	if (!VisualComp)
	{
		return;
	}

	const FBoxSphereBounds Bounds = VisualComp->GetLocalBounds();
	const FVector LocalMin = Bounds.Origin - Bounds.BoxExtent;
	const FVector LocalMax = Bounds.Origin + Bounds.BoxExtent;

	// Эмпирический размер клетки в мире (см)
	const float CellWorldSize = 5.0f;

	// Перед = грань с максимальным X (грудь в типичной ориентации меша), зад = грань с минимальным X
	auto BuildFrontBack = [&](bool bFront, bool bEnabled, const FIntPoint& MaxGridSize, const FVector& OriginParam, const FRotator& RotationParam, const TCHAR* SideLabel)
	{
		if (!bEnabled) return;

		const float WidthY = LocalMax.Y - LocalMin.Y;
		const float HeightZ = LocalMax.Z - LocalMin.Z;

		int32 AutoX = FMath::Max(1, FMath::FloorToInt(WidthY / CellWorldSize));
		int32 AutoY = FMath::Max(1, FMath::FloorToInt(HeightZ / CellWorldSize));

		FArmorModSideGrid G;
		G.SideName = FName(SideLabel);
		G.CellsX = FMath::Clamp(AutoX, 1, MaxGridSize.X);
		G.CellsY = FMath::Clamp(AutoY, 1, MaxGridSize.Y);

		const float XPlane = bFront ? LocalMax.X : LocalMin.X;
		const float OriginY = bFront ? LocalMin.Y : LocalMax.Y;
		const float OriginZ = LocalMin.Z;
		FVector ComputedOrigin(XPlane, OriginY, OriginZ);
		G.Origin = OriginParam.IsNearlyZero() ? ComputedOrigin : OriginParam;

		FVector DefRight = bFront ? FVector(0.f, 1.f, 0.f) : FVector(0.f, -1.f, 0.f);
		FVector DefUp = FVector(0.f, 0.f, 1.f);
		const FMatrix RotMat = FRotationMatrix(RotationParam);
		G.AxisRight = RotMat.TransformVector(DefRight).GetSafeNormal();
		G.AxisUp = RotMat.TransformVector(DefUp).GetSafeNormal();

		ModSideGrids.Add(G);
	};

	// Лево = грань с минимальным Y, право = грань с максимальным Y
	auto BuildLeftRight = [&](bool bLeft, bool bEnabled, const FIntPoint& MaxGridSize, const FVector& OriginParam, const FRotator& RotationParam, const TCHAR* SideLabel)
	{
		if (!bEnabled) return;

		const float WidthX = LocalMax.X - LocalMin.X;
		const float HeightZ = LocalMax.Z - LocalMin.Z;

		int32 AutoX = FMath::Max(1, FMath::FloorToInt(WidthX / CellWorldSize));
		int32 AutoY = FMath::Max(1, FMath::FloorToInt(HeightZ / CellWorldSize));

		FArmorModSideGrid G;
		G.SideName = FName(SideLabel);
		G.CellsX = FMath::Clamp(AutoX, 1, MaxGridSize.X);
		G.CellsY = FMath::Clamp(AutoY, 1, MaxGridSize.Y);

		const float YPlane = bLeft ? LocalMin.Y : LocalMax.Y;
		const float OriginX = bLeft ? LocalMin.X : LocalMax.X;
		const float OriginZ = LocalMin.Z;
		FVector ComputedOrigin(OriginX, YPlane, OriginZ);
		G.Origin = OriginParam.IsNearlyZero() ? ComputedOrigin : OriginParam;

		FVector DefRight = bLeft ? FVector(1.f, 0.f, 0.f) : FVector(-1.f, 0.f, 0.f);
		FVector DefUp = FVector(0.f, 0.f, 1.f);
		const FMatrix RotMat = FRotationMatrix(RotationParam);
		G.AxisRight = RotMat.TransformVector(DefRight).GetSafeNormal();
		G.AxisUp = RotMat.TransformVector(DefUp).GetSafeNormal();

		ModSideGrids.Add(G);
	};

	// Строим сетки для включённых сторон (Origin и Rotation из параметров Mods* в пространстве модели)
	BuildFrontBack(true,  bModsFront, ModsFrontMaxGridSize, ModsFrontOrigin, ModsFrontRotation, TEXT("Front"));
	BuildFrontBack(false, bModsBack,  ModsBackMaxGridSize,  ModsBackOrigin,  ModsBackRotation,  TEXT("Back"));
	BuildLeftRight(true,  bModsLeft,  ModsLeftMaxGridSize,  ModsLeftOrigin,  ModsLeftRotation,  TEXT("Left"));
	BuildLeftRight(false, bModsRight, ModsRightMaxGridSize, ModsRightOrigin, ModsRightRotation, TEXT("Right"));
}

void AEquipmentBase::ApplyItemInstanceVisuals()
{
    // Настраиваем вид из ItemInstance: если это UEquippableItemData с SkeletalMesh/StaticMesh
    if (!ItemInstance)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("❌ ApplyItemInstanceVisuals: ItemInstance is null"));
        }
        return;
    }
    UEquippableItemData* EqData = Cast<UEquippableItemData>(ItemInstance);
    if (!EqData)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("❌ ApplyItemInstanceVisuals: ItemInstance is not UEquippableItemData"));
        }
        return;
    }
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, 
            FString::Printf(TEXT("🎭 ApplyItemInstanceVisuals: Item=%s, EquippedMesh=%s"), 
                *EqData->DisplayName.ToString(),
                EqData->EquippedMesh ? *EqData->EquippedMesh->GetName() : TEXT("null")));
    }
    
    // Если EquippedMesh стал недействительным, попробуем восстановить его
    if (!EqData->EquippedMesh || !EqData->EquippedMesh->IsValidLowLevel())
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange, 
                FString::Printf(TEXT("🔧 EquippedMesh is invalid for %s, attempting to restore..."), *EqData->DisplayName.ToString()));
        }
        
        // Восстанавливаем EquippedMesh для жилета
        if (EqData->EquipmentSlot == Vest)
        {
            // Пробуем несколько путей для меша жилета
            TArray<FString> VestMeshPaths = {
                TEXT("/Game/Insurgent_2/Mesh/Separate_Parts/SK_ChestRigSmall.SK_ChestRigSmall"),
                TEXT("/Game/insurgent_2/Characters/SK_ChestRigSmall.SK_ChestRigSmall"),
                TEXT("/Game/BackToZaraysk/Core/Items/Meshes/SK_ChestRigSmall.SK_ChestRigSmall")
            };
            
            bool bRestored = false;
            for (const FString& MeshPath : VestMeshPaths)
            {
                EqData->EquippedMesh = LoadObject<USkeletalMesh>(nullptr, *MeshPath);
                if (EqData->EquippedMesh && EqData->EquippedMesh->IsValidLowLevel())
                {
                    if (GEngine)
                    {
                        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                            FString::Printf(TEXT("✅ Restored Vest EquippedMesh: %s from %s"), 
                                *EqData->EquippedMesh->GetName(), *MeshPath));
                    }
                    bRestored = true;
                    break;
                }
            }
            
            if (!bRestored)
            {
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("❌ Failed to restore Vest EquippedMesh from all paths"));
                }
            }
        }
        // Восстанавливаем EquippedMesh для рюкзака
        else if (EqData->EquipmentSlot == Backpack)
        {
            FString MeshPath = TEXT("/Engine/BasicShapes/Cube.Cube");
            EqData->EquippedMesh = LoadObject<UStaticMesh>(nullptr, *MeshPath);
            
            if (EqData->EquippedMesh && EqData->EquippedMesh->IsValidLowLevel())
            {
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                        FString::Printf(TEXT("✅ Restored Backpack EquippedMesh: %s"), *EqData->EquippedMesh->GetName()));
                }
            }
            else
            {
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("❌ Failed to restore Backpack EquippedMesh"));
                }
            }
        }
    }
    
    // Если задан скелетный меш в данных — отобразим SkeletalMesh
    USkeletalMesh* AsSkeletal = Cast<USkeletalMesh>(EqData->EquippedMesh);
    UStaticMesh* AsStatic = Cast<UStaticMesh>(EqData->EquippedMesh);
    if (AsSkeletal && SkeletalMesh)
    {
        SkeletalMesh->SetSkeletalMesh(AsSkeletal);
        SkeletalMesh->SetVisibility(true, true);
        SkeletalMesh->SetHiddenInGame(false, true);
        if (Mesh)
        {
            Mesh->SetVisibility(false, true);
            Mesh->SetHiddenInGame(true, true);
            Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            Mesh->SetSimulatePhysics(false);
        }
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                FString::Printf(TEXT("✅ Applied SkeletalMesh: %s"), *AsSkeletal->GetName()));
        }
        return;
    }
    if (AsStatic && Mesh)
    {
        Mesh->SetStaticMesh(AsStatic);
        Mesh->SetVisibility(true, true);
        Mesh->SetHiddenInGame(false, true);
        SkeletalMesh->SetVisibility(false, true);
        SkeletalMesh->SetHiddenInGame(true, true);
        SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        SkeletalMesh->SetSimulatePhysics(false);
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                FString::Printf(TEXT("✅ Applied StaticMesh: %s"), *AsStatic->GetName()));
        }
        return;
    }
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
            FString::Printf(TEXT("❌ No valid mesh to apply. EquippedMesh=%s, SkeletalMesh=%s, Mesh=%s"), 
                EqData->EquippedMesh ? *EqData->EquippedMesh->GetName() : TEXT("null"),
                SkeletalMesh ? TEXT("valid") : TEXT("null"),
                Mesh ? TEXT("valid") : TEXT("null")));
    }
}

bool AEquipmentBase::Equip(ACharacter* Character)
{
	if (!Character)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
				TEXT("EquipmentBase: Cannot equip - Character is nullptr"));
		}
		return false;
	}

	if (bIsEquipped)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange, 
				TEXT("EquipmentBase: Already equipped"));
		}
		return false;
	}

	// Получаем скелетный меш персонажа
	USkeletalMeshComponent* CharacterMesh = Character->GetMesh();
	if (!CharacterMesh)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
				TEXT("EquipmentBase: Character has no mesh"));
		}
		return false;
	}

	// Проверяем существование сокета
	if (AttachSocketName != TEXT("None") && !CharacterMesh->DoesSocketExist(AttachSocketName))
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
				FString::Printf(TEXT("EquipmentBase: Socket '%s' not found on character mesh"), 
					*AttachSocketName.ToString()));
		}
		return false;
	}

	// Отключаем физику
	SetPhysicsEnabled(false);

	// Прикрепляем к персонажу
	FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, true);
	
	if (AttachSocketName != TEXT("None"))
	{
		AttachToComponent(CharacterMesh, AttachRules, AttachSocketName);
	}
	else
	{
		AttachToComponent(CharacterMesh, AttachRules);
	}

	// Применяем смещение и ротацию
	SetActorRelativeLocation(AttachOffset);
	SetActorRelativeRotation(AttachRotation);
	SetActorRelativeScale3D(AttachScale);

	// Обновляем состояние
	bIsEquipped = true;
	OwnerCharacter = Character;

	// Вызываем событие
	OnEquipped();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
			FString::Printf(TEXT("✅ Equipment equipped on %s (socket: %s)"), 
				*Character->GetName(), *AttachSocketName.ToString()));
	}

	return true;
}

bool AEquipmentBase::Unequip(bool bDropToWorld)
{
	if (!bIsEquipped)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange, 
				TEXT("EquipmentBase: Not equipped"));
		}
		return false;
	}

	// Отсоединяем от персонажа
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	DetachFromActor(DetachRules);

	if (bDropToWorld)
	{
		// Включаем физику для падения в мир
		SetPhysicsEnabled(true);

		// Убираем сильный импульс вперёд, даём лёгкий подъём, чтобы не проваливался/не улетал
		if (OwnerCharacter && Mesh)
		{
			Mesh->AddImpulse(FVector(0.f, 0.f, 150.0f), NAME_None, true);
		}
	}
	else
	{
		// Просто отключаем физику
		SetPhysicsEnabled(false);
	}

	// Обновляем состояние
	bIsEquipped = false;
	OwnerCharacter = nullptr;

	// Вызываем событие
	OnUnequipped();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
			TEXT("Equipment unequipped"));
	}

	return true;
}

void AEquipmentBase::SetPhysicsEnabled(bool bEnabled)
{
	// Настраиваем физику для активного меша
	if (SkeletalMesh && SkeletalMesh->IsVisible())
	{
		SkeletalMesh->SetSimulatePhysics(bEnabled);
		SkeletalMesh->SetEnableGravity(bEnabled);

		if (bEnabled)
		{
			SkeletalMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
		}
		else
		{
			SkeletalMesh->SetCollisionProfileName(TEXT("NoCollision"));
		}
	}
	else if (Mesh)
	{
		Mesh->SetSimulatePhysics(bEnabled);
		Mesh->SetEnableGravity(bEnabled);

		if (bEnabled)
		{
			Mesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
		}
		else
		{
			Mesh->SetCollisionProfileName(TEXT("NoCollision"));
		}
	}
}

void AEquipmentBase::OnEquipped_Implementation()
{
	// Базовая реализация - можно переопределить в Blueprint
	// Здесь можно добавить звуки, эффекты и т.д.
}

void AEquipmentBase::OnUnequipped_Implementation()
{
	// Базовая реализация - можно переопределить в Blueprint
	// Здесь можно добавить звуки, эффекты и т.д.
}

