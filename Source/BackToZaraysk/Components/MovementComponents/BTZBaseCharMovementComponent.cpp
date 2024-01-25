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
	/*if (!bIsProning && )
	{
		Result = CrouchSpeed;
	}*/
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

	float BTZProneCapsuleHalfHeight = 30.0f;
	//DefaultCapsuleRadius = GetCharacterOwner()->GetCapsuleComponent()->GetScaledCapsuleRadius();
	//DefaultCapsuleHalfHeight = GetCharacterOwner()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	float HalfHeightAdjust = (DefaultCapsuleHalfHeight - BTZProneCapsuleHalfHeight);
	const float ComponentScale = GetCharacterOwner()->GetCapsuleComponent()->GetShapeScale();
	float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;
	const float SweepInflation = UE_KINDA_SMALL_NUMBER * 10.f;
	StandingCapsuleShape = GetPawnCapsuleCollisionShape(SHRINK_HeightCustom, -SweepInflation - ScaledHalfHeightAdjust);
	return StandingCapsuleShape;
}

void UBTZBaseCharMovementComponent::Crouch(bool bClientSimulation)
{
	float CrouchCapsuleRadius = GetCharacterOwner()->GetCapsuleComponent()->GetUnscaledCapsuleRadius();
	float CrouchCapsuleHalfHeight = CrouchedHalfHeight;
	const float ComponentScale = GetCharacterOwner()->GetCapsuleComponent()->GetShapeScale();

	Super::Crouch(bClientSimulation);

	if (IsProning())
	{
		//bIsProning = false;
		if (!CachedCrouchState)
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, FString::Printf(TEXT("dick")));
			UnProne();
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, FString::Printf(TEXT("suck")));
			GetCharacterOwner()->bIsCrouched = false;
			UnProne();
		}
		//UnCrouch(bClientSimulation);
		//Super::Crouch(bClientSimulation);
		//SpringArmComponent->TargetOffset += FVector(0.0f, 0.0f, (CrouchCapsuleHalfHeight - ProneCapsuleHalfHeight) * ComponentScale);
	}
	
}

void UBTZBaseCharMovementComponent::UnCrouch(bool bClientSimulation)
{
	if (IsProning())
	{
		//Super::UnCrouch(bClientSimulation);
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, FString::Printf(TEXT("lox")));
		Crouch(bClientSimulation);
		CachedCrouchState = GetCharacterOwner()->bIsCrouched;
		//UnProne();
		//Super::UnCrouch(bClientSimulation);
	}
	else
	{
		Super::UnCrouch(bClientSimulation);
	}
}

