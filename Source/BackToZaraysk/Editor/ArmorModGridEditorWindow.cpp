// Copyright Epic Games, Inc. All Rights Reserved.

// UE57 migration note: this editor-only tool lives in runtime module source tree,
// so we hard-guard compilation for non-editor targets.
#if WITH_EDITOR
#include "BackToZaraysk/Editor/ArmorModGridEditorWindow.h"
#include "BackToZaraysk/GameData/Items/ArmorModPreviewActor.h"
#include "BackToZaraysk/Inventory/EquippableItemData.h"
#include "EditorViewportClient.h"
#include "CanvasItem.h"
#include "CanvasTypes.h"
#include "SceneView.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Framework/Application/SlateApplication.h"
#include "UnrealClient.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"

#define LOCTEXT_NAMESPACE "ArmorModGridEditor"

static constexpr float ModGridCellWorldSize = 5.0f;

namespace
{
static UObject* ResolveArmorPreviewMesh(AEquipmentBase* InArmor)
{
	if (!InArmor)
	{
		return nullptr;
	}

	// 1) Preferred source for equipped preview.
	if (UEquippableItemData* Eq = Cast<UEquippableItemData>(InArmor->ItemInstance))
	{
		if (Eq->EquippedMesh)
		{
			return Eq->EquippedMesh;
		}
		if (Eq->WorldMesh)
		{
			return Eq->WorldMesh;
		}
	}

	// 2) Generic item visual.
	if (InArmor->ItemInstance && InArmor->ItemInstance->WorldMesh)
	{
		return InArmor->ItemInstance->WorldMesh;
	}

	// 3) Runtime components on actor instance.
	if (InArmor->SkeletalMesh && InArmor->SkeletalMesh->GetSkeletalMeshAsset())
	{
		return InArmor->SkeletalMesh->GetSkeletalMeshAsset();
	}
	if (InArmor->Mesh && InArmor->Mesh->GetStaticMesh())
	{
		return InArmor->Mesh->GetStaticMesh();
	}

	// 4) Never leave viewport completely empty.
	return LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
}
}

// ---- FArmorModGridEditorWindow ----

void FArmorModGridEditorWindow::OpenWindow(AEquipmentBase* Armor)
{
	if (!Armor || !Armor->bIsModdable)
	{
		return;
	}

	TSharedPtr<SArmorModGridEditorViewport> ViewportWidget;

	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(LOCTEXT("WindowTitle", "Armor Mod Grid Editor"))
		.ClientSize(FVector2D(900, 600))
		.SupportsMinimize(true)
		.SupportsMaximize(true)
		[
			SAssignNew(ViewportWidget, SArmorModGridEditorViewport)
			.Armor(Armor)
		];

	FSlateApplication::Get().AddWindow(Window);
}

// ---- FArmorModGridEditorViewportClient ----

