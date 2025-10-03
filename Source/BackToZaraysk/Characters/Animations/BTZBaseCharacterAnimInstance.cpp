// Fill out your copyright notice in the Description page of Project Settings.


#include "BTZBaseCharacterAnimInstance.h"
#include "../BTZBaseCharacter.h"
#include "../PlayerCharacter.h"
#include "../../Components/MovementComponents/BTZBaseCharMovementComponent.h"
#include "../../Components/StrafeComponent.h"
#include "../../Components/ObstacleClimbingComponent.h"

void UBTZBaseCharacterAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
	checkf(TryGetPawnOwner()->IsA<ABTZBaseCharacter>(), TEXT("UBTZBaseCharacterAnimInstance::NativeBeginPlay() can be used only with ABTZBaseCharacter"));
	CachedBaseCharacter = StaticCast<ABTZBaseCharacter*>(TryGetPawnOwner());
}

void UBTZBaseCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (!CachedBaseCharacter.IsValid())
	{
		return;
	}

	UBTZBaseCharMovementComponent* CharacterMovement = CachedBaseCharacter->GetBaseCharacterMovementComponent();
	Speed = CharacterMovement->Velocity.Size();
	bIsFalling = CharacterMovement->IsFalling();
	bIsCrouching = CharacterMovement->IsCrouching();

	bIsSprinting = CharacterMovement->IsSprinting();
	bIsOutOfStamina = CharacterMovement->IsOutOfStamina();
	bIsProning = CharacterMovement->IsProning();

	// Strafe animation synchronization
	if (CachedBaseCharacter.IsValid())
	{
		// Cast to PlayerCharacter to access strafe component
		if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(CachedBaseCharacter.Get()))
		{
			if (PlayerCharacter->StrafeComponent)
			{
				bIsStrafing = PlayerCharacter->StrafeComponent->bIsStrafing;
				
				// Convert EStrafeType to float direction
				switch (PlayerCharacter->StrafeComponent->CurrentStrafeType)
				{
				case EStrafeType::Left:
					StrafeDirection = -1.0f;
					break;
				case EStrafeType::Right:
					StrafeDirection = 1.0f;
					break;
				default:
					StrafeDirection = 0.0f;
					break;
				}
			}
		}
	}

    // ИСПРАВЛЕНО: Улучшенная обработка ИК данных в AnimInstance
    if (CachedBaseCharacter.IsValid())
    {
        // Получаем IK смещения от персонажа
        float LeftIKOffset = CachedBaseCharacter->GetIKLeftFootOffset();
        float RightIKOffset = CachedBaseCharacter->GetIKRightFootOffset();
        
        // Проверяем валидность данных
        if (FMath::IsFinite(LeftIKOffset) && FMath::IsFinite(RightIKOffset))
        {
            // ИСПРАВЛЕНО: Правильное применение смещений для TwoBoneIK
            // Смещения уже правильно рассчитаны в BTZBaseCharacter
            // Просто применяем их как effector locations
            LeftFootEffectorLocation = FVector(0.0f, 0.0f, LeftIKOffset);
            RightFootEffectorLocation = FVector(0.0f, 0.0f, RightIKOffset);
            
            // Улучшенная логика Alpha для более плавной работы ИК
            UBTZBaseCharMovementComponent* MovementComp = CachedBaseCharacter->GetBaseCharacterMovementComponent();
            if (MovementComp)
            {
                bool bIsOnGround = MovementComp->IsMovingOnGround();
                bool bIsMoving = MovementComp->Velocity.Size() > 5.0f;
                
                if (bIsOnGround)
                {
                    // На земле - полная сила ИК
                    float TargetAlpha = bIsMoving ? 1.0f : 0.8f; // Немного меньше когда стоит
                    LeftFootIKAlpha = FMath::FInterpTo(LeftFootIKAlpha, TargetAlpha, DeltaSeconds, 8.0f);
                    RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, TargetAlpha, DeltaSeconds, 8.0f);
                }
                else
                {
                    // В воздухе - плавно отключаем ИК
                    LeftFootIKAlpha = FMath::FInterpTo(LeftFootIKAlpha, 0.0f, DeltaSeconds, 3.0f);
                    RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, 0.0f, DeltaSeconds, 3.0f);
                }
            }
            else
            {
                // Fallback если MovementComp недоступен
                LeftFootIKAlpha = 1.0f;
                RightFootIKAlpha = 1.0f;
            }
        }
        else
        {
            // Некорректные данные - плавно сбрасываем ИК
            LeftFootEffectorLocation = FVector::ZeroVector;
            RightFootEffectorLocation = FVector::ZeroVector;
            LeftFootIKAlpha = FMath::FInterpTo(LeftFootIKAlpha, 0.0f, DeltaSeconds, 5.0f);
            RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, 0.0f, DeltaSeconds, 5.0f);
        }

        // Debug information - улучшенное отображение
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(8, 1.0f, FColor::Yellow, 
                FString::Printf(TEXT("AnimInstance IK - Left: %.2f (Alpha: %.2f), Right: %.2f (Alpha: %.2f)"),
                LeftIKOffset, LeftFootIKAlpha, RightIKOffset, RightFootIKAlpha));
        }
    }
    else
    {
        // Fallback to zero offsets - плавный сброс
        LeftFootEffectorLocation = FVector::ZeroVector;
        RightFootEffectorLocation = FVector::ZeroVector;
        LeftFootIKAlpha = FMath::FInterpTo(LeftFootIKAlpha, 0.0f, DeltaSeconds, 5.0f);
        RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, 0.0f, DeltaSeconds, 5.0f);
    }

	// === CLIMBING ANIMATION SYNCHRONIZATION (Новая система) ===
	if (CachedBaseCharacter.IsValid())
	{
		// Cast to PlayerCharacter to access climbing component
		if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(CachedBaseCharacter.Get()))
		{
			if (PlayerCharacter->ObstacleClimbingComponent)
			{
				UObstacleClimbingComponent* ClimbComp = PlayerCharacter->ObstacleClimbingComponent;
				
				// Синхронизируем основное состояние
				bIsClimbing = ClimbComp->bIsClimbing;
				
				// Устанавливаем тип лазания
				ClimbingType = (int32)ClimbComp->CurrentObstacleType;
				
				// Устанавливаем конкретные флаги анимации
				bIsVaulting = (ClimbComp->CurrentObstacleType == EObstacleType::Vault);
				bIsMantling = (ClimbComp->CurrentObstacleType == EObstacleType::Mantle);
				bIsClimbing_High = (ClimbComp->CurrentObstacleType == EObstacleType::Climb);
				
				// Debug информация (минимальная)
				if (bIsClimbing && GEngine)
				{
					GEngine->AddOnScreenDebugMessage(100, 0.1f, FColor::Green, 
						FString::Printf(TEXT("🎬 AnimInstance: Climbing | Type=%d | Vault=%d Mantle=%d Climb=%d"), 
							ClimbingType, bIsVaulting, bIsMantling, bIsClimbing_High));
				}
			}
		}
	}
}
