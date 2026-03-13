// Copyright Epic Games, Inc. All Rights Reserved.

#include "ArmorBaseDetailsCustomization.h"
#include "BackToZaraysk/GameData/Items/EquipmentBase.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Framework/Application/SlateApplication.h"
#include "BackToZaraysk/Editor/ArmorModGridEditorWindow.h"

#define LOCTEXT_NAMESPACE "ArmorBaseDetails"

void FArmorBaseDetailsCustomization::CustomizeDetails(IDetailLayoutBuilder& LayoutBuilder)
{
	TArray<TWeakObjectPtr<UObject>> Objects;
	LayoutBuilder.GetObjectsBeingCustomized(Objects);
	if (Objects.Num() == 0) return;

	AEquipmentBase* Armor = Cast<AEquipmentBase>(Objects[0].Get());
	if (!Armor || !Armor->bIsModdable) return;

	ArmorPtr = Armor;

	IDetailCategoryBuilder& ModCategory = LayoutBuilder.EditCategory("Armor|Mods");
	ModCategory.AddCustomRow(LOCTEXT("ModGridsFilter", "Edit Mod Grids"))
		.NameContent()
		[
			SNew(STextBlock).Text(LOCTEXT("EditModGrids", "Mod Grid Editor"))
		]
		.ValueContent()
		[
			SNew(SButton)
			.Text(LOCTEXT("EditModGridsButton", "Edit Mod Grids..."))
			.OnClicked(this, &FArmorBaseDetailsCustomization::OnEditModGridsClicked)
		];
}

FReply FArmorBaseDetailsCustomization::OnEditModGridsClicked()
{
	if (AEquipmentBase* Armor = ArmorPtr.Get())
	{
		FArmorModGridEditorWindow::OpenWindow(Armor);
	}
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