FArmorModGridEditorViewportClient::FArmorModGridEditorViewportClient(FPreviewScene* InPreviewScene, TSharedPtr<SEditorViewport> InViewport, AEquipmentBase* InArmor)
	: FEditorViewportClient(nullptr, InPreviewScene, TWeakPtr<SEditorViewport>(InViewport))
	, ArmorPtr(InArmor)
	, PreviewScene(InPreviewScene)
{
	check(InPreviewScene && InArmor);

	SetViewportType(LVT_Perspective);
	SetViewModes(VMI_Lit, VMI_Lit);
	SetViewLocation(FVector(120.f, 0.f, 0.f));
	SetViewRotation(FRotator(0.f, 180.f, 0.f));
	bUsingOrbitCamera = true;

	// Ensure armor has up-to-date grids
	InArmor->RebuildModSideGrids();

	// Working copy: 4 sides
	WorkingOrigins.SetNum(4);
	WorkingOrigins[0] = InArmor->ModsFrontOrigin;
	WorkingOrigins[1] = InArmor->ModsBackOrigin;
	WorkingOrigins[2] = InArmor->ModsLeftOrigin;
	WorkingOrigins[3] = InArmor->ModsRightOrigin;
	WorkingRotations.SetNum(4);
	WorkingRotations[0] = InArmor->ModsFrontRotation;
	WorkingRotations[1] = InArmor->ModsBackRotation;
	WorkingRotations[2] = InArmor->ModsLeftRotation;
	WorkingRotations[3] = InArmor->ModsRightRotation;

	GridIndexToSideIndex.Empty();
	GridCells.Empty();
	if (InArmor->bModsFront) { GridIndexToSideIndex.Add(0); GridCells.Add(FIntPoint(InArmor->ModSideGrids[GridCells.Num()].CellsX, InArmor->ModSideGrids[GridCells.Num()].CellsY)); }
	if (InArmor->bModsBack)  { GridIndexToSideIndex.Add(1); GridCells.Add(FIntPoint(InArmor->ModSideGrids[GridCells.Num()].CellsX, InArmor->ModSideGrids[GridCells.Num()].CellsY)); }
	if (InArmor->bModsLeft)  { GridIndexToSideIndex.Add(2); GridCells.Add(FIntPoint(InArmor->ModSideGrids[GridCells.Num()].CellsX, InArmor->ModSideGrids[GridCells.Num()].CellsY)); }
	if (InArmor->bModsRight) { GridIndexToSideIndex.Add(3); GridCells.Add(FIntPoint(InArmor->ModSideGrids[GridCells.Num()].CellsX, InArmor->ModSideGrids[GridCells.Num()].CellsY)); }

	UWorld* World = InPreviewScene->GetWorld();
	if (!World) return;

	UObject* MeshObj = ResolveArmorPreviewMesh(InArmor);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AArmorModPreviewActor* Preview = World->SpawnActor<AArmorModPreviewActor>(SpawnParams);
	if (Preview)
	{
		PreviewActorPtr = Preview;
		Preview->SetMeshFromEquipped(MeshObj);
		SyncWorkingCopyToPreview();
	}
}

FArmorModGridEditorViewportClient::~FArmorModGridEditorViewportClient()
{
	if (AArmorModPreviewActor* Preview = PreviewActorPtr.Get())
	{
		Preview->Destroy();
	}
}

void FArmorModGridEditorViewportClient::Tick(float DeltaSeconds)
{
	FEditorViewportClient::Tick(DeltaSeconds);
	// Тикаем мир превью, чтобы AArmorModPreviewActor обновлял и отрисовывал сетку
	if (PreviewScene && DeltaSeconds > 0.f)
	{
		if (UWorld* World = PreviewScene->GetWorld())
		{
			World->Tick(ELevelTick::LEVELTICK_All, DeltaSeconds);
		}
	}
}

void FArmorModGridEditorViewportClient::BuildDisplayGridsFromWorkingCopy(TArray<FArmorModSideGrid>& OutGrids)
{
	OutGrids.Reset();
	AEquipmentBase* Armor = ArmorPtr.Get();
	if (!Armor || GridIndexToSideIndex.Num() != GridCells.Num()) return;

	int32 g = 0;
	auto AddFrontBack = [&](int32 Side, bool bFront, FVector DefRight)
	{
		if (g >= GridCells.Num()) return;
		FArmorModSideGrid G;
		G.SideName = bFront ? FName(TEXT("Front")) : FName(TEXT("Back"));
		G.CellsX = GridCells[g].X;
		G.CellsY = GridCells[g].Y;
		G.Origin = WorkingOrigins[Side];
		FVector DefUp(0.f, 0.f, 1.f);
		const FMatrix RotMat = FRotationMatrix(WorkingRotations[Side]);
		G.AxisRight = RotMat.TransformVector(DefRight).GetSafeNormal();
		G.AxisUp = RotMat.TransformVector(DefUp).GetSafeNormal();
		OutGrids.Add(G);
		g++;
	};
	auto AddLeftRight = [&](int32 Side, bool bLeft, FVector DefRight)
	{
		if (g >= GridCells.Num()) return;
		FArmorModSideGrid G;
		G.SideName = bLeft ? FName(TEXT("Left")) : FName(TEXT("Right"));
		G.CellsX = GridCells[g].X;
		G.CellsY = GridCells[g].Y;
		G.Origin = WorkingOrigins[Side];
		FVector DefUp(0.f, 0.f, 1.f);
		const FMatrix RotMat = FRotationMatrix(WorkingRotations[Side]);
		G.AxisRight = RotMat.TransformVector(DefRight).GetSafeNormal();
		G.AxisUp = RotMat.TransformVector(DefUp).GetSafeNormal();
		OutGrids.Add(G);
		g++;
	};

	if (Armor->bModsFront) AddFrontBack(0, true, FVector(0.f, 1.f, 0.f));
	if (Armor->bModsBack)  AddFrontBack(1, false, FVector(0.f, -1.f, 0.f));
	if (Armor->bModsLeft)  AddLeftRight(2, true, FVector(1.f, 0.f, 0.f));
	if (Armor->bModsRight) AddLeftRight(3, false, FVector(-1.f, 0.f, 0.f));
}

