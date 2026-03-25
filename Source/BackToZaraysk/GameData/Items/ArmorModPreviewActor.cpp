#include "ArmorModPreviewActor.h"
#include "BackToZaraysk/Inventory/EquippableItemData.h"
#include "BackToZaraysk/Inventory/InventoryItemData.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Rendering/SkeletalMeshRenderData.h"
#include "Engine/Engine.h"
#include "Materials/Material.h"
#include "UObject/ConstructorHelpers.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "Components/LineBatchComponent.h"
#include "ProceduralMeshComponent.h"
#include "KismetProceduralMeshLibrary.h"

AArmorModPreviewActor::AArmorModPreviewActor()
{
	PrimaryActorTick.bCanEverTick = true;
	// Коллизия актора должна быть включена, иначе трассировка сетки не попадает в меш (ProjectPointOntoMesh).
	SetActorEnableCollision(true);

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	MeshHolder = CreateDefaultSubobject<USceneComponent>(TEXT("MeshHolder"));
	MeshHolder->SetupAttachment(Root);

	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMeshComponent->SetupAttachment(MeshHolder);
	SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SkeletalMeshComponent->SetCollisionResponseToAllChannels(ECR_Block); // трассировка должна попадать в меш
	SkeletalMeshComponent->SetVisibility(true);
	SkeletalMeshComponent->SetHiddenInGame(false);

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMeshComponent->SetupAttachment(MeshHolder);
	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	StaticMeshComponent->SetCollisionResponseToAllChannels(ECR_Block); // трассировка должна попадать в меш
	StaticMeshComponent->SetVisibility(true);
	StaticMeshComponent->SetHiddenInGame(false);

	// Основной направленный свет: слева сзади за камерой (умеренная интенсивность, без пересвета)
	LightComponent = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("DirectionalLight"));
	LightComponent->SetupAttachment(Root);
	LightComponent->SetRelativeRotation(FRotator(-25.f, 135.f, 0.f)); // сзади-слева, чуть сверху
	LightComponent->Intensity = 4.f;
	LightComponent->LightColor = FColor::White;
	LightComponent->CastShadows = true;

	// Заполняющий свет (с противоположной стороны)
	FillLightComponent = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("FillLight"));
	FillLightComponent->SetupAttachment(Root);
	FillLightComponent->SetRelativeRotation(FRotator(-30.f, -120.f, 0.f));
	FillLightComponent->Intensity = 1.5f;
	FillLightComponent->LightColor = FColor(200, 220, 255); // лёгкий холодный оттенок
	FillLightComponent->CastShadows = false;

	// Небесный свет (ambient)
	SkyLightComponent = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
	SkyLightComponent->SetupAttachment(Root);
	SkyLightComponent->Intensity = 0.4f;
	SkyLightComponent->LightColor = FColor::White;
	// UE57 migration note: отключаем realtime capture, чтобы не получать предупреждения в превью.
	SkyLightComponent->bRealTimeCapture = false;

	// Точечный свет слева сзади за камерой
	PointLightComponent = CreateDefaultSubobject<UPointLightComponent>(TEXT("PointLight"));
	PointLightComponent->SetupAttachment(Root);
	PointLightComponent->SetRelativeLocation(FVector(280.f, -100.f, 40.f)); // сзади и слева от камеры
	PointLightComponent->Intensity = 800.f;
	PointLightComponent->AttenuationRadius = 500.f;
	PointLightComponent->LightColor = FColor::White;
	PointLightComponent->CastShadows = false;

	GridLineBatchComponent = CreateDefaultSubobject<ULineBatchComponent>(TEXT("GridLineBatch"));
	GridLineBatchComponent->SetupAttachment(Root);
	GridLineBatchComponent->SetHiddenInGame(false);

	CaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));
	CaptureComponent->SetupAttachment(Root);
	CaptureComponent->bCaptureEveryFrame = true; // Включаем постоянный захват для корректной работы
	CaptureComponent->bCaptureOnMovement = true;
	CaptureComponent->bAlwaysPersistRenderingState = true; // Решает проблему "SceneCapture has no view"
	// UE57 migration note: используем обычный финальный проход как в runtime-рендере.
	CaptureComponent->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	// Камера смотрит вдоль -X на меш у origin; смещение задаётся через CameraPanOffset
	CaptureComponent->SetRelativeLocation(FVector(220.f, 0.f, 0.f));
	CaptureComponent->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));
	CaptureComponent->FOVAngle = 45.f;
	
	// Настраиваем ShowFlags, чтобы не захватывать небо из основного мира
	FEngineShowFlags ShowFlags = CaptureComponent->ShowFlags;
	ShowFlags.Atmosphere = false; // Отключаем атмосферу/небо
	ShowFlags.SkyLighting = false; // Отключаем освещение от неба
	ShowFlags.DynamicShadows = true; // Включаем динамические тени
	ShowFlags.AmbientCubemap = false; // Отключаем ambient cubemap
	CaptureComponent->ShowFlags = ShowFlags;
	
	// Серый фон через PostProcess
	CaptureComponent->PostProcessSettings.bOverride_SceneFringeIntensity = true;
	CaptureComponent->PostProcessSettings.SceneFringeIntensity = 0.f;
	CaptureComponent->PostProcessSettings.bOverride_FilmGrainIntensity = true;
	CaptureComponent->PostProcessSettings.FilmGrainIntensity = 0.f;
	// Фиксируем экспозицию для предсказуемого UI-превью (без авто-адаптации в черноту).
	CaptureComponent->PostProcessSettings.bOverride_AutoExposureMethod = true;
	CaptureComponent->PostProcessSettings.AutoExposureMethod = EAutoExposureMethod::AEM_Manual;
	CaptureComponent->PostProcessSettings.bOverride_AutoExposureBias = true;
	CaptureComponent->PostProcessSettings.AutoExposureBias = 0.0f;
	
	// Устанавливаем цвет фона (серый)
	CaptureComponent->PostProcessSettings.bOverride_ColorGradingIntensity = true;
	CaptureComponent->PostProcessSettings.ColorGradingIntensity = 1.0f;
}