void UBTZBaseCharMovementComponent::UnProne()
{
	
	bool bEnproning; //= true;
	const ECollisionChannel CollisionChannel = GetCharacterOwner()->GetCapsuleComponent()->GetCollisionObjectType();
	float HalfHeightAdjust = (DefaultCapsuleHalfHeight - ProneCapsuleHalfHeight);
	const float SweepInflation = UE_KINDA_SMALL_NUMBER * 10.f;
	const float ComponentScale = GetCharacterOwner()->GetCapsuleComponent()->GetShapeScale();
	float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;
	const FVector PawnLocation = GetCharacterOwner()->GetMesh()->GetComponentLocation();

	FCollisionQueryParams CapsuleParams(SCENE_QUERY_STAT(CrouchTrace), false, BTZCharacterOwner);
	FCollisionResponseParams ResponseParam;
	FHitResult Hit(1.f);
	bEnproning = GetWorld()->OverlapBlockingTestByChannel(PawnLocation, FQuat::Identity, CollisionChannel, GetCollisionShape(), CapsuleParams, ResponseParam);

	float CrouchCapsuleRadius = GetCharacterOwner()->GetCapsuleComponent()->GetUnscaledCapsuleRadius();
	float CrouchCapsuleHalfHeight = CrouchedHalfHeight;

	float PawnRadius, PawnHalfHeight;
	GetCharacterOwner()->GetCapsuleComponent()->GetScaledCapsuleSize(PawnRadius, PawnHalfHeight);
	const float ShrinkHalfHeight = PawnHalfHeight - PawnRadius;
	const float TraceDist = PawnHalfHeight - ShrinkHalfHeight;
	const float DistanceToBase = (Hit.Time * TraceDist) + ProneCapsuleHalfHeight;
	float CapsuleHeight = DefaultCapsuleHalfHeight * 2.0f;
	float BTZProneCapsuleHeight = ProneCapsuleHalfHeight * 2.0f;
	const FVector CapsuleLocation = GetCharacterOwner()->GetCapsuleComponent()->GetComponentLocation();
	FVector& MeshRelativeLocation = GetCharacterOwner()->GetMesh()->GetRelativeLocation_DirectMutable();
	MeshRelativeLocation.Z = GetCharacterOwner()->GetMesh()->GetRelativeLocation().Z - ScaledHalfHeightAdjust / 4.3f; //эмпирически подобранная константа, благодаря которой мешка не клипается сквозь пол/не зависает в воздухе
	//GetCharacterOwner()->BaseTranslationOffset.Z = MeshRelativeLocation.Z;
	FVector OldLocation = GetCharacterOwner()->GetMesh()->GetRelativeLocation();
	const FVector NewLoc = FVector(PawnLocation.X, PawnLocation.Y, PawnLocation.Z - DistanceToBase + DefaultCapsuleHalfHeight + SweepInflation + MIN_FLOOR_DIST / 2.f);

	

	if (IsCrouching() && IsProning())
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, FString::Printf(TEXT("Crouching")));
		GetCharacterOwner()->GetCapsuleComponent()->SetCapsuleSize(CrouchCapsuleRadius, CrouchCapsuleHalfHeight);
		GetCharacterOwner()->GetMesh()->SetRelativeLocation(OldLocation - FVector(0.0f, 0.0f, 2.0 / MeshRelativeLocation.Z), false, nullptr, GetTeleportType());
		//GetCharacterOwner()->GetCapsuleComponent()->MoveComponent(FVector(0.f, 0.f, ScaledHalfHeightAdjust), GetCharacterOwner()->GetCapsuleComponent()->GetComponentQuat(), true, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);*/
		//GetCharacterOwner()->Crouch();
		SpringArmComponent->TargetOffset -= FVector(0.0f, 0.0f, (CrouchCapsuleHalfHeight - ProneCapsuleHalfHeight) * ComponentScale);
		//GetCharacterOwner()->bIsCrouched = true;
		bIsProning = false;
	}
	else 
	{
		//GetCharacterOwner()->UnCrouch();
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, FString::Printf(TEXT("Not crouching")));
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, FString::Printf(TEXT("%.2f"), DefaultCapsuleRadius));
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, FString::Printf(TEXT("%.2f"), DefaultCapsuleHalfHeight));
		GetCharacterOwner()->GetCapsuleComponent()->SetCapsuleSize(DefaultCapsuleRadius, DefaultCapsuleHalfHeight);
		GetCharacterOwner()->GetMesh()->SetRelativeLocation(OldLocation + FVector(0.0f, 0.0f, MeshRelativeLocation.Z), false, nullptr, GetTeleportType());
		GetCharacterOwner()->GetCapsuleComponent()->MoveComponent(FVector(0.f, 0.f, ScaledHalfHeightAdjust), GetCharacterOwner()->GetCapsuleComponent()->GetComponentQuat(), true, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
		SpringArmComponent->TargetOffset -= FVector(0.0f, 0.0f, ScaledHalfHeightAdjust);
		bIsProning = false;
	}

	/*Capsule->SetCapsuleSize(DefaultCapsuleRadius, DefaultCapsuleHalfHeight);
	GetMesh()->SetRelativeLocation(OldLocation + FVector(0.0f, 0.0f, MeshRelativeLocation.Z), false, nullptr, BTZBaseCharMovementComponent->GetTeleportType());
	Capsule->MoveComponent(FVector(0.f, 0.f, ScaledHalfHeightAdjust), Capsule->GetComponentQuat(), true, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
	BTZBaseCharMovementComponent->bIsProning = false;
	SpringArmComponent->TargetOffset -= FVector(0.0f, 0.0f, ScaledHalfHeightAdjust);*/

	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, FString::Printf(TEXT("Now unproning")));

	float test = GetCharacterOwner()->GetMesh()->GetRelativeLocation().Z;
	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, FString::Printf(TEXT("%.2f"), test));
}