void FArmorModGridEditorViewportClient::SyncWorkingCopyToPreview()
{
	TArray<FArmorModSideGrid> Grids;
	BuildDisplayGridsFromWorkingCopy(Grids);
	if (AArmorModPreviewActor* Preview = PreviewActorPtr.Get())
	{
		Preview->SetModGrids(Grids);
	}
}

FTransform FArmorModGridEditorViewportClient::GetMeshWorldTransform() const
{
	AArmorModPreviewActor* Preview = PreviewActorPtr.Get();
	if (!Preview) return FTransform::Identity;
	USceneComponent* MeshComp = nullptr;
	if (Preview->SkeletalMeshComponent && Preview->SkeletalMeshComponent->GetSkeletalMeshAsset() && Preview->SkeletalMeshComponent->IsVisible())
		MeshComp = Preview->SkeletalMeshComponent;
	else if (Preview->StaticMeshComponent && Preview->StaticMeshComponent->GetStaticMesh() && Preview->StaticMeshComponent->IsVisible())
		MeshComp = Preview->StaticMeshComponent;
	return MeshComp ? MeshComp->GetComponentTransform() : Preview->GetActorTransform();
}

int32 FArmorModGridEditorViewportClient::HitTestGrid(const FVector& WorldRayOrigin, const FVector& WorldRayDir) const
{
	TArray<FArmorModSideGrid> Grids;
	const_cast<FArmorModGridEditorViewportClient*>(this)->BuildDisplayGridsFromWorkingCopy(Grids);
	const FTransform MeshToWorld = GetMeshWorldTransform();
	int32 BestGrid = INDEX_NONE;
	float BestT = 1e30f;

	for (int32 g = 0; g < Grids.Num(); ++g)
	{
		const FArmorModSideGrid& G = Grids[g];
		FVector WorldOrigin = MeshToWorld.TransformPosition(G.Origin);
		FVector WorldNormal = MeshToWorld.TransformVector(FVector::CrossProduct(G.AxisRight, G.AxisUp)).GetSafeNormal();
		float Denom = FVector::DotProduct(WorldNormal, WorldRayDir);
		if (FMath::Abs(Denom) < 1e-5f) continue;
		float T = FVector::DotProduct(WorldOrigin - WorldRayOrigin, WorldNormal) / Denom;
		if (T < 0.f) continue;
		FVector HitWorld = WorldRayOrigin + WorldRayDir * T;
		FVector LocalHit = MeshToWorld.Inverse().TransformPosition(HitWorld);
		FVector LocalOrigin = G.Origin;
		FVector LocalRight = G.AxisRight;
		FVector LocalUp = G.AxisUp;
		FVector Delta = LocalHit - LocalOrigin;
		float LenRight = (float)(G.CellsX) * ModGridCellWorldSize;
		float LenUp = (float)(G.CellsY) * ModGridCellWorldSize;
		float u = FVector::DotProduct(Delta, LocalRight);
		float v = FVector::DotProduct(Delta, LocalUp);
		// Запас по краям: сетка на меше кривая, плоскость плоская — даём большой допуск для выделения
		const float Margin = 15.f;
		if (u >= -Margin && u <= LenRight + Margin && v >= -Margin && v <= LenUp + Margin && T < BestT)
		{
			BestT = T;
			BestGrid = g;
		}
	}
	return BestGrid;
}

