// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "BackToZaraysk/Inventory/InventoryComponent.h"
#include "BackToZaraysk/Components/EquipmentComponent.h"
#include "../Components/MovementComponents/BTZBaseCharMovementComponent.h"
#include "../Components/ObstacleClimbingComponent.h"
#include "../Components/StrafeComponent.h"
#include "Animation/AnimBlueprintGeneratedClass.h"
#include "GameFramework/PlayerController.h"
#include "Curves/CurveFloat.h"

APlayerCharacter::APlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComponent->SetupAttachment(RootComponent);
    SpringArmComponent->bUsePawnControlRotation = true;
    // Не наследуем Roll от контроллера, чтобы можно было наклонять руками
    SpringArmComponent->bInheritRoll = false;
    SpringArmComponent->TargetArmLength = 0.0f; // First-person: no offset from the pawn
    SpringArmComponent->bDoCollisionTest = false; // Avoid camera push-back

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(SpringArmComponent);
    CameraComponent->bUsePawnControlRotation = false;
    CameraComponent->SetRelativeLocation(FVector(0.0f, 0.0f, BaseEyeHeight)); // Place camera at eye height

    GetCharacterMovement()->bOrientRotationToMovement = 0; // First-person: character rotates with controller
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.f, 0.0f);

    //SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Player mesh"));
	//SkeletalMeshComponent->SetupAttachment(RootComponent);

    InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
    if (!InventoryComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerCharacter: Failed to create InventoryComponent!"));
    }

    EquipmentComponent = CreateDefaultSubobject<UEquipmentComponent>(TEXT("EquipmentComponent"));
    if (!EquipmentComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerCharacter: Failed to create EquipmentComponent!"));
    }

    ObstacleClimbingComponent = CreateDefaultSubobject<UObstacleClimbingComponent>(TEXT("ObstacleClimbingComponent"));

    StrafeComponent = CreateDefaultSubobject<UStrafeComponent>(TEXT("StrafeComponent"));

}

void APlayerCharacter::MoveForward(float Value)
{
	if (!FMath::IsNearlyZero(Value, 1e-6f))
	{
		// КРИТИЧЕСКИ ВАЖНО: Блокировка движения во время лазания!
		if (ObstacleClimbingComponent && ObstacleClimbingComponent->bIsClimbing)
		{
			return;
		}
		
		// ИСПРАВЛЕНО: Полная блокировка движения во время стрейфа
		if (StrafeComponent && StrafeComponent->bIsStrafing)
		{
			// Во время стрейфа полностью блокируем обычное движение
			return;
		}
		
		FRotator YawRotator(0.0f, GetControlRotation().Yaw, 0.0f);
		FVector ForwardVector = YawRotator.RotateVector(FVector::ForwardVector);
		AddMovementInput(ForwardVector, Value);
	}
}

void APlayerCharacter::MoveRight(float Value)
{
	if (!FMath::IsNearlyZero(Value, 1e-6f))
	{
		// КРИТИЧЕСКИ ВАЖНО: Блокировка движения во время лазания!
		if (ObstacleClimbingComponent && ObstacleClimbingComponent->bIsClimbing)
		{
			return;
		}
		
		// ИСПРАВЛЕНО: Полная блокировка движения во время стрейфа
		if (StrafeComponent && StrafeComponent->bIsStrafing)
		{
			// Во время стрейфа полностью блокируем обычное движение
			return;
		}
		
		FRotator YawRotator(0.0f, GetControlRotation().Yaw, 0.0f);
		FVector RightVector = YawRotator.RotateVector(FVector::RightVector);
		AddMovementInput(RightVector, Value);
	}
}
void APlayerCharacter::Turn(float Value)
{
	// КРИТИЧЕСКИ ВАЖНО: Блокируем поворот во время лазания!
	if (ObstacleClimbingComponent && ObstacleClimbingComponent->bIsClimbing)
	{
		return;
	}
	
	AddControllerYawInput(Value);
}

void APlayerCharacter::LookUp(float Value)
{
	// КРИТИЧЕСКИ ВАЖНО: Блокируем взгляд вверх/вниз во время лазания!
	if (ObstacleClimbingComponent && ObstacleClimbingComponent->bIsClimbing)
	{
		return;
	}
	
	AddControllerPitchInput(Value);
}

void APlayerCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
    // Обновляем по голове
    USkeletalMeshComponent* Skel = GetMesh();
    if (Skel && Skel->DoesSocketExist(HeadSocketName))
    {
        const FTransform HeadWorld = Skel->GetSocketTransform(HeadSocketName, RTS_World);
        const FVector PivotWorld = HeadWorld.GetLocation() + HeadOffset;
        const FVector PivotLocal = GetRootComponent()->GetComponentTransform().InverseTransformPosition(PivotWorld);
        SpringArmComponent->SetRelativeLocation(PivotLocal);
    }
}

void APlayerCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
    // Обновляем по голове
    USkeletalMeshComponent* Skel2 = GetMesh();
    if (Skel2 && Skel2->DoesSocketExist(HeadSocketName))
    {
        const FTransform HeadWorld = Skel2->GetSocketTransform(HeadSocketName, RTS_World);
        const FVector PivotWorld = HeadWorld.GetLocation() + HeadOffset;
        const FVector PivotLocal = GetRootComponent()->GetComponentTransform().InverseTransformPosition(PivotWorld);
        SpringArmComponent->SetRelativeLocation(PivotLocal);
    }
}

bool APlayerCharacter::CanJumpInternal_Implementation() const
{
	return bIsCrouched || Super::CanJumpInternal_Implementation();
}

void APlayerCharacter::OnJumped_Implementation()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
}

void APlayerCharacter::StartSprint()
{
	ABTZBaseCharacter::StartSprint();
	BeginCameraMove();
}

void APlayerCharacter::StopSprint()
{
	ABTZBaseCharacter::StopSprint();
	EndCameraMove();
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	CurrentStamina = MaxStamina;
    // Ensure first-person at runtime regardless of defaults
    DefaultLength = 0.0f;
    TargetLength = 0.0f;
    SpringArmComponent->TargetArmLength = 0.0f;
    CurrentLength = 0.0f;
    // Raise the pivot (spring arm) to eye height, camera stays at zero relative location
    SpringArmEyeZStanding = GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 0.9f;
    SpringArmComponent->SetRelativeLocation(FVector(0.0f, 0.0f, SpringArmEyeZStanding));
    CameraComponent->SetRelativeLocation(FVector::ZeroVector);
    // Привязка пивота камеры к голове на старте
    {
        USkeletalMeshComponent* Skel = GetMesh();
        if (Skel && Skel->DoesSocketExist(HeadSocketName))
        {
            const FTransform HeadWorld = Skel->GetSocketTransform(HeadSocketName, RTS_World);
            const FVector PivotWorld = HeadWorld.GetLocation() + HeadOffset;
            const FVector PivotLocal = GetRootComponent()->GetComponentTransform().InverseTransformPosition(PivotWorld);
            SpringArmComponent->SetRelativeLocation(PivotLocal);
        }
        // Не переопределяем AnimInstanceClass программно, чтобы сохранить штатный AnimBP проекта
    }
    
    // Настройка IK для ног персонажа
    if (GetMesh() && bEnableFootIK)
    {
        // Устанавливаем сокеты для ног
        LeftFootSocketName = FName(TEXT("foot_l"));
        RightFootSocketName = FName(TEXT("foot_r"));
        
        // Настройка параметров IK
        IKTraceExtendDistance = 30.0f;
        IKInterpSpeed = FootIKBlendSpeed; // Используем настраиваемую скорость
        IKTraceDistance = 120.0f; // Расстояние трассировки для ног
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                FString::Printf(TEXT("PlayerCharacter: IK system initialized (Strength: %.1f, Speed: %.1f)"), 
                    FootIKStrength, FootIKBlendSpeed));
        }
    }
    
	if (IsValid(CameraCurve))
	{
		FOnTimelineFloat TimelineCallback;
		TimelineCallback.BindUFunction(this, FName("UpdateCameraMove"));
		CameraMoveTimeline.AddInterpFloat(CameraCurve, TimelineCallback);
	}

}
void APlayerCharacter::Tick(float DeltaTime)
{
	TryChangeSprintState(DeltaTime);
	BTZBaseCharMovementComponent->GetMaxSpeed();
	CameraMoveTimeline.TickTimeline(DeltaTime);
	
	// КРИТИЧЕСКИ ВАЖНО: НЕ обновляем камеру во время лазания!
	if (!ObstacleClimbingComponent || !ObstacleClimbingComponent->bIsClimbing)
	{
		// Держим пивот камеры на голове каждый тик (стоя/сидя/лежа)
		{
			USkeletalMeshComponent* Skel = GetMesh();
			if (Skel && Skel->DoesSocketExist(HeadSocketName))
			{
				const FTransform HeadWorld = Skel->GetSocketTransform(HeadSocketName, RTS_World);
				const FVector PivotWorld = HeadWorld.GetLocation() + HeadOffset;
				FVector PivotLocal = GetRootComponent()->GetComponentTransform().InverseTransformPosition(PivotWorld);
				// добавляем боковое смещение для выглядывания (локальная ось Y = вправо)
				PivotLocal += FVector(0.0f, CurrentLeanSideOffset, 0.0f);
				SpringArmComponent->SetRelativeLocation(PivotLocal);
				// и наклоняем камеру через Roll
				FRotator RelRot = SpringArmComponent->GetRelativeRotation();
				RelRot.Roll = CurrentLeanAngleDeg;
				SpringArmComponent->SetRelativeRotation(RelRot);
			}
		}
	}

    // IK переменные управляются базовым классом - синхронизация не нужна

    // Lean interpolation (roll + lateral offset on camera pivot only)
    CurrentLeanAngleDeg = FMath::FInterpTo(CurrentLeanAngleDeg, TargetLeanAngleDeg, DeltaTime, LeanInterpSpeed);
    float TargetSideOffset = (TargetLeanAngleDeg / MaxLeanAngleDeg) * MaxLeanSideOffsetCm;
    CurrentLeanSideOffset = FMath::FInterpTo(CurrentLeanSideOffset, TargetSideOffset, DeltaTime, LeanInterpSpeed);

    // Корпус и капсула остаются на месте: смещаем только пивот камеры (выше при привязке к голове)

    // Передаём угол наклона в анимграф через параметры на SkeletalMesh (Animation Blueprint может читать их как AnimBP variables)
    if (USkeletalMeshComponent* Skel = GetMesh())
    {
        // Эти имена должны совпасть с переменными в ABP_Player (Float)
        Skel->SetScalarParameterValueOnMaterials(TEXT("LeanAngleDeg"), CurrentLeanAngleDeg);
    }

	GEngine->AddOnScreenDebugMessage(0, 1.0f, FColor::Yellow, FString::Printf(TEXT("Stamina: %.2f"), CurrentStamina));
	if (!BTZBaseCharMovementComponent->IsSprinting() || !BTZBaseCharMovementComponent->IsFalling())
	{
		CurrentStamina += StaminaRestoreVelocity * DeltaTime;
		CurrentStamina = FMath::Clamp(CurrentStamina, 0.0f, MaxStamina);
	}


}

