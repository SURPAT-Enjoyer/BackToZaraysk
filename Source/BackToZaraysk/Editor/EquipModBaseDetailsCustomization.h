// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "PropertyEditorModule.h"

class AEquipModBase;

class FEquipModBaseDetailsCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance() { return MakeShared<FEquipModBaseDetailsCustomization>(); }

	virtual void CustomizeDetails(IDetailLayoutBuilder& LayoutBuilder) override;

private:
	FReply OnEditAttachmentGridClicked();
	TWeakObjectPtr<AEquipModBase> EquipModPtr;
};
