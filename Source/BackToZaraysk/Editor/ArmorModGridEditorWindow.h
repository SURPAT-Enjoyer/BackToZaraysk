// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "EditorViewportClient.h"
#include "SEditorViewport.h"
#include "PreviewScene.h"
#include "BackToZaraysk/GameData/Items/EquipmentBase.h"
#include "BackToZaraysk/GameData/Items/ArmorModPreviewActor.h"
#include "BackToZaraysk/GameData/Items/ArmorModSideGrid.h"

class FArmorModGridEditorViewportClient;
class AArmorModPreviewActor;

/** Standalone window for editing equipment mod grids (armor, vest, backpack, helmet, ...). */
class FArmorModGridEditorWindow
{
public:
	static void OpenWindow(AEquipmentBase* Armor);
};

/** Viewport client: draws armor + grids, handles LMB select/drag and rotation. */
class FArmorModGridEditorViewportClient : public FEditorViewportClient
{
public:
	FArmorModGridEditorViewportClient(FPreviewScene* InPreviewScene, TSharedPtr<SEditorViewport> InViewport,
		AEquipmentBase* InArmor);
	virtual ~FArmorModGridEditorViewportClient() override;

	virtual void Tick(float DeltaSeconds) override;
	virtual void Draw(FViewport* InViewport, FCanvas* Canvas) override;
	virtual bool InputKey(const FInputKeyEventArgs& EventArgs) override;
	virtual bool InputAxis(const FInputKeyEventArgs& EventArgs) override;

	void SetRotationMode(bool bEnabled) { bRotationMode = bEnabled; }
	bool IsRotationMode() const { return bRotationMode; }
	void SaveToArmor();

	/** Working data: origin and rotation per side (0=Front, 1=Back, 2=Left, 3=Right). */
	TArray<FVector> WorkingOrigins;
	TArray<FRotator> WorkingRotations;
	/** For each grid drawn, the side index (0-3). */
	TArray<int32> GridIndexToSideIndex;
	/** Cells X/Y per grid (same order as GridIndexToSideIndex). */
	TArray<FIntPoint> GridCells;
	int32 SelectedGridIndex = INDEX_NONE;
	/** Dragging along axis: 0=Right, 1=Up, 2=Normal; -1=none. */
	int32 SelectedAxis = -1;
	TWeakObjectPtr<AEquipmentBase> ArmorPtr;
	TWeakObjectPtr<AArmorModPreviewActor> PreviewActorPtr;
	/** R = rotation mode, S = save to armor. */

private:
	void SyncWorkingCopyToPreview();
	void BuildDisplayGridsFromWorkingCopy(TArray<FArmorModSideGrid>& OutGrids);
	int32 HitTestGrid(const FVector& WorldRayOrigin, const FVector& WorldRayDir) const;
	int32 HitTestAxis(const FVector& WorldRayOrigin, const FVector& WorldRayDir, const FVector& WorldOrigin, const FVector& WorldAxisX, const FVector& WorldAxisY, const FVector& WorldAxisZ) const;
	void DrawSelectedGridAxes(FViewport* InViewport, FCanvas* Canvas);
	FTransform GetMeshWorldTransform() const;

	FPreviewScene* PreviewScene = nullptr;
	bool bRotationMode = false;
	bool bDragging = false;
	FIntPoint LastMousePos;
	static constexpr float AxisDrawLength = 25.f;
	static constexpr float AxisHitThreshold = 8.f;
};

/** Widget: toolbar + viewport. */
class SArmorModGridEditorViewport : public SEditorViewport
{
public:
	SLATE_BEGIN_ARGS(SArmorModGridEditorViewport) {}
		SLATE_ARGUMENT(AEquipmentBase*, Armor)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;

	TSharedPtr<FArmorModGridEditorViewportClient> GetViewportClient() const { return ViewportClient; }

private:
	TSharedPtr<FArmorModGridEditorViewportClient> ViewportClient;
	FPreviewScene PreviewScene;
	AEquipmentBase* Armor = nullptr;
};