void AArmorModPreviewActor::BeginPlay()
{
	Super::BeginPlay();
	
	// Убеждаемся, что все компоненты зарегистрированы
	RegisterAllComponents();
	
	// Убеждаемся, что CaptureComponent зарегистрирован
	if (CaptureComponent && !CaptureComponent->IsRegistered())
	{
		CaptureComponent->RegisterComponent();
	}
}

void AArmorModPreviewActor::SetMeshFromEquipped(UObject* EquippedMesh)
{
	if (!EquippedMesh)
	{
		SkeletalMeshComponent->SetSkeletalMesh(nullptr);
		SkeletalMeshComponent->SetVisibility(false);
		SkeletalMeshComponent->SetHiddenInGame(true);
		StaticMeshComponent->SetStaticMesh(nullptr);
		StaticMeshComponent->SetVisibility(false);
		StaticMeshComponent->SetHiddenInGame(true);
		MeshTrianglesLocal.Empty();
		return;
	}

	if (USkeletalMesh* Sk = Cast<USkeletalMesh>(EquippedMesh))
	{
		// Полностью скрываем статический меш
		StaticMeshComponent->SetStaticMesh(nullptr);
		StaticMeshComponent->SetVisibility(false);
		StaticMeshComponent->SetHiddenInGame(true);

		SkeletalMeshComponent->SetSkeletalMesh(Sk);
		SkeletalMeshComponent->SetVisibility(true);
		SkeletalMeshComponent->SetHiddenInGame(false);
		SkeletalMeshComponent->SetRelativeRotation(FRotator::ZeroRotator);
		SkeletalMeshComponent->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
		
		// SkeletalMesh без скелета/анимации рендерится в ref pose; принудительно обновляем кости и bounds
		SkeletalMeshComponent->RefreshBoneTransforms();
		SkeletalMeshComponent->UpdateBounds();
		SkeletalMeshComponent->MarkRenderStateDirty();

		// UE57 migration note: в превью используем только bounds-центр (без pivot по кости),
		// чтобы избежать нестабильных bone-space значений и "улёта" меша из кадра.
		{
			SkeletalMeshComponent->UpdateBounds();
			const FBoxSphereBounds WorldBounds = SkeletalMeshComponent->CalcBounds(SkeletalMeshComponent->GetComponentTransform());
			const FVector LocalCenter = SkeletalMeshComponent->GetComponentTransform().Inverse().TransformPosition(WorldBounds.Origin);
			const bool bCenterFinite = LocalCenter.X == LocalCenter.X && LocalCenter.Y == LocalCenter.Y && LocalCenter.Z == LocalCenter.Z;
			const bool bCenterReasonable = LocalCenter.SizeSquared() < FMath::Square(10000.0f);
			if (bCenterFinite && bCenterReasonable)
			{
				SkeletalMeshComponent->SetRelativeLocation(-LocalCenter);
			}
			else
			{
				SkeletalMeshComponent->SetRelativeLocation(FVector::ZeroVector);
			}
		}
		
		// Регистрируем компонент в мире
		if (SkeletalMeshComponent->IsRegistered())
		{
			SkeletalMeshComponent->ReregisterComponent();
		}
		else
		{
			SkeletalMeshComponent->RegisterComponent();
		}
		
		SkeletalMeshComponent->SetRenderCustomDepth(false);
		SkeletalMeshComponent->SetCastShadow(true);

		RefreshCaptureShowOnly();
		
	}
	else if (UStaticMesh* Sm = Cast<UStaticMesh>(EquippedMesh))
	{
		// Полностью скрываем скелетный меш
		SkeletalMeshComponent->SetSkeletalMesh(nullptr);
		SkeletalMeshComponent->SetVisibility(false);
		SkeletalMeshComponent->SetHiddenInGame(true);

		StaticMeshComponent->SetStaticMesh(Sm);
		StaticMeshComponent->SetVisibility(true);
		StaticMeshComponent->SetHiddenInGame(false);
		StaticMeshComponent->SetRelativeRotation(FRotator::ZeroRotator);
		StaticMeshComponent->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
		// Центр вращения — центр bounding box (в локальном пространстве компонента)
		StaticMeshComponent->UpdateBounds();
		FBoxSphereBounds WorldBounds = StaticMeshComponent->CalcBounds(StaticMeshComponent->GetComponentTransform());
		FVector LocalCenter = StaticMeshComponent->GetComponentTransform().Inverse().TransformPosition(WorldBounds.Origin);
		const bool bCenterFinite = LocalCenter.X == LocalCenter.X && LocalCenter.Y == LocalCenter.Y && LocalCenter.Z == LocalCenter.Z;
		const bool bCenterReasonable = LocalCenter.SizeSquared() < FMath::Square(10000.0f);
		if (bCenterFinite && bCenterReasonable)
		{
			StaticMeshComponent->SetRelativeLocation(-LocalCenter);
		}
		else
		{
			// UE57 migration note: guard against invalid local center values causing off-screen preview.
			StaticMeshComponent->SetRelativeLocation(FVector::ZeroVector);
		}
		
		// Регистрируем компонент в мире
		if (StaticMeshComponent->IsRegistered())
		{
			StaticMeshComponent->ReregisterComponent();
		}
		else
		{
			StaticMeshComponent->RegisterComponent();
		}
		
		StaticMeshComponent->SetRenderCustomDepth(false);
		StaticMeshComponent->SetCastShadow(true);
		StaticMeshComponent->MarkRenderStateDirty();

		RefreshCaptureShowOnly();
		
	}
	else
	{
		SkeletalMeshComponent->SetSkeletalMesh(nullptr);
		SkeletalMeshComponent->SetVisibility(false);
		SkeletalMeshComponent->SetHiddenInGame(true);
		StaticMeshComponent->SetStaticMesh(nullptr);
		StaticMeshComponent->SetVisibility(false);
		StaticMeshComponent->SetHiddenInGame(true);
		MeshTrianglesLocal.Empty();
	}
	ApplyRotation();
	FrameCaptureToActiveMesh();
	// Кэш треугольников отложен на следующий Tick, чтобы не тормозить открытие окна
	UPrimitiveComponent* ActiveMesh = nullptr;
	if (SkeletalMeshComponent && SkeletalMeshComponent->GetSkeletalMeshAsset() && SkeletalMeshComponent->IsVisible())
		ActiveMesh = SkeletalMeshComponent;
	else if (StaticMeshComponent && StaticMeshComponent->GetStaticMesh() && StaticMeshComponent->IsVisible())
		ActiveMesh = StaticMeshComponent;
	if (ActiveMesh)
	{
		bPendingMeshTriangleCache = true;
		CachedGridLines.Empty(); // пересчёт сетки после построения кэша треугольников
	}
}