// Input setup reverted externally per project settings

// Удалено: UpdateAnimationBlueprintIK

void APlayerCharacter::UpdateCameraMove(float Value)
{
	if (!BTZBaseCharMovementComponent->IsProning())
	{
		SpringArmComponent->TargetArmLength = FMath::Lerp(DefaultLength, TargetLength, Value);
	}
}

void APlayerCharacter::SetLeanTarget(float AngleDeg)
{
    TargetLeanAngleDeg = FMath::Clamp(AngleDeg, -MaxLeanAngleDeg, MaxLeanAngleDeg);
}

void APlayerCharacter::ResetLean()
{
    TargetLeanAngleDeg = 0.0f;
}

void APlayerCharacter::TryClimbObstacle()
{
    if (ObstacleClimbingComponent)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Magenta, TEXT("PlayerCharacter: Calling obstacle climbing component"));
        }
        ObstacleClimbingComponent->TryClimb();  // ИЗМЕНЕНО: TryClimb() вместо TryClimbObstacle()
    }
    else
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("PlayerCharacter: No obstacle climbing component!"));
        }
    }
}

// Strafe Implementation (delegated to StrafeComponent)
void APlayerCharacter::StrafeRight()
{
    if (StrafeComponent)
    {
        StrafeComponent->TryStrafe(EStrafeType::Right);
    }
}

void APlayerCharacter::StrafeLeft()
{
    if (StrafeComponent)
    {
        StrafeComponent->TryStrafe(EStrafeType::Left);
    }
}

bool APlayerCharacter::CanStrafe() const
{
    if (StrafeComponent)
    {
        return StrafeComponent->CanStrafe();
    }
    return false;
}

// Input handling implementation (delegated to StrafeComponent)
void APlayerCharacter::HandleAInput(bool bPressed)
{
    if (StrafeComponent)
    {
        StrafeComponent->HandleAInput(bPressed);
    }
}

void APlayerCharacter::HandleDInput(bool bPressed)
{
    if (StrafeComponent)
    {
        StrafeComponent->HandleDInput(bPressed);
    }
}

void APlayerCharacter::HandleSpaceInput(bool bPressed)
{
    // ИСПРАВЛЕНО: Добавлена обработка Space для стрейфа
    if (StrafeComponent)
    {
        StrafeComponent->HandleSpaceInput(bPressed);
    }
    
    // Дополнительно можно обработать прыжок, если не в режиме стрейфа
    if (!bPressed && StrafeComponent && !StrafeComponent->bIsStrafing)
    {
        // Обычный прыжок, если не стрейфим
        Jump();
    }
}



