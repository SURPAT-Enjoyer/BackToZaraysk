#pragma once

#include "CoreMinimal.h"
#include "ArmorModSideGrid.generated.h"

/** Геометрия сетки модификаций на стороне бронежилета (для UI и превью). */
USTRUCT(BlueprintType)
struct BACKTOZARAYSK_API FArmorModSideGrid
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Armor|Mods")
	FName SideName = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Armor|Mods")
	int32 CellsX = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Armor|Mods")
	int32 CellsY = 0;

	/** Начало отсчёта сетки в пространстве модели (задаётся параметром Mods*Origin на брони для каждой стороны). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Armor|Mods", meta = (DisplayName = "Origin (X Y Z)"))
	FVector Origin = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Armor|Mods")
	FVector AxisRight = FVector::RightVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Armor|Mods")
	FVector AxisUp = FVector::UpVector;
};