void AArmorModPreviewActor::ApplyRotation()
{
	if (MeshHolder)
	{
		MeshHolder->SetRelativeRotation(FRotator(PreviewPitchDegrees, PreviewYawDegrees, PreviewRollDegrees));
	}
}

void AArmorModPreviewActor::FrameCaptureToActiveMesh()
{
	if (!CaptureComponent)
	{
		return;
	}

	UPrimitiveComponent* ActiveMesh = nullptr;
	if (SkeletalMeshComponent && SkeletalMeshComponent->GetSkeletalMeshAsset() && SkeletalMeshComponent->IsVisible())
	{
		ActiveMesh = SkeletalMeshComponent;
	}
	else if (StaticMeshComponent && StaticMeshComponent->GetStaticMesh() && StaticMeshComponent->IsVisible())
	{
		ActiveMesh = StaticMeshComponent;
	}
	if (!ActiveMesh)
	{
		return;
	}

	ActiveMesh->UpdateBounds();
	const FBoxSphereBounds Bounds = ActiveMesh->CalcBounds(ActiveMesh->GetComponentTransform());
	const float Radius = FMath::Max(20.0f, Bounds.SphereRadius);

	// UE57 migration note: авто-фрейминг камеры как compatibility shim для разных pivot/bounds после миграции.
	CameraDistance = FMath::Clamp(Radius * 1.5f, 90.0f, 500.0f);
	CameraPanOffset = FVector::ZeroVector;
	ApplyCameraPan();

	const FVector CamLoc = CaptureComponent->GetComponentLocation();
	CaptureComponent->SetWorldRotation((Bounds.Origin - CamLoc).Rotation());
}

void AArmorModPreviewActor::RefreshCaptureShowOnly()
{
	if (!CaptureComponent)
	{
		return;
	}

	// UE57 migration note: возвращаем стандартный scene-primitives pipeline,
	// чтобы исключить артефакты ShowOnly в UI-превью.
	CaptureComponent->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
	CaptureComponent->ShowOnlyComponents.Empty();
}

void AArmorModPreviewActor::AddPanDelta(float DeltaRight, float DeltaUp, float DeltaForward)
{
	// В пространстве актора: камера смотрит по -X, вправо = +Y, вверх = +Z, вперёд = -X
	CameraPanOffset.Y += DeltaRight;
	CameraPanOffset.Z += DeltaUp;
	CameraPanOffset.X -= DeltaForward;
	ApplyCameraPan();
}

void AArmorModPreviewActor::AddZoomDelta(float Delta)
{
	// Положительный Delta (колесо вверх) — приближение, уменьшаем дистанцию
	CameraDistance = FMath::Clamp(CameraDistance - Delta, 50.f, 600.f);
	ApplyCameraPan();
}

void AArmorModPreviewActor::ApplyCameraPan()
{
	if (CaptureComponent)
	{
		// X — дистанция (зум), Y/Z — панорамирование
		FVector Loc(CameraDistance + CameraPanOffset.X, CameraPanOffset.Y, CameraPanOffset.Z);
		CaptureComponent->SetRelativeLocation(Loc);
	}
}

void AArmorModPreviewActor::SetModGrids(const TArray<FArmorModSideGrid>& Grids)
{
	ModGrids = Grids;
	CachedGridLines.Empty(); // при смене сеток кэш недействителен
	if (ModGrids.Num() == 0)
	{
		BuildDefaultGridFromMeshBounds();
	}
}

