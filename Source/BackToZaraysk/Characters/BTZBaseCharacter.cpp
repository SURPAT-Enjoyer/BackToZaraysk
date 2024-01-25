// Fill out your copyright notice in the Description page of Project Settings.


#include "BTZBaseCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/SpringArmComponent.h"

ABTZBaseCharacter::ABTZBaseCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UBTZBaseCharMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	BTZBaseCharMovementComponent = StaticCast<UBTZBaseCharMovementComponent*>(GetCharacterMovement());

	
}

void ABTZBaseCharacter::ChangeCrouchState()
{
	if (BTZBaseCharMovementComponent->IsCrouching())
	{
		UnCrouch();
		//BTZBaseCharMovementComponent->UnProne();
	}
	else
	{
		Crouch();
		//BTZBaseCharMovementComponent->UnProne();
	}
}

void ABTZBaseCharacter::ChangeProneState()
{
	if (BTZBaseCharMovementComponent->IsProning())
	{
		BTZBaseCharMovementComponent->UnProne();
		//UnCrouch();
	}
	else
	{
		BTZBaseCharMovementComponent->Prone();
		//UnCrouch();
	}
}

void ABTZBaseCharacter::StartSprint()
{
	bIsSprintRequested = true;
	if (bIsCrouched)
	{
		UnCrouch();
	}
}

void ABTZBaseCharacter::StopSprint()
{
	bIsSprintRequested = false;
}

void ABTZBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	CurrentStamina = MaxStamina;
	//Capsule = GetCapsuleComponent();
	float test = GetMesh()->GetRelativeLocation().Z;
	GEngine->AddOnScreenDebugMessage(1, 1.0f, FColor::Red, FString::Printf(TEXT("%.2f"), test));

	IKScale = GetActorScale3D().Z;
	IKTraceDistance = BTZBaseCharMovementComponent->DefaultCapsuleHalfHeight * IKScale;
}

void ABTZBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	BTZBaseCharMovementComponent->GetMaxSpeed();
	TryChangeSprintState(DeltaTime);
	GEngine->AddOnScreenDebugMessage(0, 1.0f, FColor::Yellow, FString::Printf(TEXT("Stamina: %.2f"), CurrentStamina));
	if (!BTZBaseCharMovementComponent->IsSprinting() || !BTZBaseCharMovementComponent->IsFalling())
	{
		CurrentStamina += StaminaRestoreVelocity * DeltaTime;
		CurrentStamina = FMath::Clamp(CurrentStamina, 0.0f, MaxStamina);
	}
	
	IKLeftFootOffset = FMath::FInterpTo(IKLeftFootOffset, GetIKOffsetForASocket(LeftFootSocketName), DeltaTime, IKInterpSpeed);
	IKRightFootOffset = FMath::FInterpTo(IKRightFootOffset, GetIKOffsetForASocket(RightFootSocketName), DeltaTime, IKInterpSpeed);
}

void ABTZBaseCharacter::OnSprintStart_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("ABTZBaseCharacter::OnSprintStart_Implementation()"));
}

void ABTZBaseCharacter::OnSprintEnd_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("ABTZBaseCharacter::OnSprintEnd_Implementation()"));
}

bool ABTZBaseCharacter::CanSprint()
{
	return true;
}

void ABTZBaseCharacter::OnProneStart()
{
	//Capsule = CharacterOwner->GetCapsuleComponent();
	//SetCapsuleSize(1, 1);
	//GEngine->AddOnScreenDebugMessage(0, 1.0f, FColor::Yellow, FString::Printf(TEXT("%.2f"), Capsule));
}

void ABTZBaseCharacter::TryChangeSprintState(float DeltaTime)
{
	if ((BTZBaseCharMovementComponent->IsSprinting() || BTZBaseCharMovementComponent->IsFalling()) && !BTZBaseCharMovementComponent->IsProning())
	{
		CurrentStamina -= SprintStaminaConsumptionVelocity * DeltaTime;
	}
	if (CurrentStamina <= StaminaTiredThreshold)
	{
		GEngine->AddOnScreenDebugMessage(1, 1.0f, FColor::Red, FString::Printf(TEXT("Stamina is 20%")));
		BTZBaseCharMovementComponent->SetIsOutOfStamina(true);
		BTZBaseCharMovementComponent->CanAttemptJump();
	}
	if (CurrentStamina >= StaminaCanSprintAndJumpThreshold && !BTZBaseCharMovementComponent->IsProning())
	{
		BTZBaseCharMovementComponent->SetIsOutOfStamina(false);
		if (bIsSprintRequested && !BTZBaseCharMovementComponent->IsSprinting() && CanSprint())
		{
			BTZBaseCharMovementComponent->StartSprint();
			OnSprintStart();
		}
	}
	if (!bIsSprintRequested && BTZBaseCharMovementComponent->IsSprinting() )
	{
		BTZBaseCharMovementComponent->StopSprint();
		OnSprintEnd();
	}
}

float ABTZBaseCharacter::GetIKOffsetForASocket(const FName& SocketName)
{
	float Result = 0.0f;
	FVector SocketLocation = GetMesh()->GetSocketLocation(SocketName);
	FVector TraceStart(SocketLocation.X, SocketLocation.Y, GetActorLocation().Z);
	FVector TraceEnd = TraceStart - IKTraceDistance * FVector::UpVector;
	FHitResult HitResult;
	ETraceTypeQuery TraceType = UEngineTypes::ConvertToTraceType(ECC_Visibility);
	if (UKismetSystemLibrary::LineTraceSingle(GetWorld(), TraceStart, TraceEnd, TraceType, true, TArray<AActor*>(), EDrawDebugTrace::ForOneFrame, HitResult, true))
	{
		Result = (TraceEnd.Z - HitResult.Location.Z) / IKScale;
	}
	else if (UKismetSystemLibrary::LineTraceSingle(GetWorld(), TraceEnd, TraceEnd - IKTraceExtandDistance * FVector::UpVector, TraceType, true, TArray<AActor*>(), EDrawDebugTrace::ForOneFrame, HitResult, true))
	{
		Result = (TraceEnd.Z - HitResult.Location.Z) / IKScale;
	}

	return Result;
}