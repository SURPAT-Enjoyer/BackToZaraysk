// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "PropertyEditorModule.h"

class AEquipmentBase;

class FArmorBaseDetailsCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance() { return MakeShared<FArmorBaseDetailsCustomization>(); }

	virtual void CustomizeDetails(IDetailLayoutBuilder& LayoutBuilder) override;

private:
	FReply OnEditModGridsClicked();
	TWeakObjectPtr<AEquipmentBase> ArmorPtr;
};
