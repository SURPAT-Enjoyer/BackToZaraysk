// Copyright Epic Games, Inc. All Rights Reserved.

// UE57 migration note: this editor-only tool lives in runtime module source tree,
// so we hard-guard compilation for non-editor targets.
#if WITH_EDITOR
#include "BackToZaraysk/Editor/EquipModGridEditorWindow.h"
#include "BackToZaraysk/GameData/Items/EquipModBase.h"
#include "BackToZaraysk/GameData/Items/ArmorModPreviewActor.h"
#include "BackToZaraysk/Inventory/InventoryItemData.h"
#include "EditorViewportClient.h"
#include "CanvasItem.h"
#include "CanvasTypes.h"
#include "SceneView.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"

#define LOCTEXT_NAMESPACE "EquipModGridEditor"

namespace
{
float EquipModRayToSegmentDist(const FVector& RayOrigin, const FVector& RayDir, const FVector& SegA, const FVector& SegB)
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
}

static UObject* GetModMesh(AEquipModBase* EquipMod)
{
	if (!EquipMod) return nullptr;
	if (UEquipModBaseItemData* ModData = Cast<UEquipModBaseItemData>(EquipMod->ItemInstance))
	{
		if (ModData->WorldMesh && ModData->WorldMesh->IsValidLowLevel())
			return ModData->WorldMesh;
	}
	if (EquipMod->SkeletalMesh && EquipMod->SkeletalMesh->GetSkeletalMeshAsset())
		return EquipMod->SkeletalMesh->GetSkeletalMeshAsset();
	if (EquipMod->Mesh && EquipMod->Mesh->GetStaticMesh())
		return EquipMod->Mesh->GetStaticMesh();
	return nullptr;
}

// ---- FEquipModGridEditorWindow ----

