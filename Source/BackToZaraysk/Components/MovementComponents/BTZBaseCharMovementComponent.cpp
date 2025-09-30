// Fill out your copyright notice in the Description page of Project Settings.


#include "BTZBaseCharMovementComponent.h"
#include "../../Characters/BTZBaseCharacter.h"
#include "Components/CapsuleComponent.h"

bool UBTZBaseCharMovementComponent::IsProning()
{
	//CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(ProneCapsuleRadius, ProneCapsuleHalfHeight);
	return bIsProning;	
}

float UBTZBaseCharMovementComponent::GetMaxSpeed() const
{
	float Result = Super::GetMaxSpeed();
	if (bIsSprinting && !bIsOutOfStamina)
	{
		Result = SprintSpeed;
	}
	if (bIsOutOfStamina)
	{
		Result = TiredSpeed;
	}
	if (!bIsOutOfStamina && !bIsSprinting)
	{
		Result = RunSpeed;
	}
	if (bIsProning)
	{
		Result = ProneSpeed;
	}
	if (!bIsProning && IsCrouching())
	{
		Result = CrouchSpeed;
	}
	return Result;
}

bool UBTZBaseCharMovementComponent::CanAttemptJump() const
{
	if (bIsOutOfStamina || bIsProning)
	{
		return false;
	}
	else
	{
		return Super::CanAttemptJump();
	}
}

void UBTZBaseCharMovementComponent::StartSprint()
{
	bIsSprinting = true;
	bForceMaxAccel = 1;
}

void UBTZBaseCharMovementComponent::StopSprint()
{
	bIsSprinting = false;
	bForceMaxAccel = 0;
}

void UBTZBaseCharMovementComponent::SetIsOutOfStamina(bool bIsOutOfStamina_In)
{
	bIsOutOfStamina = bIsOutOfStamina_In;
}
/*
void UBTZBaseCharMovementComponent::OnProne()
{
	//BTZBaseCharacter = StaticCast<ABTZBaseCharacter*>(GetCharacter());
	
}*/

//void UBTZBaseCharMovementComponent::Crouch(bool bClientSimulation)
//{
//	Super::Crouch(bClientSimulation);
//	//bIsProning = false;
//}

FCollisionShape UBTZBaseCharMovementComponent::GetCollisionShape()
{

	float LocalProneCapsuleHalfHeight = 30.0f;
	DefaultCapsuleRadius = GetCharacterOwner()->GetCapsuleComponent()->GetScaledCapsuleRadius();
	DefaultCapsuleHalfHeight = GetCharacterOwner()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	float HalfHeightAdjust = (DefaultCapsuleHalfHeight - LocalProneCapsuleHalfHeight);
	const float ComponentScale = GetCharacterOwner()->GetCapsuleComponent()->GetShapeScale();
	float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;
	const float SweepInflation = UE_KINDA_SMALL_NUMBER * 10.f;
	StandingCapsuleShape = GetPawnCapsuleCollisionShape(SHRINK_HeightCustom, -SweepInflation - ScaledHalfHeightAdjust);
	return StandingCapsuleShape;
}

void UBTZBaseCharMovementComponent::Crouch(bool bClientSimulation)
{
	if (IsProning())
	{
		UnProne();
	}
    // Before calling Super, cache old half height
    UCapsuleComponent* Capsule = GetCharacterOwner()->GetCapsuleComponent();
    const float OldHalf = Capsule ? Capsule->GetUnscaledCapsuleHalfHeight() : 0.0f;
    Super::Crouch(bClientSimulation);
    // After crouch, lift mesh to visually follow capsule change
    if (ACharacter* Character = GetCharacterOwner())
    {
        if (USkeletalMeshComponent* Mesh = Character->GetMesh())
        {
            const float NewHalf = Character->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
            const float MeshLift = (OldHalf - NewHalf) + CrouchMeshExtraZOffset; // positive when shrinking
            if (!FMath::IsNearlyZero(MeshLift))
            {
                FVector RelLoc = Mesh->GetRelativeLocation();
                RelLoc.Z += MeshLift;
                Mesh->SetRelativeLocation(RelLoc);
                CrouchMeshAppliedLift = MeshLift;
            }
        }
    }
}

