#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArmorModSideGrid.h"
#include "Components/LineBatchComponent.h"
#include "ArmorModPreviewActor.generated.h"

class USceneCaptureComponent2D;
class USkeletalMeshComponent;
class UStaticMeshComponent;
class UTextureRenderTarget2D;
class USkyLightComponent;
class UDirectionalLightComponent;
class UPointLightComponent;
class ULineBatchComponent;

/**
 * Актор для превью 3D-модели бронежилета в окне модификации.
 * Рендерит меш в UTextureRenderTarget2D через SceneCaptureComponent2D.
 */
UCLASS()
class BACKTOZARAYSK_API AArmorModPreviewActor : public AActor
{
	GENERATED_BODY()

public:
	AArmorModPreviewActor();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	/** Установить меш из данных предмета (SkeletalMesh или StaticMesh). */
	void SetMeshFromEquipped(UObject* EquippedMesh);

	/** Установить сетки модификаций для отрисовки на модели (из UEquippableItemData::ModSideGrids). */
	void SetModGrids(const TArray<FArmorModSideGrid>& Grids);

	/** Отобразить меши установленных модов на превью в позициях по сетке (из ArmorItemData::InstalledMods + ModSideGrids). */
	void SetInstalledMods(class UEquippableItemData* ArmorItemData);

	/** Угол поворота модели: Pitch (X), Yaw (Y), Roll (Z) в градусах. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ArmorModPreview")
	float PreviewPitchDegrees = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ArmorModPreview")
	float PreviewYawDegrees = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ArmorModPreview")
	float PreviewRollDegrees = 0.f;

	/** Применить текущий поворот к мешу. */
	void ApplyRotation();

	/** Дистанция камеры от модели (приближение/удаление по колесу мыши). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ArmorModPreview", meta = (ClampMin = "50", ClampMax = "600"))
	float CameraDistance = 220.f;

	/** Смещение камеры (pan) в локальном пространстве: X — вперёд/назад, Y — вправо, Z — вверх. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ArmorModPreview")
	FVector CameraPanOffset = FVector::ZeroVector;

	/** Добавить смещение камеры (перемещение в трёх плоскостях без вращения модели). Вызывать при драге ЛКМ+ПКМ. */
	void AddPanDelta(float DeltaRight, float DeltaUp, float DeltaForward = 0.f);

	/** Приблизить/отдалить камеру (положительный Delta — приближение). Вызывать при кручении колеса мыши. */
	void AddZoomDelta(float Delta);

	/** Применить текущее смещение камеры к SceneCapture. */
	void ApplyCameraPan();

	/** Компонент захвата сцены — рендер в текстуру. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneCaptureComponent2D> CaptureComponent;

	/** Скелетный меш (если предмет использует SkeletalMesh). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;

	/** Статический меш (если предмет использует StaticMesh). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;

	/** Основной направленный свет. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UDirectionalLightComponent> LightComponent = nullptr;

	/** Заполняющий свет (вторая лампа). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UDirectionalLightComponent> FillLightComponent = nullptr;

	/** Небесный свет (ambient). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkyLightComponent> SkyLightComponent = nullptr;

	/** Точечный свет перед объектом. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPointLightComponent> PointLightComponent = nullptr;

	/** Отрисовка линий сетки (рендерится в сцене и попадает в SceneCapture). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<ULineBatchComponent> GridLineBatchComponent = nullptr;

	/** Назначить цель захвата (создаётся и владеется виджетом). */
	void SetRenderTarget(UTextureRenderTarget2D* RT);

	/** Хит-тест: позиция внутри виджета превью (0..Size.X, 0..Size.Y) и размер виджета. Возвращает true и заполняет OutSideIndex, OutCellX, OutCellY если луч попал в ячейку сетки. */
	bool HitTestGridFromWidgetPosition(FVector2D LocalPosInWidget, FVector2D WidgetSize, int32& OutSideIndex, int32& OutCellX, int32& OutCellY) const;

	/** Одна ячейка для подсветки (SideIndex, CellX, CellY, bValid: true=зелёная, false=красная). */
	struct FHighlightCell { int32 SideIndex = 0; int32 CellX = 0; int32 CellY = 0; bool bValid = true; };
	void SetHighlightCells(const TArray<FHighlightCell>& Cells);

	/** Текущие сетки для отрисовки (копия из SetModGrids). В Details видны Origin и параметры каждой сетки. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ArmorModPreview|Grid", meta = (DisplayName = "Mod Grids (Origin per side)"))
	TArray<FArmorModSideGrid> ModGrids;

private:
	void FrameCaptureToActiveMesh();
	void RefreshCaptureShowOnly();

	UPROPERTY()
	TObjectPtr<USceneComponent> MeshHolder = nullptr;

	/** Кэш отрисованных линий сетки — пересчёт только при изменении трансформа меша (без тормозов каждый кадр). */
	TArray<FBatchedLine> CachedGridLines;
	FTransform CachedGridMeshTransform;

	TArray<FHighlightCell> HighlightCells;

	/** Размер клетки в см. */
	static constexpr float ModGridCellWorldSize = 5.0f;
	static constexpr float ModGridTraceDistance = 200.f;
	/** Сегментов на ребро клетки (меньше = быстрее, сетка по поверхности). */
	static constexpr int32 ModGridSegmentsPerCellEdge = 6;

	/** Треугольники меша в локальном пространстве (видимая геометрия). */
	TArray<FVector> MeshTrianglesLocal;
	/** AABB каждого треугольника в локальном пространстве — для быстрого отсечения лучей. */
	TArray<FBox> MeshTriangleBoundsLocal;
	/** Построить кэш треугольников на следующем тике (не блокировать открытие окна). */
	bool bPendingMeshTriangleCache = false;

	/** Построить кэш треугольников видимой геометрии (StaticMesh — через GetSectionFromStaticMesh; SkeletalMesh — через LOD). */
	void BuildMeshTriangleCache(UPrimitiveComponent* MeshComp);

	/** Спроецировать точку на поверхность меша по видимой геометрии (ray–triangle). Если кэш пуст — возвращает WorldPoint. */
	FVector ProjectPointOntoMesh(UPrimitiveComponent* MeshComp, const FTransform& MeshToWorld, const FVector& WorldPoint, const FVector& WorldPlaneNormal) const;

	/** Построить одну сетку по умолчанию (фронт) из bounds текущего меша, если ModGrids пуст. */
	void BuildDefaultGridFromMeshBounds();
};