void AArmorModPreviewActor::SetInstalledMods(UEquippableItemData* ArmorItemData)
{
	USceneComponent* ParentMesh = nullptr;
	if (SkeletalMeshComponent && SkeletalMeshComponent->GetSkeletalMeshAsset() && SkeletalMeshComponent->IsVisible())
		ParentMesh = SkeletalMeshComponent;
	else if (StaticMeshComponent && StaticMeshComponent->GetStaticMesh() && StaticMeshComponent->IsVisible())
		ParentMesh = StaticMeshComponent;
	if (!ParentMesh || !ArmorItemData || ArmorItemData->ModSideGrids.Num() == 0) return;

	// Размер ячейки 5 см — как в EquipmentComponent и ArmorBase::RebuildModSideGrids (координаты сетки в локальном пространстве меша)
	const float CS = ModGridCellWorldSize;

	TArray<USceneComponent*> ToRemove;
	for (USceneComponent* Child : ParentMesh->GetAttachChildren())
		if (Child && Child->GetName().StartsWith(TEXT("PreviewMod_")))
			ToRemove.Add(Child);
	for (USceneComponent* C : ToRemove)
		C->DestroyComponent();

	for (int32 i = 0; i < ArmorItemData->InstalledMods.Num(); ++i)
	{
		const FInstalledArmorMod& Inst = ArmorItemData->InstalledMods[i];
		if (!Inst.ModItemData || Inst.SideIndex < 0 || Inst.SideIndex >= ArmorItemData->ModSideGrids.Num()) continue;

		UObject* ModMeshObj = Inst.ModItemData->WorldMesh;
		if (!ModMeshObj || !ModMeshObj->IsValidLowLevel()) continue;

		const FArmorModSideGrid& G = ArmorItemData->ModSideGrids[Inst.SideIndex];
		const FIntPoint Foot = UEquippableItemData::GetModFootprint(Inst.ModItemData);
		const int32 SizeX = FMath::Max(1, Foot.X);
		const int32 SizeY = FMath::Max(1, Foot.Y);
		// Точка привязки — геометрический центр области, в которую помещён мод (как в EquipmentComponent)
		const float GridCenterX = (Inst.CellX + SizeX * 0.5f) * CS;
		const float GridCenterY = (Inst.CellY + SizeY * 0.5f) * CS;
		const FVector GridCenterPos = G.Origin + GridCenterX * G.AxisRight + GridCenterY * G.AxisUp;
		const FVector Normal = FVector::CrossProduct(G.AxisRight, G.AxisUp).GetSafeNormal();

		FVector LocalPos;
		if (const UEquipModBaseItemData* ModData = Cast<UEquipModBaseItemData>(Inst.ModItemData))
		{
			if (!ModData->AttachmentGridOrigin.IsNearlyZero())
			{
				const FVector OriginOffset = ModData->AttachmentGridOrigin.X * G.AxisRight + ModData->AttachmentGridOrigin.Y * G.AxisUp + ModData->AttachmentGridOrigin.Z * Normal;
				LocalPos = GridCenterPos - OriginOffset;
			}
			else
			{
				// Пивот меша в центре: ставим компонент в центр области (как в EquipmentComponent)
				LocalPos = GridCenterPos;
			}
		}
		else
		{
			LocalPos = GridCenterPos;
		}

		FMatrix RotMat(G.AxisRight, G.AxisUp, Normal, FVector::ZeroVector);
		FRotator LocalRot = RotMat.Rotator();
		FTransform LocalTM(LocalRot, LocalPos, FVector(1.f, 1.f, 1.f));

		USceneComponent* ModComp = nullptr;
		if (USkeletalMesh* Sk = Cast<USkeletalMesh>(ModMeshObj))
		{
			USkeletalMeshComponent* SkComp = NewObject<USkeletalMeshComponent>(this, FName(*FString::Printf(TEXT("PreviewMod_%d"), i)));
			if (SkComp) { SkComp->SetSkeletalMesh(Sk); ModComp = SkComp; }
		}
		else if (UStaticMesh* St = Cast<UStaticMesh>(ModMeshObj))
		{
			UStaticMeshComponent* StComp = NewObject<UStaticMeshComponent>(this, FName(*FString::Printf(TEXT("PreviewMod_%d"), i)));
			if (StComp) { StComp->SetStaticMesh(St); ModComp = StComp; }
		}
		if (!ModComp) continue;

		ModComp->SetupAttachment(ParentMesh);
		ModComp->SetRelativeTransform(LocalTM);
		ModComp->RegisterComponent();
		if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(ModComp))
		{
			Prim->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Prim->SetCastShadow(true);
		}
	}

	RefreshCaptureShowOnly();
}

void AArmorModPreviewActor::BuildDefaultGridFromMeshBounds()
{
	UPrimitiveComponent* MeshComp = nullptr;
	if (SkeletalMeshComponent && SkeletalMeshComponent->GetSkeletalMeshAsset())
		MeshComp = SkeletalMeshComponent;
	else if (StaticMeshComponent && StaticMeshComponent->GetStaticMesh())
		MeshComp = StaticMeshComponent;
	if (!MeshComp) return;

	FBoxSphereBounds Bounds = MeshComp->GetLocalBounds();
	FVector LocalMin = Bounds.Origin - Bounds.BoxExtent;
	FVector LocalMax = Bounds.Origin + Bounds.BoxExtent;
	const float CS = ModGridCellWorldSize;
	// Перед = грань с макс X (как в ArmorBase)
	const float WidthY = LocalMax.Y - LocalMin.Y;
	const float HeightZ = LocalMax.Z - LocalMin.Z;
	int32 CX = FMath::Clamp(FMath::FloorToInt(WidthY / CS), 1, 8);
	int32 CY = FMath::Clamp(FMath::FloorToInt(HeightZ / CS), 1, 6);

	FArmorModSideGrid G;
	G.SideName = FName(TEXT("Front"));
	G.CellsX = CX;
	G.CellsY = CY;
	G.Origin = FVector(LocalMax.X, LocalMin.Y, LocalMin.Z);
	G.AxisRight = FVector(0.f, 1.f, 0.f);
	G.AxisUp = FVector(0.f, 0.f, 1.f);
	ModGrids.Add(G);
}

