// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Test/PickupBase.h"
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

protected:
	virtual void BeginPlay() override;
};