static float DistanceFromRayToSegment(const FVector& RayOrigin, const FVector& RayDir, const FVector& SegA, const FVector& SegB)
{
	FVector SegDir = (SegB - SegA).GetSafeNormal();
	FVector W0 = RayOrigin - SegA;
	float A = FVector::DotProduct(RayDir, RayDir);
	float B = FVector::DotProduct(RayDir, SegDir);
	float C = FVector::DotProduct(SegDir, SegDir);
	float D = FVector::DotProduct(RayDir, W0);
	float E = FVector::DotProduct(SegDir, W0);
	float Denom = A * C - B * B;
	if (FMath::Abs(Denom) < 1e-8f) return 1e30f;
	float s = (B * E - C * D) / Denom;
	float t = (A * E - B * D) / Denom;
	t = FMath::Clamp(t, 0.f, (SegB - SegA).Size());
	FVector Closest = SegA + SegDir * t;
	return (RayOrigin + RayDir * s - Closest).Size();
}

int32 FArmorModGridEditorViewportClient::HitTestAxis(const FVector& WorldRayOrigin, const FVector& WorldRayDir,
	const FVector& WorldOrigin, const FVector& WorldAxisX, const FVector& WorldAxisY, const FVector& WorldAxisZ) const
{
	float Len = AxisDrawLength;
	float dX = DistanceFromRayToSegment(WorldRayOrigin, WorldRayDir, WorldOrigin, WorldOrigin + WorldAxisX * Len);
	float dY = DistanceFromRayToSegment(WorldRayOrigin, WorldRayDir, WorldOrigin, WorldOrigin + WorldAxisY * Len);
	float dZ = DistanceFromRayToSegment(WorldRayOrigin, WorldRayDir, WorldOrigin, WorldOrigin + WorldAxisZ * Len);
	int32 Best = -1;
	float BestD = AxisHitThreshold;
	if (dX < BestD) { BestD = dX; Best = 0; }
	if (dY < BestD) { BestD = dY; Best = 1; }
	if (dZ < BestD) { BestD = dZ; Best = 2; }
	return Best;
}

void FArmorModGridEditorViewportClient::DrawSelectedGridAxes(FViewport* InViewport, FCanvas* Canvas)
{
	if (SelectedGridIndex < 0 || SelectedGridIndex >= GridIndexToSideIndex.Num()) return;
	TArray<FArmorModSideGrid> Grids;
	BuildDisplayGridsFromWorkingCopy(Grids);
	if (SelectedGridIndex >= Grids.Num()) return;

	const FArmorModSideGrid& G = Grids[SelectedGridIndex];
	FTransform MeshToWorld = GetMeshWorldTransform();
	FVector WorldOrigin = MeshToWorld.TransformPosition(G.Origin);
	FVector WorldRight = MeshToWorld.TransformVector(G.AxisRight).GetSafeNormal();
	FVector WorldUp = MeshToWorld.TransformVector(G.AxisUp).GetSafeNormal();
	FVector WorldNormal = FVector::CrossProduct(WorldRight, WorldUp).GetSafeNormal();

	FSceneView* View = (FSceneView*)ViewState.GetReference();
	if (!View) return;

	auto WorldToScreen = [View](const FVector& W) -> FVector2D
	{
		FVector2D P;
		View->WorldToPixel(W, P);
		return P;
	};

	FVector2D O = WorldToScreen(WorldOrigin);
	FVector2D EndX = WorldToScreen(WorldOrigin + WorldRight * AxisDrawLength);
	FVector2D EndY = WorldToScreen(WorldOrigin + WorldUp * AxisDrawLength);
	FVector2D EndZ = WorldToScreen(WorldOrigin + WorldNormal * AxisDrawLength);

	FCanvasLineItem LineX(O, EndX); LineX.SetColor(FLinearColor::Red); LineX.LineThickness = 2.f; Canvas->DrawItem(LineX);
	FCanvasLineItem LineY(O, EndY); LineY.SetColor(FLinearColor::Green); LineY.LineThickness = 2.f; Canvas->DrawItem(LineY);
	FCanvasLineItem LineZ(O, EndZ); LineZ.SetColor(FLinearColor::Blue); LineZ.LineThickness = 2.f; Canvas->DrawItem(LineZ);
}