void AArmorModPreviewActor::BuildMeshTriangleCache(UPrimitiveComponent* MeshComp)
{
	MeshTrianglesLocal.Empty();
	MeshTriangleBoundsLocal.Empty();
	if (!MeshComp) return;

	// ВАЖНО: кэш нужен только для "прилипания" сетки к реально изогнутой поверхности.
	// Полный проход по всем треугольникам hi‑poly меша может занимать заметное время
	// при первом открытии окна, поэтому жёстко ограничиваем количество обрабатываемых
	// треугольников. Этого достаточно для визуального качества, но избегает фризов.
	const int32 MaxCachedTriangles = 15000; // 5k–15k треугольников достаточно для превью

	auto AddTriangle = [this](const FVector& V0, const FVector& V1, const FVector& V2)
	{
		// Не выходим внутри AddTriangle, чтобы цикл мог корректно завершиться по счётчику
		MeshTrianglesLocal.Add(V0);
		MeshTrianglesLocal.Add(V1);
		MeshTrianglesLocal.Add(V2);
		FVector MinV = V0.ComponentMin(V1).ComponentMin(V2);
		FVector MaxV = V0.ComponentMax(V1).ComponentMax(V2);
		MeshTriangleBoundsLocal.Add(FBox(MinV, MaxV));
	};

	UStaticMeshComponent* SMComp = Cast<UStaticMeshComponent>(MeshComp);
	if (SMComp && SMComp->GetStaticMesh())
	{
		UStaticMesh* SM = SMComp->GetStaticMesh();
		const int32 LODIndex = 0;
		int32 TotalTris = 0;
		for (int32 SectionIndex = 0; SectionIndex < 64; ++SectionIndex)
		{
			TArray<FVector> Vertices;
			TArray<int32> Triangles;
			TArray<FVector> Normals;
			TArray<FVector2D> UVs;
			TArray<FProcMeshTangent> Tangents;
			UKismetProceduralMeshLibrary::GetSectionFromStaticMesh(SM, LODIndex, SectionIndex, Vertices, Triangles, Normals, UVs, Tangents);
			if (Triangles.Num() < 3) break;
			for (int32 i = 0; i + 2 < Triangles.Num(); i += 3)
			{
				if (TotalTris >= MaxCachedTriangles)
				{
					return;
				}
				AddTriangle(Vertices[Triangles[i]], Vertices[Triangles[i + 1]], Vertices[Triangles[i + 2]]);
				++TotalTris;
			}
		}
		return;
	}

	USkeletalMeshComponent* SkComp = Cast<USkeletalMeshComponent>(MeshComp);
	if (SkComp && SkComp->GetSkeletalMeshAsset())
	{
		USkeletalMesh* SkMesh = SkComp->GetSkeletalMeshAsset();
		FSkeletalMeshRenderData* RenderData = SkMesh->GetResourceForRendering();
		if (RenderData && RenderData->LODRenderData.Num() > 0)
		{
			FSkeletalMeshLODRenderData& LOD0 = RenderData->LODRenderData[0];
			TArray<uint32> Indices;
			LOD0.MultiSizeIndexContainer.GetIndexBuffer(Indices);
			const FSkinWeightVertexBuffer& SkinWeightBuffer = LOD0.SkinWeightVertexBuffer;
			int32 TotalTris = 0;
			for (const FSkelMeshRenderSection& Section : LOD0.RenderSections)
			{
				if (Section.bDisabled || Section.NumTriangles == 0) continue;
				const uint32 BaseIdx = Section.BaseIndex;
				const uint32 NumTris = Section.NumTriangles;
				for (uint32 t = 0; t < NumTris; ++t)
				{
					if (TotalTris >= MaxCachedTriangles)
					{
						return;
					}
					const uint32 i0 = Indices[BaseIdx + t * 3 + 0];
					const uint32 i1 = Indices[BaseIdx + t * 3 + 1];
					const uint32 i2 = Indices[BaseIdx + t * 3 + 2];
					FVector V0 = GetSkeletalMeshRefVertLocation(SkMesh, LOD0, SkinWeightBuffer, (int32)i0);
					FVector V1 = GetSkeletalMeshRefVertLocation(SkMesh, LOD0, SkinWeightBuffer, (int32)i1);
					FVector V2 = GetSkeletalMeshRefVertLocation(SkMesh, LOD0, SkinWeightBuffer, (int32)i2);
					AddTriangle(V0, V1, V2);
					++TotalTris;
				}
			}
		}
	}
}