void FEquipModGridEditorWindow::OpenWindow(AEquipModBase* EquipMod)
{
	if (!IsValid(EquipMod))
		return;
	UObject* MeshObj = GetModMesh(EquipMod);
	if (!MeshObj)
		return;

	TSharedPtr<SEquipModGridEditorViewport> ViewportWidget = SNew(SEquipModGridEditorViewport).EquipMod(EquipMod);
	TWeakObjectPtr<AEquipModBase> EquipModWeak(EquipMod);

	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(LOCTEXT("WindowTitle", "Attachment Grid Editor (Equip Mod)"))
		.ClientSize(FVector2D(1000, 700))
		.SupportsMinimize(true)
		.SupportsMaximize(true)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(4.f)
			[
				SNew(SBorder).Padding(4.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
					[ SNew(STextBlock).Text(LOCTEXT("OriginX", "Origin X")) ]
					+ SHorizontalBox::Slot().FillWidth(0.15f)
					[
						SNew(SSpinBox<float>)
						.Value_Lambda([EquipModWeak, ViewportWidget]() -> float {
							if (TSharedPtr<FEquipModGridEditorViewportClient> VC = ViewportWidget.IsValid() ? ViewportWidget->GetViewportClient() : nullptr)
								return VC->GetWorkingOrigin().X;
							if (AEquipModBase* M = EquipModWeak.Get())
								return M->AttachmentGridOrigin.X;
							return 0.f;
						})
						.OnValueCommitted_Lambda([ViewportWidget](float V, ETextCommit::Type) {
							if (TSharedPtr<FEquipModGridEditorViewportClient> VC = ViewportWidget.IsValid() ? ViewportWidget->GetViewportClient() : nullptr)
							{
								FVector O = VC->GetWorkingOrigin();
								O.X = V;
								VC->SetWorkingOrigin(O);
							}
						})
						.MinValue(-1000.f).MaxValue(1000.f)
					]
					+ SHorizontalBox::Slot().AutoWidth().Padding(8, 0, 0, 0)
					[ SNew(STextBlock).Text(LOCTEXT("OriginY", "Y")) ]
					+ SHorizontalBox::Slot().FillWidth(0.15f)
					[
						SNew(SSpinBox<float>)
						.Value_Lambda([EquipModWeak, ViewportWidget]() -> float {
							if (TSharedPtr<FEquipModGridEditorViewportClient> VC = ViewportWidget.IsValid() ? ViewportWidget->GetViewportClient() : nullptr)
								return VC->GetWorkingOrigin().Y;
							if (AEquipModBase* M = EquipModWeak.Get())
								return M->AttachmentGridOrigin.Y;
							return 0.f;
						})
						.OnValueCommitted_Lambda([ViewportWidget](float V, ETextCommit::Type) {
							if (TSharedPtr<FEquipModGridEditorViewportClient> VC = ViewportWidget.IsValid() ? ViewportWidget->GetViewportClient() : nullptr)
							{
								FVector O = VC->GetWorkingOrigin();
								O.Y = V;
								VC->SetWorkingOrigin(O);
							}
						})
						.MinValue(-1000.f).MaxValue(1000.f)
					]
					+ SHorizontalBox::Slot().AutoWidth().Padding(8, 0, 0, 0)
					[ SNew(STextBlock).Text(LOCTEXT("OriginZ", "Z")) ]
					+ SHorizontalBox::Slot().FillWidth(0.15f)
					[
						SNew(SSpinBox<float>)
						.Value_Lambda([EquipModWeak, ViewportWidget]() -> float {
							if (TSharedPtr<FEquipModGridEditorViewportClient> VC = ViewportWidget.IsValid() ? ViewportWidget->GetViewportClient() : nullptr)
								return VC->GetWorkingOrigin().Z;
							if (AEquipModBase* M = EquipModWeak.Get())
								return M->AttachmentGridOrigin.Z;
							return 0.f;
						})
						.OnValueCommitted_Lambda([ViewportWidget](float V, ETextCommit::Type) {
							if (TSharedPtr<FEquipModGridEditorViewportClient> VC = ViewportWidget.IsValid() ? ViewportWidget->GetViewportClient() : nullptr)
							{
								FVector O = VC->GetWorkingOrigin();
								O.Z = V;
								VC->SetWorkingOrigin(O);
							}
						})
						.MinValue(-1000.f).MaxValue(1000.f)
					]
					+ SHorizontalBox::Slot().AutoWidth().Padding(16, 0, 0, 0)
					[
						SNew(SButton)
						.Text(LOCTEXT("SaveButton", "Save to Equip Mod"))
						.OnClicked_Lambda([ViewportWidget]() {
							if (TSharedPtr<FEquipModGridEditorViewportClient> VC = ViewportWidget.IsValid() ? ViewportWidget->GetViewportClient() : nullptr)
								VC->SaveToEquipMod();
							return FReply::Handled();
						})
					]
				]
			]
			+ SVerticalBox::Slot().FillHeight(1.f)
			[
				ViewportWidget.ToSharedRef()
			]
		];

	FSlateApplication::Get().AddWindow(Window);
}

// ---- FEquipModGridEditorViewportClient ----

FEquipModGridEditorViewportClient::FEquipModGridEditorViewportClient(FPreviewScene* InPreviewScene, TSharedPtr<SEditorViewport> InViewport, AEquipModBase* InEquipMod)
	: FEditorViewportClient(nullptr, InPreviewScene, TWeakPtr<SEditorViewport>(InViewport))
	, EquipModPtr(InEquipMod)
	, PreviewScene(InPreviewScene)
{
	if (!InPreviewScene || !IsValid(InEquipMod))
		return;

	WorkingOrigin = InEquipMod->AttachmentGridOrigin;

	SetViewportType(LVT_Perspective);
	SetViewModes(VMI_Lit, VMI_Lit);
	SetViewLocation(FVector(120.f, 0.f, 0.f));
	SetViewRotation(FRotator(0.f, 180.f, 0.f));
	bUsingOrbitCamera = true;

	UWorld* World = InPreviewScene->GetWorld();
	if (!World)
		return;

	UObject* MeshObj = GetModMesh(InEquipMod);
	if (!MeshObj)
		return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AArmorModPreviewActor* Preview = World->SpawnActor<AArmorModPreviewActor>(SpawnParams);
	if (!Preview)
		return;

	PreviewActorPtr = Preview;
	Preview->SetMeshFromEquipped(MeshObj);
	SyncWorkingCopyToPreview();
}

FEquipModGridEditorViewportClient::~FEquipModGridEditorViewportClient()
{
	if (AArmorModPreviewActor* Preview = PreviewActorPtr.Get())
		Preview->Destroy();
}

