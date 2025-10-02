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

    // Climbing animation synchronization
    if (CachedBaseCharacter.IsValid())
    {
        // Try to get ObstacleClimbingComponent from base character
        ABTZBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
        if (BaseCharacter)
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Orange, 
                    TEXT("AnimInstance: BaseCharacter found"));
            }
            
            // Cast to PlayerCharacter to access climbing component
            if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(BaseCharacter))
            {
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Orange, 
                        TEXT("AnimInstance: PlayerCharacter cast SUCCESS"));
                }
                
                if (PlayerCharacter->ObstacleClimbingComponent)
                {
                    if (GEngine)
                    {
                        GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Orange, 
                            TEXT("AnimInstance: ObstacleClimbingComponent found"));
                    }
                    
                    bIsClimbing = PlayerCharacter->ObstacleClimbingComponent->bIsClimbing;
                
                // Debug: show current climb type
                if (GEngine)
                {
                    int32 ClimbTypeValue = (int32)PlayerCharacter->ObstacleClimbingComponent->CurrentClimbType;
                    GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, 
                        FString::Printf(TEXT("AnimInstance: CurrentClimbType = %d"), ClimbTypeValue));
                }
                
                // Set specific climbing animation states based on climb type
                switch (PlayerCharacter->ObstacleClimbingComponent->CurrentClimbType)
                {
                case EObstacleClimbType::Vault:
                    bIsVaulting = true;
                    bIsMantling = false;
                    bIsLedgeClimbing = false;
                    if (GEngine)
                    {
                        GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Cyan, 
                            TEXT("AnimInstance: bIsVaulting = TRUE"));
                    }
                    break;
                case EObstacleClimbType::Climb:
                    bIsVaulting = false;
                    bIsMantling = true;
                    bIsLedgeClimbing = false;
                    if (GEngine)
                    {
                        GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Cyan, 
                            TEXT("AnimInstance: bIsMantling = TRUE"));
                    }
                    break;
                case EObstacleClimbType::ClimbOver:
                    bIsVaulting = false;
                    bIsMantling = false;
                    bIsLedgeClimbing = true;
                    if (GEngine)
                    {
                        GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Cyan, 
                            TEXT("AnimInstance: bIsLedgeClimbing = TRUE"));
                    }
                    break;
                default:
                    bIsVaulting = false;
                    bIsMantling = false;
                    bIsLedgeClimbing = false;
                    break;
                }
                
                // Set climbing speed and height for animation blending
                if (bIsClimbing)
                {
                    // New climb started - reset everything
                    ClimbingSpeed = CharacterMovement->Velocity.Size();
                    ClimbingHeight = PlayerCharacter->ObstacleClimbingComponent->CurrentObstacle.ObstacleHeight;
                    bWasClimbing = true;
                    ClimbingAnimationResetTimer = 0.0f;
                    
                    if (GEngine)
                    {
                        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, 
                            TEXT("AnimInstance: New climb started, reset timer"));
                    }
                }
                else if (bWasClimbing)
                {
                    // Climbing just finished, keep animation state active
                    ClimbingAnimationResetTimer += DeltaSeconds;
                    
                    if (GEngine)
                    {
                        GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Magenta, 
                            FString::Printf(TEXT("AnimInstance: Reset timer = %.2f"), ClimbingAnimationResetTimer));
                    }
                    
                    // Keep animation states active for 2 seconds (animation duration)
                    if (ClimbingAnimationResetTimer >= 2.0f)
                    {
                        ClimbingSpeed = 0.0f;
                        ClimbingHeight = 0.0f;
                        bIsVaulting = false;
                        bIsMantling = false;
                        bIsLedgeClimbing = false;
                        bWasClimbing = false;
                        ClimbingAnimationResetTimer = 0.0f;
                        
                        // Force update locomotion variables
                        if (CharacterMovement)
                        {
                            Speed = CharacterMovement->Velocity.Size();
                            bIsFalling = CharacterMovement->IsFalling();
                        }
                        
                        if (GEngine)
                        {
                            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, 
                                FString::Printf(TEXT("AnimInstance: RESET, Speed=%.1f"), Speed));
                        }
                    }
                }
                }
            }
        }
    }
}