void UBTZBaseCharMovementComponent::Prone()
{
	//bIsCrouched = false;
	StopSprint();

	
	
	const float ComponentScale = GetCharacterOwner()->GetCapsuleComponent()->GetShapeScale();
	FVector& MeshRelativeLocation = GetCharacterOwner()->GetMesh()->GetRelativeLocation_DirectMutable();
	//Capsule->GetScaledCapsuleSize(DefaultCapsuleRadius, DefaultCapsuleHalfHeight);
	float HalfHeightAdjust = (DefaultCapsuleHalfHeight - ProneCapsuleHalfHeight);
	float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;
	float CrouchCapsuleRadius = GetCharacterOwner()->GetCapsuleComponent()->GetUnscaledCapsuleRadius();
	float CrouchCapsuleHalfHeight = CrouchedHalfHeight;
	MeshRelativeLocation.Z = GetCharacterOwner()->GetMesh()->GetRelativeLocation().Z + ScaledHalfHeightAdjust;
	//BaseTranslationOffset.Z = MeshRelativeLocation.Z;
	FVector OldLocation = GetCharacterOwner()->GetMesh()->GetRelativeLocation();
	MeshRelativeLocation.Z = GetCharacterOwner()->GetMesh()->GetRelativeLocation().Z + ScaledHalfHeightAdjust / 1.8f;//эмпирически подобранная константа, благодаря которой мешка не клипается сквозь пол/не зависает в воздухе
	GetCharacterOwner()->GetMesh()->SetRelativeLocation(OldLocation - FVector(0.0f, 0.0f, MeshRelativeLocation.Z), false, nullptr, GetTeleportType());

	bIsProning = true;

	if (IsCrouching())
	{
		//GetCharacterOwner()->UnCrouch();
		GetCharacterOwner()->GetCapsuleComponent()->SetCapsuleSize(ProneCapsuleRadius, ProneCapsuleHalfHeight);
		GetCharacterOwner()->GetCapsuleComponent()->MoveComponent(FVector(0.f, 0.f, -((CrouchCapsuleHalfHeight - ProneCapsuleHalfHeight) * ComponentScale) + 1.0f), GetCharacterOwner()->GetCapsuleComponent()->GetComponentQuat(), true, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
		SpringArmComponent->TargetOffset += FVector(0.0f, 0.0f, (CrouchCapsuleHalfHeight - ProneCapsuleHalfHeight) * ComponentScale);
	}
	else
	{
		GetCharacterOwner()->GetCapsuleComponent()->SetCapsuleSize(ProneCapsuleRadius, ProneCapsuleHalfHeight);
		GetCharacterOwner()->GetCapsuleComponent()->MoveComponent(FVector(0.f, 0.f, -ScaledHalfHeightAdjust + 1.0f), GetCharacterOwner()->GetCapsuleComponent()->GetComponentQuat(), true, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
		SpringArmComponent->TargetOffset += FVector(0.0f, 0.0f, ScaledHalfHeightAdjust);
	}

	/*Capsule->MoveComponent(FVector(0.f, 0.f, -ScaledHalfHeightAdjust + 1.0f), Capsule->GetComponentQuat(), true, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
	SpringArmComponent->TargetOffset += FVector(0.0f, 0.0f, ScaledHalfHeightAdjust);*/

	

	GEngine->AddOnScreenDebugMessage(6, 1.0f, FColor::Yellow, FString::Printf(TEXT("Now proning")));
	float test = GetCharacterOwner()->GetCapsuleComponent()->GetRelativeLocation().Z;
	GEngine->AddOnScreenDebugMessage(1, 1.0f, FColor::Red, FString::Printf(TEXT("%.2f"), test));
}

void UBTZBaseCharMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	DefaultCapsuleRadius = GetCharacterOwner()->GetCapsuleComponent()->GetScaledCapsuleRadius();
	DefaultCapsuleHalfHeight = GetCharacterOwner()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	SpringArmComponent = Cast<ABTZBaseCharacter>(GetCharacterOwner())->SpringArmComponent;
}