void FArmorModGridEditorViewportClient::Draw(FViewport* InViewport, FCanvas* Canvas)
{
	FEditorViewportClient::Draw(InViewport, Canvas);
	DrawSelectedGridAxes(InViewport, Canvas);
}

bool FArmorModGridEditorViewportClient::InputKey(const FInputKeyEventArgs& EventArgs)
{
	FViewport* InViewport = EventArgs.Viewport;
	const FKey Key = EventArgs.Key;
	const EInputEvent Event = EventArgs.Event;
	if (!InViewport)
	{
		return FEditorViewportClient::InputKey(EventArgs);
	}

	if (Event == IE_Pressed)
	{
		if (Key == EKeys::R)
		{
			SetRotationMode(!bRotationMode);
			return true;
		}
		if (Key == EKeys::S)
		{
			SaveToArmor();
			return true;
		}
	}
	if (Key == EKeys::LeftMouseButton)
	{
		if (Event == IE_Pressed)
		{
			LastMousePos.X = InViewport->GetMouseX();
			LastMousePos.Y = InViewport->GetMouseY();
			FVector RayOrigin, RayDir;
			bool bHaveRay = false;
			FSceneView* View = (FSceneView*)ViewState.GetReference();
			if (View)
			{
				const FIntRect& ViewRect = View->UnscaledViewRect;
				FSceneView::DeprojectScreenToWorld(FVector2D(LastMousePos.X, LastMousePos.Y), ViewRect, View->ViewMatrices.GetInvViewProjectionMatrix(), RayOrigin, RayDir);
				bHaveRay = true;
			}
			else
			{
				// Запасной вариант: луч из камеры через экранную точку (ViewState может быть ещё не готов)
				FIntPoint VPSize = InViewport->GetSizeXY();
				if (VPSize.X > 0 && VPSize.Y > 0)
				{
					float NX = (LastMousePos.X / (float)VPSize.X) * 2.f - 1.f;
					float NY = 1.f - (LastMousePos.Y / (float)VPSize.Y) * 2.f;
					FMatrix ViewMat = FRotationMatrix(GetViewRotation());
					FVector ViewRight = ViewMat.GetColumn(0), ViewUp = ViewMat.GetColumn(1), ViewForward = -ViewMat.GetColumn(2);
					float FOVDeg = 90.f;
					float TanFov = FMath::Tan(FOVDeg * (float)PI / 360.f);
					float Aspect = (float)VPSize.X / (float)VPSize.Y;
					RayOrigin = GetViewLocation();
					RayDir = (ViewForward + ViewRight * (NX * TanFov * Aspect) + ViewUp * (NY * TanFov)).GetSafeNormal();
					bHaveRay = true;
				}
			}
			if (bHaveRay)
			{
				RayDir.Normalize();
				int32 Hit = HitTestGrid(RayOrigin, RayDir);
				SelectedGridIndex = Hit;
				SelectedAxis = -1;
				if (Hit >= 0)
				{
					TArray<FArmorModSideGrid> Grids;
					BuildDisplayGridsFromWorkingCopy(Grids);
					if (Hit < Grids.Num())
					{
						const FArmorModSideGrid& G = Grids[Hit];
						FTransform MeshToWorld = GetMeshWorldTransform();
						FVector WorldOrigin = MeshToWorld.TransformPosition(G.Origin);
						FVector WorldRight = MeshToWorld.TransformVector(G.AxisRight).GetSafeNormal();
						FVector WorldUp = MeshToWorld.TransformVector(G.AxisUp).GetSafeNormal();
						FVector WorldNormal = FVector::CrossProduct(WorldRight, WorldUp).GetSafeNormal();
						SelectedAxis = HitTestAxis(RayOrigin, RayDir, WorldOrigin, WorldRight, WorldUp, WorldNormal);
					}
				}
			}
			bDragging = true;
		}
		else if (Event == IE_Released)
		{
			bDragging = false;
			SelectedAxis = -1;
		}
	}
	return FEditorViewportClient::InputKey(EventArgs);
}