void FEquipModGridEditorViewportClient::BuildDisplayGrid(FArmorModSideGrid& OutGrid) const
{
	AEquipModBase* Mod = EquipModPtr.Get();
	if (!Mod) return;

	OutGrid.SideName = FName(TEXT("Attachment"));
	OutGrid.CellsX = FMath::Max(1, Mod->ModSlotSize.X);
	OutGrid.CellsY = FMath::Max(1, Mod->ModSlotSize.Y);
	OutGrid.Origin = WorkingOrigin;
	OutGrid.AxisRight = FVector(0.f, 1.f, 0.f);
	OutGrid.AxisUp = FVector(0.f, 0.f, 1.f);
}

void FEquipModGridEditorViewportClient::SyncWorkingCopyToPreview()
{
	FArmorModSideGrid G;
	BuildDisplayGrid(G);
	TArray<FArmorModSideGrid> Grids;
	Grids.Add(G);
	if (AArmorModPreviewActor* Preview = PreviewActorPtr.Get())
		Preview->SetModGrids(Grids);
}

FTransform FEquipModGridEditorViewportClient::GetMeshWorldTransform() const
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

int32 FEquipModGridEditorViewportClient::HitTestAxis(const FVector& WorldRayOrigin, const FVector& WorldRayDir,
	const FVector& WorldOrigin, const FVector& WorldAxisX, const FVector& WorldAxisY, const FVector& WorldAxisZ) const
{
	const float Len = AxisDrawLength;
	const FVector EndX = WorldOrigin + WorldAxisX * Len;
	const FVector EndY = WorldOrigin + WorldAxisY * Len;
	const FVector EndZ = WorldOrigin + WorldAxisZ * Len;
	float dX = EquipModRayToSegmentDist(WorldRayOrigin, WorldRayDir, WorldOrigin, EndX);
	float dY = EquipModRayToSegmentDist(WorldRayOrigin, WorldRayDir, WorldOrigin, EndY);
	float dZ = EquipModRayToSegmentDist(WorldRayOrigin, WorldRayDir, WorldOrigin, EndZ);
	int32 Best = -1;
	float BestD = AxisHitThreshold;
	if (dX < BestD) { BestD = dX; Best = 0; }
	if (dY < BestD) { BestD = dY; Best = 1; }
	if (dZ < BestD) { BestD = dZ; Best = 2; }
	return Best;
}