/** Быстрая проверка: пересекает ли отрезок лужа AABB (отсечение перед ray-triangle). */
static bool RaySegmentIntersectsAABB(const FVector& RayOrigin, const FVector& RayDir, float RayLen, const FBox& Box)
{
	const FVector& Min = Box.Min;
	const FVector& Max = Box.Max;
	float TMin = 0.f, TMax = RayLen;
	const float Eps = 1e-6f;
	for (int32 d = 0; d < 3; ++d)
	{
		float O = d == 0 ? RayOrigin.X : (d == 1 ? RayOrigin.Y : RayOrigin.Z);
		float D = d == 0 ? RayDir.X : (d == 1 ? RayDir.Y : RayDir.Z);
		float Lo = d == 0 ? Min.X : (d == 1 ? Min.Y : Min.Z);
		float Hi = d == 0 ? Max.X : (d == 1 ? Max.Y : Max.Z);
		if (FMath::Abs(D) < Eps) { if (O < Lo || O > Hi) return false; continue; }
		float T0 = (Lo - O) / D, T1 = (Hi - O) / D;
		if (T0 > T1) { float Tmp = T0; T0 = T1; T1 = Tmp; }
		TMin = FMath::Max(TMin, T0);
		TMax = FMath::Min(TMax, T1);
		if (TMin > TMax) return false;
	}
	return true;
}

static bool RayTriangleIntersect(const FVector& RayOrigin, const FVector& RayDir, const FVector& V0, const FVector& V1, const FVector& V2, float& OutT)
{
	const FVector Edge1 = V1 - V0;
	const FVector Edge2 = V2 - V0;
	FVector H = FVector::CrossProduct(RayDir, Edge2);
	float A = FVector::DotProduct(Edge1, H);
	const float Epsilon = 1e-6f;
	if (FMath::Abs(A) < Epsilon) return false;
	float F = 1.f / A;
	FVector S = RayOrigin - V0;
	float U = F * FVector::DotProduct(S, H);
	if (U < 0.f || U > 1.f) return false;
	FVector Q = FVector::CrossProduct(S, Edge1);
	float V = F * FVector::DotProduct(RayDir, Q);
	if (V < 0.f || U + V > 1.f) return false;
	OutT = F * FVector::DotProduct(Edge2, Q);
	return OutT >= 0.f;
}

FVector AArmorModPreviewActor::ProjectPointOntoMesh(UPrimitiveComponent* MeshComp, const FTransform& MeshToWorld, const FVector& WorldPoint, const FVector& WorldPlaneNormal) const
{
	if (!MeshComp) return WorldPoint;

	FVector Dir = WorldPlaneNormal.GetSafeNormal();
	if (Dir.IsNearlyZero()) return WorldPoint;

	// Проекция по видимой геометрии (треугольники), не по коллизии
	if (MeshTrianglesLocal.Num() >= 3)
	{
		const FVector RayStart = WorldPoint + Dir * ModGridTraceDistance;
		const FVector RayEnd = WorldPoint - Dir * ModGridTraceDistance;
		const FVector RayOrigin = RayStart;
		const FVector RayDir = (RayEnd - RayStart).GetSafeNormal();
		const float RayLen = (RayEnd - RayStart).Size();
		const FTransform WorldToMesh = MeshToWorld.Inverse();
		const FVector LocalOrigin = WorldToMesh.TransformPosition(RayOrigin);
		const FVector LocalDir = WorldToMesh.TransformVector(RayDir).GetSafeNormal();

		float BestT = RayLen + 1.f;
		const int32 NumTris = MeshTriangleBoundsLocal.Num();
		for (int32 ti = 0; ti < NumTris; ++ti)
		{
			if (!RaySegmentIntersectsAABB(LocalOrigin, LocalDir, RayLen, MeshTriangleBoundsLocal[ti]))
				continue;
			const int32 i = ti * 3;
			float T;
			if (RayTriangleIntersect(LocalOrigin, LocalDir, MeshTrianglesLocal[i], MeshTrianglesLocal[i + 1], MeshTrianglesLocal[i + 2], T) && T < BestT && T >= 0.f)
				BestT = T;
		}
		if (BestT <= RayLen)
			return MeshToWorld.TransformPosition(LocalOrigin + LocalDir * BestT);
	}
	return WorldPoint;
}

void AArmorModPreviewActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Раньше здесь однократно строился кэш треугольников меша (BuildMeshTriangleCache),
	// что могло приводить к заметной "задумчивости" при первом открытии окна
	// на сложных hi‑poly мешах. Для превью достаточно плоской сетки без точного
	// прилипания к геометрии, поэтому отложенное построение кэша отключено.
	if (bPendingMeshTriangleCache)
	{
		bPendingMeshTriangleCache = false;
	}

	if (ModGrids.Num() == 0) return;

	UPrimitiveComponent* MeshComp = nullptr;
	if (SkeletalMeshComponent && SkeletalMeshComponent->GetSkeletalMeshAsset() && SkeletalMeshComponent->IsVisible())
		MeshComp = SkeletalMeshComponent;
	else if (StaticMeshComponent && StaticMeshComponent->GetStaticMesh() && StaticMeshComponent->IsVisible())
		MeshComp = StaticMeshComponent;
	if (!MeshComp || !GetWorld()) return;

	const FTransform MeshToWorld = MeshComp->GetComponentTransform();

	// Кэш: пересчитываем сетку только при изменении трансформа меша (поворот/зум/панорама)
	if (CachedGridLines.Num() > 0 && CachedGridMeshTransform.Equals(MeshToWorld, 1e-4f))
	{
		if (GridLineBatchComponent)
		{
			GridLineBatchComponent->Flush();
			GridLineBatchComponent->DrawLines(CachedGridLines);
		}
		// Подсветка ячеек (ниже общий блок)
	}
	else
	{
		const FLinearColor GridColor(0.4f, 1.f, 0.47f, 1.f);
		const float Thickness = 0.4f;
		TArray<FBatchedLine> BatchedLines;

		// Проецируем только узлы сетки (без разбиения рёбер) — в разы меньше вызовов ProjectPointOntoMesh при повороте
		for (const FArmorModSideGrid& G : ModGrids)
		{
			if (G.CellsX <= 0 || G.CellsY <= 0) continue;
			const float CS = ModGridCellWorldSize;
			const int32 NX = G.CellsX + 1;
			const int32 NY = G.CellsY + 1;

			FVector LocalNormal = FVector::CrossProduct(G.AxisRight, G.AxisUp).GetSafeNormal();
			FVector WorldNormal = MeshToWorld.TransformVector(LocalNormal).GetSafeNormal();

			// 1) Спроецировать только вершины сетки (NX*NY точек вместо тысяч при разбиении рёбер)
			TArray<FVector> Nodes;
			Nodes.SetNumUninitialized(NX * NY);
			for (int32 j = 0; j < NY; ++j)
				for (int32 i = 0; i < NX; ++i)
				{
					FVector LocalP = G.Origin + (float)i * CS * G.AxisRight + (float)j * CS * G.AxisUp;
					Nodes[i + j * NX] = ProjectPointOntoMesh(MeshComp, MeshToWorld, MeshToWorld.TransformPosition(LocalP), WorldNormal);
				}

			// 2) Линии между соседними узлами (без разбиения — быстро)
			auto AddLine = [&](const FVector& A, const FVector& B)
			{
				FBatchedLine& Line = BatchedLines.AddDefaulted_GetRef();
				Line.Start = A;
				Line.End = B;
				Line.Color = GridColor;
				Line.Thickness = Thickness;
				Line.DepthPriority = SDPG_World;
			};
			for (int32 i = 0; i < NX; ++i)
				for (int32 j = 0; j < G.CellsY; ++j)
					AddLine(Nodes[i + j * NX], Nodes[i + (j + 1) * NX]);
			for (int32 j = 0; j < NY; ++j)
				for (int32 i = 0; i < G.CellsX; ++i)
					AddLine(Nodes[i + j * NX], Nodes[(i + 1) + j * NX]);
		}

		if (GridLineBatchComponent && BatchedLines.Num() > 0)
		{
			CachedGridLines = MoveTemp(BatchedLines);
			CachedGridMeshTransform = MeshToWorld;
			GridLineBatchComponent->Flush();
			GridLineBatchComponent->DrawLines(CachedGridLines);
		}
	}

	// Подсветка ячеек (зелёная/красная) — всегда при наличии сетки
	const float HighlightThickness = 1.2f;
	for (const FHighlightCell& H : HighlightCells)
	{
		if (H.SideIndex < 0 || H.SideIndex >= ModGrids.Num()) continue;
		const FArmorModSideGrid& G = ModGrids[H.SideIndex];
		if (G.CellsX <= 0 || G.CellsY <= 0) continue;
		if (H.CellX < 0 || H.CellX >= G.CellsX || H.CellY < 0 || H.CellY >= G.CellsY) continue;
		const float CS = ModGridCellWorldSize;
		const FVector LocalOrigin = G.Origin + (float)H.CellX * CS * G.AxisRight + (float)H.CellY * CS * G.AxisUp;
		const FVector P0 = MeshToWorld.TransformPosition(LocalOrigin);
		const FVector P1 = MeshToWorld.TransformPosition(LocalOrigin + CS * G.AxisRight);
		const FVector P2 = MeshToWorld.TransformPosition(LocalOrigin + CS * G.AxisRight + CS * G.AxisUp);
		const FVector P3 = MeshToWorld.TransformPosition(LocalOrigin + CS * G.AxisUp);
		const FLinearColor Color = H.bValid ? FLinearColor(0.2f, 1.f, 0.3f, 1.f) : FLinearColor(1.f, 0.2f, 0.2f, 1.f);
		ULineBatchComponent* LineBatch = GridLineBatchComponent;
		auto AddHL = [LineBatch, Color, HighlightThickness](const FVector& A, const FVector& B)
		{
			if (!LineBatch) return;
			FBatchedLine Line;
			Line.Start = A; Line.End = B; Line.Color = Color; Line.Thickness = HighlightThickness; Line.DepthPriority = SDPG_World;
			TArray<FBatchedLine> One; One.Add(Line);
			LineBatch->DrawLines(One);
		};
		AddHL(P0, P1); AddHL(P1, P2); AddHL(P2, P3); AddHL(P3, P0);
	}
}

