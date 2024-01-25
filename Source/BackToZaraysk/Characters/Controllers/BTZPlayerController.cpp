// Fill out your copyright notice in the Description page of Project Settings.


#include "BTZPlayerController.h"
#include"../BTZBaseCharacter.h"

void ABTZPlayerController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);
	CachedBaseCharacter = Cast<ABTZBaseCharacter>(InPawn);
}

void ABTZPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	InputComponent->BindAxis("MoveForward", this, &ABTZPlayerController::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &ABTZPlayerController::MoveRight);
	InputComponent->BindAxis("Turn", this, &ABTZPlayerController::Turn);
	InputComponent->BindAxis("LookUp", this, &ABTZPlayerController::LookUp);
	InputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &ABTZPlayerController::Jump);
	InputComponent->BindAction("Crouch", EInputEvent::IE_Pressed, this, &ABTZPlayerController::ChangeCrouchState);
	InputComponent->BindAction("Sprint", EInputEvent::IE_Pressed, this, &ABTZPlayerController::StartSprint);
	InputComponent->BindAction("Sprint", EInputEvent::IE_Released, this, &ABTZPlayerController::StopSprint);
	InputComponent->BindAction("Prone", EInputEvent::IE_Pressed, this, &ABTZPlayerController::ChangeProneState);
}

void ABTZPlayerController::MoveForward(float Value)
{
	if (ensure(CachedBaseCharacter.IsValid()))
	{
		CachedBaseCharacter->MoveForward(Value);
	}
}

void ABTZPlayerController::MoveRight(float Value)
{
	if (ensure(CachedBaseCharacter.IsValid()))
	{
		CachedBaseCharacter->MoveRight(Value);
	}
}

void ABTZPlayerController::Turn(float Value)
{
	if (ensure(CachedBaseCharacter.IsValid()))
	{
		CachedBaseCharacter->Turn(Value);
	}
}

void ABTZPlayerController::LookUp(float Value)
{
	if (ensure(CachedBaseCharacter.IsValid()))
	{
		CachedBaseCharacter->LookUp(Value);
	}
}

void ABTZPlayerController::Jump()
{
	if (ensure(CachedBaseCharacter.IsValid()))
	{
		CachedBaseCharacter->Jump();
	}
}
void ABTZPlayerController::ChangeCrouchState()
{
	if (ensure(CachedBaseCharacter.IsValid()))
	{
		CachedBaseCharacter->ChangeCrouchState();
	}

}

void ABTZPlayerController::StartSprint()
{
	if (ensure(CachedBaseCharacter.IsValid()))
	{
		CachedBaseCharacter->StartSprint();
	}
}

void ABTZPlayerController::StopSprint()
{
	if (ensure(CachedBaseCharacter.IsValid()))
	{
		CachedBaseCharacter->StopSprint();
	}
}

void ABTZPlayerController::ChangeProneState()
{
	if (ensure(CachedBaseCharacter.IsValid()))
	{
		CachedBaseCharacter->ChangeProneState();
	}
}