bool FArmorModGridEditorViewportClient::InputAxis(const FInputKeyEventArgs& EventArgs)
{
	FViewport* InViewport = EventArgs.Viewport;
	const FKey Key = EventArgs.Key;
	const float Delta = EventArgs.AmountDepressed;
	if (!InViewport)
	{
		return FEditorViewportClient::InputAxis(EventArgs);
	}

	if (bDragging && SelectedGridIndex >= 0 && SelectedGridIndex < GridIndexToSideIndex.Num())
	{
		int32 Side = GridIndexToSideIndex[SelectedGridIndex];
		FSceneView* View = (FSceneView*)ViewState.GetReference();
		if (View)
		{
			TArray<FArmorModSideGrid> Grids;
			BuildDisplayGridsFromWorkingCopy(Grids);
			if (SelectedGridIndex < Grids.Num())
			{
				const FArmorModSideGrid& G = Grids[SelectedGridIndex];
				FTransform MeshToWorld = GetMeshWorldTransform();
				FVector WorldRight = MeshToWorld.TransformVector(G.AxisRight).GetSafeNormal();
				FVector WorldUp = MeshToWorld.TransformVector(G.AxisUp).GetSafeNormal();
				FVector WorldNormal = FVector::CrossProduct(WorldRight, WorldUp).GetSafeNormal();

				if (bRotationMode)
				{
					FRotator& R = WorkingRotations[Side];
					R.Yaw   += Delta * (Key == EKeys::MouseX ? 1.f : 0.f);
					R.Pitch += Delta * (Key == EKeys::MouseY ? -1.f : 0.f);
					SyncWorkingCopyToPreview();
				}
				else if (SelectedAxis >= 0)
				{
					FVector AxisWorld = (SelectedAxis == 0) ? WorldRight : (SelectedAxis == 1) ? WorldUp : WorldNormal;
					FMatrix ViewMat = View->ViewMatrices.GetViewMatrix();
					FVector ViewRight = ViewMat.GetColumn(0);
					FVector ViewUp = ViewMat.GetColumn(1);
					float Move = 0.f;
					if (Key == EKeys::MouseX) Move = Delta * FVector::DotProduct(ViewRight, AxisWorld);
					else if (Key == EKeys::MouseY) Move = -Delta * FVector::DotProduct(ViewUp, AxisWorld);
					WorkingOrigins[Side] += AxisWorld * Move;
					SyncWorkingCopyToPreview();
				}
			}
		}
	}
	LastMousePos.X = InViewport->GetMouseX();
	LastMousePos.Y = InViewport->GetMouseY();
	return FEditorViewportClient::InputAxis(EventArgs);
}

void FArmorModGridEditorViewportClient::SaveToArmor()
{
	AEquipmentBase* Armor = ArmorPtr.Get();
	if (!Armor) return;

	Armor->ModsFrontOrigin = WorkingOrigins[0];
	Armor->ModsBackOrigin  = WorkingOrigins[1];
	Armor->ModsLeftOrigin  = WorkingOrigins[2];
	Armor->ModsRightOrigin = WorkingOrigins[3];
	Armor->ModsFrontRotation = WorkingRotations[0];
	Armor->ModsBackRotation  = WorkingRotations[1];
	Armor->ModsLeftRotation  = WorkingRotations[2];
	Armor->ModsRightRotation = WorkingRotations[3];

	Armor->Modify();
	Armor->RebuildModSideGrids();
}

// ---- SArmorModGridEditorViewport ----

void SArmorModGridEditorViewport::Construct(const FArguments& InArgs)
{
	Armor = InArgs._Armor;
	SEditorViewport::Construct(SEditorViewport::FArguments());
}

TSharedRef<FEditorViewportClient> SArmorModGridEditorViewport::MakeEditorViewportClient()
{
	ViewportClient = MakeShared<FArmorModGridEditorViewportClient>(&PreviewScene, SharedThis(this), Armor);
	return ViewportClient.ToSharedRef();
}

#undef LOCTEXT_NAMESPACE
#endif // WITH_EDITOR
