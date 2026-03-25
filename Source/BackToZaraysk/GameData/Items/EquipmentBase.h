// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Test/PickupBase.h"
#include "ArmorModSideGrid.h"
#include "EquipmentBase.generated.h"

/**
 * Базовый класс для экипируемых предметов (оружие, рюкзаки, броня и т.д.)
 * Наследуется от PickupBase, добавляя функциональность экипировки
 */
UCLASS(Blueprintable)
class BACKTOZARAYSK_API AEquipmentBase : public APickupBase
{
	GENERATED_BODY()

public:
	AEquipmentBase();

	// === КОМПОНЕНТЫ ===

	// Скелетный меш для экипировки (например, жилеты, рюкзаки)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class USkeletalMeshComponent> SkeletalMesh;

	// === НАСТРОЙКИ ЭКИПИРОВКИ ===

	// Имя сокета на персонаже для прикрепления (например, "weapon_r", "spine_03" для рюкзака)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|Attachment")
	FName AttachSocketName = TEXT("None");

	// Смещение относительно сокета
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|Attachment")
	FVector AttachOffset = FVector::ZeroVector;

	// Поворот относительно сокета
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|Attachment")
	FRotator AttachRotation = FRotator::ZeroRotator;

	// Масштаб при экипировке
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|Attachment")
	FVector AttachScale = FVector(1.0f, 1.0f, 1.0f);

	// === СОСТОЯНИЕ ===

	// Экипирован ли предмет на персонаже
	UPROPERTY(BlueprintReadOnly, Category = "Equipment|State")
	bool bIsEquipped = false;

	// Владелец (персонаж, на котором экипирован)
	UPROPERTY(BlueprintReadOnly, Category = "Equipment|State")
	TObjectPtr<class ACharacter> OwnerCharacter = nullptr;

	// === Модификации (обвесы) ===

	// Можно ли на этот предмет вешать модули (подсумки и т.п.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Armor|Mods")
	bool bIsModdable = false;

	// Сторона "спереди" (front) имеет сетку под модули
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Armor|Mods", meta=(EditCondition="bIsModdable", EditConditionHides))
	bool bModsFront = false;

	// Максимальный размер сетки модификаций спереди (в клетках X/Y)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Armor|Mods", meta=(EditCondition="bIsModdable && bModsFront", EditConditionHides, ClampMin="1", UIMin="1"))
	FIntPoint ModsFrontMaxGridSize = FIntPoint(1, 1);
	// Начало отсчёта сетки спереди — координаты в пространстве модели (левый нижний угол сетки)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Armor|Mods", meta=(EditCondition="bIsModdable && bModsFront", EditConditionHides))
	FVector ModsFrontOrigin = FVector::ZeroVector;
	// Поворот сетки спереди (Pitch/Yaw/Roll в градусах)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Armor|Mods", meta=(EditCondition="bIsModdable && bModsFront", EditConditionHides))
	FRotator ModsFrontRotation = FRotator::ZeroRotator;

	// Сторона "сзади" (back) имеет сетку под модули
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Armor|Mods", meta=(EditCondition="bIsModdable", EditConditionHides))
	bool bModsBack = false;

	// Максимальный размер сетки модификаций сзади (в клетках X/Y)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Armor|Mods", meta=(EditCondition="bIsModdable && bModsBack", EditConditionHides, ClampMin="1", UIMin="1"))
	FIntPoint ModsBackMaxGridSize = FIntPoint(1, 1);
	// Начало отсчёта сетки сзади — координаты в пространстве модели
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Armor|Mods", meta=(EditCondition="bIsModdable && bModsBack", EditConditionHides))
	FVector ModsBackOrigin = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Armor|Mods", meta=(EditCondition="bIsModdable && bModsBack", EditConditionHides))
	FRotator ModsBackRotation = FRotator::ZeroRotator;

	// Сторона "слева" (left) имеет сетку под модули
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Armor|Mods", meta=(EditCondition="bIsModdable", EditConditionHides))
	bool bModsLeft = false;

	// Максимальный размер сетки модификаций слева (в клетках X/Y)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Armor|Mods", meta=(EditCondition="bIsModdable && bModsLeft", EditConditionHides, ClampMin="1", UIMin="1"))
	FIntPoint ModsLeftMaxGridSize = FIntPoint(1, 1);
	// Начало отсчёта сетки слева — координаты в пространстве модели
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Armor|Mods", meta=(EditCondition="bIsModdable && bModsLeft", EditConditionHides))
	FVector ModsLeftOrigin = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Armor|Mods", meta=(EditCondition="bIsModdable && bModsLeft", EditConditionHides))
	FRotator ModsLeftRotation = FRotator::ZeroRotator;

	// Сторона "справа" (right) имеет сетку под модули
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Armor|Mods", meta=(EditCondition="bIsModdable", EditConditionHides))
	bool bModsRight = false;

	// Максимальный размер сетки модификаций справа (в клетках X/Y)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Armor|Mods", meta=(EditCondition="bIsModdable && bModsRight", EditConditionHides, ClampMin="1", UIMin="1"))
	FIntPoint ModsRightMaxGridSize = FIntPoint(1, 1);
	// Начало отсчёта сетки справа — координаты в пространстве модели
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Armor|Mods", meta=(EditCondition="bIsModdable && bModsRight", EditConditionHides))
	FVector ModsRightOrigin = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Armor|Mods", meta=(EditCondition="bIsModdable && bModsRight", EditConditionHides))
	FRotator ModsRightRotation = FRotator::ZeroRotator;

	// Результат пересчёта сеток (Origin берётся из Mods*Origin для каждой стороны)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Armor|Mods", meta=(DisplayName="Mod Side Grids"))
	TArray<FArmorModSideGrid> ModSideGrids;

	/** Пересчитать ModSideGrids на основе текущего меша и настроек bMods* / Mods*MaxGridSize (вызывается из редактора сеток). */
	void RebuildModSideGrids();

	// === ФУНКЦИИ ===

	// Экипировать предмет на персонажа
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	virtual bool Equip(ACharacter* Character);

	// Снять предмет с персонажа
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	virtual bool Unequip(bool bDropToWorld = false);

	// Установить физику (вкл/выкл)
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void SetPhysicsEnabled(bool bEnabled);

	// Вызывается при экипировке (можно переопределить в Blueprint)
	UFUNCTION(BlueprintNativeEvent, Category = "Equipment")
	void OnEquipped();
	virtual void OnEquipped_Implementation();

	// Вызывается при снятии экипировки (можно переопределить в Blueprint)
	UFUNCTION(BlueprintNativeEvent, Category = "Equipment")
	void OnUnequipped();
	virtual void OnUnequipped_Implementation();

	// Применяем визуал из ItemInstance (меши/видимость)
	virtual void ApplyItemInstanceVisuals() override;

protected:
	virtual void BeginPlay() override;
};