void FEquipModGridEditorViewportClient::DrawOriginAxes(FViewport* InViewport, FCanvas* Canvas)
{
	FTransform MeshToWorld = GetMeshWorldTransform();
	FVector WorldOrigin = MeshToWorld.TransformPosition(WorkingOrigin);
	FVector WorldRight = MeshToWorld.TransformVector(FVector(0.f, 1.f, 0.f)).GetSafeNormal();
	FVector WorldUp = MeshToWorld.TransformVector(FVector(0.f, 0.f, 1.f)).GetSafeNormal();
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

void FEquipModGridEditorViewportClient::Tick(float DeltaSeconds)
{
	FEditorViewportClient::Tick(DeltaSeconds);
	if (PreviewScene && DeltaSeconds > 0.f)
	{
		if (UWorld* World = PreviewScene->GetWorld())
			World->Tick(ELevelTick::LEVELTICK_All, DeltaSeconds);
	}
}

void FEquipModGridEditorViewportClient::Draw(FViewport* InViewport, FCanvas* Canvas)
{
	FEditorViewportClient::Draw(InViewport, Canvas);
	DrawOriginAxes(InViewport, Canvas);
}

bool FEquipModGridEditorViewportClient::InputKey(const FInputKeyEventArgs& EventArgs)
{
	FViewport* InViewport = EventArgs.Viewport;
	const FKey Key = EventArgs.Key;
	const EInputEvent Event = EventArgs.Event;
	if (!InViewport)
	{
		return FEditorViewportClient::InputKey(EventArgs);
	}

	if (Event == IE_Pressed && Key == EKeys::S)
	{
		SaveToEquipMod();
		return true;
	}
	if (Key == EKeys::LeftMouseButton)
	{
		if (Event == IE_Pressed)
		{
			LastMousePos.X = InViewport->GetMouseX();
			LastMousePos.Y = InViewport->GetMouseY();
			FVector RayOrigin, RayDir;
			FSceneView* View = (FSceneView*)ViewState.GetReference();
			if (View)
			{
				const FIntRect& ViewRect = View->UnscaledViewRect;
				FSceneView::DeprojectScreenToWorld(FVector2D(LastMousePos.X, LastMousePos.Y), ViewRect, View->ViewMatrices.GetInvViewProjectionMatrix(), RayOrigin, RayDir);
				RayDir.Normalize();
				FTransform MeshToWorld = GetMeshWorldTransform();
				FVector WorldOrigin = MeshToWorld.TransformPosition(WorkingOrigin);
				FVector WorldRight = MeshToWorld.TransformVector(FVector(0.f, 1.f, 0.f)).GetSafeNormal();
				FVector WorldUp = MeshToWorld.TransformVector(FVector(0.f, 0.f, 1.f)).GetSafeNormal();
				FVector WorldNormal = FVector::CrossProduct(WorldRight, WorldUp).GetSafeNormal();
				SelectedAxis = HitTestAxis(RayOrigin, RayDir, WorldOrigin, WorldRight, WorldUp, WorldNormal);
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

bool FEquipModGridEditorViewportClient::InputAxis(const FInputKeyEventArgs& EventArgs)
{
	FViewport* InViewport = EventArgs.Viewport;
	const FKey Key = EventArgs.Key;
	const float Delta = EventArgs.AmountDepressed;
	if (!InViewport)
	{
		return FEditorViewportClient::InputAxis(EventArgs);
	}

	if (bDragging && SelectedAxis >= 0)
	{
		FSceneView* View = (FSceneView*)ViewState.GetReference();
		if (View)
		{
			FTransform MeshToWorld = GetMeshWorldTransform();
			FVector WorldRight = MeshToWorld.TransformVector(FVector(0.f, 1.f, 0.f)).GetSafeNormal();
			FVector WorldUp = MeshToWorld.TransformVector(FVector(0.f, 0.f, 1.f)).GetSafeNormal();
			FVector WorldNormal = FVector::CrossProduct(WorldRight, WorldUp).GetSafeNormal();
			FVector AxisWorld = (SelectedAxis == 0) ? WorldRight : (SelectedAxis == 1) ? WorldUp : WorldNormal;
			FMatrix ViewMat = View->ViewMatrices.GetViewMatrix();
			FVector ViewRight = ViewMat.GetColumn(0);
			FVector ViewUp = ViewMat.GetColumn(1);
			float Move = 0.f;
			if (Key == EKeys::MouseX) Move = Delta * FVector::DotProduct(ViewRight, AxisWorld);
			else if (Key == EKeys::MouseY) Move = -Delta * FVector::DotProduct(ViewUp, AxisWorld);
			FVector WorldDelta = AxisWorld * Move;
			WorkingOrigin += MeshToWorld.Inverse().TransformVector(WorldDelta);
			SyncWorkingCopyToPreview();
		}
	}
	LastMousePos.X = InViewport->GetMouseX();
	LastMousePos.Y = InViewport->GetMouseY();
	return FEditorViewportClient::InputAxis(EventArgs);
}

void FEquipModGridEditorViewportClient::SaveToEquipMod()
{
	AEquipModBase* Mod = EquipModPtr.Get();
	if (!Mod) return;

	Mod->AttachmentGridOrigin = WorkingOrigin;
	Mod->Modify();
	Mod->SyncAttachmentGridOriginToItemInstance();
}

// ---- SEquipModGridEditorViewport ----

void SEquipModGridEditorViewport::Construct(const FArguments& InArgs)
{
	EquipMod = InArgs._EquipMod;
	SEditorViewport::Construct(SEditorViewport::FArguments());
}

TSharedRef<FEditorViewportClient> SEquipModGridEditorViewport::MakeEditorViewportClient()
{
	ViewportClient = MakeShared<FEquipModGridEditorViewportClient>(&PreviewScene, SharedThis(this), EquipMod);
	return ViewportClient.ToSharedRef();
}

#undef LOCTEXT_NAMESPACE
#endif // WITH_EDITOR
