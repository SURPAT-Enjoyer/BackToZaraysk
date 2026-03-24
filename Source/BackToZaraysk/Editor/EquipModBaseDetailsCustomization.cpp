// Copyright Epic Games, Inc. All Rights Reserved.

#if WITH_EDITOR
#include "EquipModBaseDetailsCustomization.h"
#include "BackToZaraysk/GameData/Items/EquipModBase.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "BackToZaraysk/Editor/EquipModGridEditorWindow.h"

#define LOCTEXT_NAMESPACE "EquipModBaseDetails"

void FEquipModBaseDetailsCustomization::CustomizeDetails(IDetailLayoutBuilder& LayoutBuilder)
{
	TArray<TWeakObjectPtr<UObject>> Objects;
	LayoutBuilder.GetObjectsBeingCustomized(Objects);
	if (Objects.Num() == 0) return;

	AEquipModBase* EquipMod = Cast<AEquipModBase>(Objects[0].Get());
	if (!EquipMod) return;

	EquipModPtr = EquipMod;

	IDetailCategoryBuilder& SlotsCategory = LayoutBuilder.EditCategory("EquipMod|Slots");
	SlotsCategory.AddCustomRow(LOCTEXT("AttachmentGridFilter", "Attachment Grid"))
		.NameContent()
		[
			SNew(STextBlock).Text(LOCTEXT("AttachmentGridEditor", "Attachment Grid Editor"))
		]
		.ValueContent()
		[
			SNew(SButton)
			.Text(LOCTEXT("EditAttachmentGridButton", "Edit Attachment Grid..."))
			.OnClicked(this, &FEquipModBaseDetailsCustomization::OnEditAttachmentGridClicked)
		];
}

FReply FEquipModBaseDetailsCustomization::OnEditAttachmentGridClicked()
{
	AEquipModBase* EquipMod = EquipModPtr.Get();
	if (IsValid(EquipMod))
		FEquipModGridEditorWindow::OpenWindow(EquipMod);
	return FReply::Handled();
}
#endif // WITH_EDITOR

#undef LOCTEXT_NAMESPACE
