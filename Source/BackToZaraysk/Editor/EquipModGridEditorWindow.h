// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "EditorViewportClient.h"
#include "SEditorViewport.h"
#include "PreviewScene.h"
#include "BackToZaraysk/GameData/Items/ArmorModSideGrid.h"
#include "BackToZaraysk/GameData/Items/EquipModBase.h"

class FEquipModGridEditorViewportClient;
class AArmorModPreviewActor;

/** Окно редактора грида привязки модуля (EquipModBase): модель + грид, точка отсчёта в пространстве меша. */
class FEquipModGridEditorWindow
{
public:
	static void OpenWindow(AEquipModBase* EquipMod);
};

/** Viewport client: меш модуля + один грид привязки (размер ModSlotSize, origin = точка отсчёта). */
class FEquipModGridEditorViewportClient : public FEditorViewportClient
{
public:
	FEquipModGridEditorViewportClient(FPreviewScene* InPreviewScene, TSharedPtr<SEditorViewport> InViewport, AEquipModBase* InEquipMod);
	virtual ~FEquipModGridEditorViewportClient() override;

	virtual void Tick(float DeltaSeconds) override;
	virtual void Draw(FViewport* InViewport, FCanvas* Canvas) override;
	virtual bool InputKey(const FInputKeyEventArgs& EventArgs) override;
	virtual bool InputAxis(const FInputKeyEventArgs& EventArgs) override;

	void SetWorkingOrigin(const FVector& Origin) { WorkingOrigin = Origin; SyncWorkingCopyToPreview(); }
	FVector GetWorkingOrigin() const { return WorkingOrigin; }
	void SaveToEquipMod();

	TWeakObjectPtr<AEquipModBase> EquipModPtr;
	TWeakObjectPtr<AArmorModPreviewActor> PreviewActorPtr;
	/** Точка отсчёта грида в пространстве меша (редактируемая). */
	FVector WorkingOrigin = FVector::ZeroVector;
	int32 SelectedAxis = -1;
	bool bDragging = false;
	FIntPoint LastMousePos;

private:
	void SyncWorkingCopyToPreview();
	void BuildDisplayGrid(FArmorModSideGrid& OutGrid) const;
	int32 HitTestAxis(const FVector& WorldRayOrigin, const FVector& WorldRayDir, const FVector& WorldOrigin, const FVector& WorldAxisX, const FVector& WorldAxisY, const FVector& WorldAxisZ) const;
	void DrawOriginAxes(FViewport* InViewport, FCanvas* Canvas);
	FTransform GetMeshWorldTransform() const;

	FPreviewScene* PreviewScene = nullptr;
	static constexpr float ModGridCellWorldSize = 5.0f;
	static constexpr float AxisDrawLength = 25.f;
	static constexpr float AxisHitThreshold = 8.f;
};

/** Виджет: панель с полями X,Y,Z + viewport. */
class SEquipModGridEditorViewport : public SEditorViewport
{
public:
	SLATE_BEGIN_ARGS(SEquipModGridEditorViewport) {}
		SLATE_ARGUMENT(AEquipModBase*, EquipMod)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;

	TSharedPtr<FEquipModGridEditorViewportClient> GetViewportClient() const { return ViewportClient; }

private:
	TSharedPtr<FEquipModGridEditorViewportClient> ViewportClient;
	FPreviewScene PreviewScene;
	AEquipModBase* EquipMod = nullptr;
};
