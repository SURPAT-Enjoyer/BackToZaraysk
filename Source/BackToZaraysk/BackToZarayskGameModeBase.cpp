// Copyright Epic Games, Inc. All Rights Reserved.


#include "BackToZarayskGameModeBase.h"
#include "Characters/Controllers/BTZPlayerController.h"

ABackToZarayskGameModeBase::ABackToZarayskGameModeBase()
{
    PlayerControllerClass = ABTZPlayerController::StaticClass();
}