void UBTZBaseCharMovementComponent::UnCrouch(bool bClientSimulation)
{
	if (IsProning())
	{
		UnProne();
	}
    // Before uncrouch, restore mesh offset applied in crouch
    if (ACharacter* Character = GetCharacterOwner())
    {
        if (USkeletalMeshComponent* Mesh = Character->GetMesh())
        {
            if (!FMath::IsNearlyZero(CrouchMeshAppliedLift))
            {
                FVector RelLoc = Mesh->GetRelativeLocation();
                RelLoc.Z -= CrouchMeshAppliedLift;
                Mesh->SetRelativeLocation(RelLoc);
                CrouchMeshAppliedLift = 0.0f;
            }
        }
    }
    Super::UnCrouch(bClientSimulation);
}

void UBTZBaseCharMovementComponent::UnProne()
{
	// Restore capsule to crouch or standing dimensions keeping capsule bottom at same height via swept move
	UCapsuleComponent* Capsule = GetCharacterOwner()->GetCapsuleComponent();
	const float OldHalf = Capsule->GetUnscaledCapsuleHalfHeight();
	float TargetRadius = Capsule->GetUnscaledCapsuleRadius();
	float TargetHalf = OldHalf;

	if (IsCrouching())
	{
		TargetRadius = Capsule->GetUnscaledCapsuleRadius();
		TargetHalf = GetCrouchedHalfHeight();
	}
	else
	{
		TargetRadius = DefaultCapsuleRadius;
		TargetHalf = DefaultCapsuleHalfHeight;
	}

	Capsule->SetCapsuleSize(TargetRadius, TargetHalf, true);

	const float HalfDelta = TargetHalf - OldHalf; // unscaled delta
	const float ScaledHalfDelta = HalfDelta * Capsule->GetShapeScale();
	if (!FMath::IsNearlyZero(ScaledHalfDelta))
	{
		FHitResult Hit;
		const FVector Offset = FVector(0.f, 0.f, ScaledHalfDelta);
		MoveUpdatedComponent(Offset, UpdatedComponent->GetComponentQuat(), true, &Hit);
	}
    // Restore mesh offset relative by removing the previously applied prone lift
    if (ACharacter* Character = GetCharacterOwner())
    {
        if (USkeletalMeshComponent* Mesh = Character->GetMesh())
        {
            if (!FMath::IsNearlyZero(ProneMeshAppliedLift))
            {
                FVector RelLoc = Mesh->GetRelativeLocation();
                RelLoc.Z -= ProneMeshAppliedLift;
                Mesh->SetRelativeLocation(RelLoc);
                ProneMeshAppliedLift = 0.0f;
            }
        }
    }
	bIsProning = false;
}

void UBTZBaseCharMovementComponent::Prone()
{
	StopSprint();
	UCapsuleComponent* Capsule = GetCharacterOwner()->GetCapsuleComponent();
	const float OldHalf = Capsule->GetUnscaledCapsuleHalfHeight();
	Capsule->SetCapsuleSize(ProneCapsuleRadius, ProneCapsuleHalfHeight, true);
	const float HalfDelta = ProneCapsuleHalfHeight - OldHalf; // unscaled delta
	const float ScaledHalfDelta = HalfDelta * Capsule->GetShapeScale();
	if (!FMath::IsNearlyZero(ScaledHalfDelta))
	{
		FHitResult Hit;
		const FVector Offset = FVector(0.f, 0.f, ScaledHalfDelta);
		MoveUpdatedComponent(Offset, UpdatedComponent->GetComponentQuat(), true, &Hit);
	}
    // Raise mesh to avoid clipping into ground in prone: lift by (capsule half-height delta) + extra offset from current relative location
    if (ACharacter* Character = GetCharacterOwner())
    {
        if (USkeletalMeshComponent* Mesh = Character->GetMesh())
        {
            const float NewHalf = ProneCapsuleHalfHeight;
            const float MeshLift = (OldHalf - NewHalf) + ProneMeshExtraZOffset; // positive when shrinking
            if (!FMath::IsNearlyZero(MeshLift))
            {
                FVector RelLoc = Mesh->GetRelativeLocation();
                RelLoc.Z += MeshLift;
                Mesh->SetRelativeLocation(RelLoc);
                ProneMeshAppliedLift = MeshLift;
            }
        }
    }
	bIsProning = true;
}

void UBTZBaseCharMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	BTZCharacterOwner = GetCharacterOwner();
	DefaultCapsuleRadius = GetCharacterOwner()->GetCapsuleComponent()->GetScaledCapsuleRadius();
	DefaultCapsuleHalfHeight = GetCharacterOwner()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    // no default mesh caching needed anymore
}