bool AArmorModPreviewActor::HitTestGridFromWidgetPosition(FVector2D LocalPosInWidget, FVector2D WidgetSize, int32& OutSideIndex, int32& OutCellX, int32& OutCellY) const
{
	OutSideIndex = 0; OutCellX = 0; OutCellY = 0;
	if (!CaptureComponent || ModGrids.Num() == 0 || WidgetSize.X < 1.f || WidgetSize.Y < 1.f) return false;

	const float nx = (LocalPosInWidget.X / WidgetSize.X) * 2.f - 1.f;
	const float ny = 1.f - (LocalPosInWidget.Y / WidgetSize.Y) * 2.f;
	const float FOVRad = FMath::DegreesToRadians(CaptureComponent->FOVAngle * 0.5f);
	const float tanVert = FMath::Tan(FOVRad);
	const float aspect = WidgetSize.X / WidgetSize.Y;
	const float tanHoriz = tanVert * aspect;

	FVector WorldRayOrigin = CaptureComponent->GetComponentLocation();
	FVector WorldRayDir = (CaptureComponent->GetForwardVector() + (nx * tanHoriz) * CaptureComponent->GetRightVector() + (ny * tanVert) * CaptureComponent->GetUpVector()).GetSafeNormal();

	UPrimitiveComponent* MeshComp = nullptr;
	if (SkeletalMeshComponent && SkeletalMeshComponent->GetSkeletalMeshAsset() && SkeletalMeshComponent->IsVisible())
		MeshComp = SkeletalMeshComponent;
	else if (StaticMeshComponent && StaticMeshComponent->GetStaticMesh() && StaticMeshComponent->IsVisible())
		MeshComp = StaticMeshComponent;
	if (!MeshComp) return false;

	const FTransform MeshToWorld = MeshComp->GetComponentTransform();
	int32 BestSide = INDEX_NONE;
	int32 BestCX = -1, BestCY = -1;
	float BestT = 1e30f;

	for (int32 g = 0; g < ModGrids.Num(); ++g)
	{
		const FArmorModSideGrid& G = ModGrids[g];
		if (G.CellsX <= 0 || G.CellsY <= 0) continue;
		FVector WorldOrigin = MeshToWorld.TransformPosition(G.Origin);
		FVector WorldNormal = MeshToWorld.TransformVector(FVector::CrossProduct(G.AxisRight, G.AxisUp)).GetSafeNormal();
		float Denom = FVector::DotProduct(WorldNormal, WorldRayDir);
		if (FMath::Abs(Denom) < 1e-5f) continue;
		float T = FVector::DotProduct(WorldOrigin - WorldRayOrigin, WorldNormal) / Denom;
		if (T < 0.f) continue;
		FVector HitWorld = WorldRayOrigin + WorldRayDir * T;
		FVector LocalHit = MeshToWorld.Inverse().TransformPosition(HitWorld);
		FVector Delta = LocalHit - G.Origin;
		const float u = FVector::DotProduct(Delta, G.AxisRight);
		const float v = FVector::DotProduct(Delta, G.AxisUp);
		const float CS = ModGridCellWorldSize;
		const float LenRight = (float)(G.CellsX) * CS;
		const float LenUp = (float)(G.CellsY) * CS;
		const float Margin = 2.f;
		if (u >= -Margin && u <= LenRight + Margin && v >= -Margin && v <= LenUp + Margin && T < BestT)
		{
			int32 CX = (int32)(u / CS);
			int32 CY = (int32)(v / CS);
			if (u < 0.f) CX = 0; else if (CX >= G.CellsX) CX = G.CellsX - 1;
			if (v < 0.f) CY = 0; else if (CY >= G.CellsY) CY = G.CellsY - 1;
			BestT = T;
			BestSide = g;
			BestCX = CX;
			BestCY = CY;
		}
	}
	if (BestSide == INDEX_NONE) return false;
	OutSideIndex = BestSide;
	OutCellX = BestCX;
	OutCellY = BestCY;
	return true;
}

void AArmorModPreviewActor::SetHighlightCells(const TArray<FHighlightCell>& Cells)
{
	HighlightCells = Cells;
}

void AArmorModPreviewActor::SetRenderTarget(UTextureRenderTarget2D* RT)
{
	if (CaptureComponent && RT)
	{
		CaptureComponent->TextureTarget = RT;
		RefreshCaptureShowOnly();
		FrameCaptureToActiveMesh();
		
		// Убеждаемся, что актор и компоненты зарегистрированы в мире
		if (!IsActorInitialized())
		{
			RegisterAllComponents();
		}
		
		// Убеждаемся, что CaptureComponent зарегистрирован
		if (!CaptureComponent->IsRegistered())
		{
			CaptureComponent->RegisterComponent();
		}
		
		// Принудительно обновляем RT
		RT->UpdateResourceImmediate(true);
		
		// Не трогаем RelativeLocation меша здесь:
		// SetMeshFromEquipped() уже выставляет корректный pivot/центр.
		// Принудительный reset в (0,0,0) может увести модель из кадра и дать "чёрное" превью.
		if (SkeletalMeshComponent && SkeletalMeshComponent->GetSkeletalMeshAsset())
		{
			SkeletalMeshComponent->UpdateBounds();
		}
		else if (StaticMeshComponent && StaticMeshComponent->GetStaticMesh())
		{
			StaticMeshComponent->UpdateBounds();
		}
		
		// Принудительно обновляем захват сразу и через несколько кадров
		if (UWorld* World = GetWorld())
		{
			// Первый захват сразу
			CaptureComponent->CaptureScene();
			
			// Затем через несколько кадров для гарантии
			World->GetTimerManager().SetTimerForNextTick([this, World]()
			{
				if (CaptureComponent && CaptureComponent->TextureTarget)
				{
					CaptureComponent->CaptureScene();
					
					World->GetTimerManager().SetTimerForNextTick([this]()
					{
						if (CaptureComponent && CaptureComponent->TextureTarget)
						{
							CaptureComponent->CaptureScene();
							
						}
					});
				}
			});
		}
		else
		{
			// Если мира нет, вызываем сразу
			CaptureComponent->CaptureScene();
		}
	}
}
