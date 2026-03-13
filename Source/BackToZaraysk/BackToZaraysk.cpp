// Copyright Epic Games, Inc. All Rights Reserved.

#include "BackToZaraysk.h"
#include "Modules/ModuleManager.h"
#include "BackToZaraysk/GameData/Items/ArmorBase.h"
#include "BackToZaraysk/GameData/Items/HelmetBase.h"
#include "BackToZaraysk/GameData/Items/VestBase.h"
#include "BackToZaraysk/GameData/Items/BackpackBase.h"
#include "BackToZaraysk/GameData/Items/EquipModBase.h"
#if WITH_EDITOR
#include "Editor/ArmorBaseDetailsCustomization.h"
#include "Editor/EquipModBaseDetailsCustomization.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#endif

void FBackToZarayskModule::StartupModule()
{
#if WITH_EDITOR
	FPropertyEditorModule& PropModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropModule.RegisterCustomClassLayout(AArmorBase::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FArmorBaseDetailsCustomization::MakeInstance));
	PropModule.RegisterCustomClassLayout(AHelmetBase::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FArmorBaseDetailsCustomization::MakeInstance));
	PropModule.RegisterCustomClassLayout(AVestBase::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FArmorBaseDetailsCustomization::MakeInstance));
	PropModule.RegisterCustomClassLayout(ABackpackBase::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FArmorBaseDetailsCustomization::MakeInstance));
	PropModule.RegisterCustomClassLayout(AEquipModBase::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FEquipModBaseDetailsCustomization::MakeInstance));
#endif
}

void FBackToZarayskModule::ShutdownModule()
{
#if WITH_EDITOR
	if (FPropertyEditorModule* PropModule = FModuleManager::GetModulePtr<FPropertyEditorModule>("PropertyEditor"))
	{
		PropModule->UnregisterCustomClassLayout(AArmorBase::StaticClass()->GetFName());
		PropModule->UnregisterCustomClassLayout(AHelmetBase::StaticClass()->GetFName());
		PropModule->UnregisterCustomClassLayout(AVestBase::StaticClass()->GetFName());
		PropModule->UnregisterCustomClassLayout(ABackpackBase::StaticClass()->GetFName());
		PropModule->UnregisterCustomClassLayout(AEquipModBase::StaticClass()->GetFName());
	}
#endif
}

IMPLEMENT_PRIMARY_GAME_MODULE(FBackToZarayskModule, BackToZaraysk, "BackToZaraysk");
