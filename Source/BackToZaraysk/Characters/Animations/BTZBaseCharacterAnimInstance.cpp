// Fill out your copyright notice in the Description page of Project Settings.


#include "BTZBaseCharacterAnimInstance.h"
#include "../BTZBaseCharacter.h"
#include "../PlayerCharacter.h"
#include "../../Components/MovementComponents/BTZBaseCharMovementComponent.h"
#include "../../Components/StrafeComponent.h"
#include "../../Components/ObstacleClimbingComponent.h"

static float GetMaxCurveValue(const UAnimInstance* Anim, const TArray<FName>& CurveNames)
{
	if (!Anim) return 0.0f;
	float V = 0.0f;
	for (const FName& N : CurveNames)
	{
		if (N.IsNone()) continue;
		V = FMath::Max(V, Anim->GetCurveValue(N));
	}
	return V;
}

void UBTZBaseCharacterAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
	checkf(TryGetPawnOwner()->IsA<ABTZBaseCharacter>(), TEXT("UBTZBaseCharacterAnimInstance::NativeBeginPlay() can be used only with ABTZBaseCharacter"));
	CachedBaseCharacter = StaticCast<ABTZBaseCharacter*>(TryGetPawnOwner());

	// PIE/startup guard:
	// start with IK disabled and effector/pelvis offsets at zero to prevent a single-frame "drop".
	LeftFootEffectorLocation = FVector::ZeroVector;
	RightFootEffectorLocation = FVector::ZeroVector;
	PelvisOffsetZ = 0.0f;
	LeftFootIKAlpha = 0.0f;
	RightFootIKAlpha = 0.0f;
	bLeftFootLocked = false;
	bRightFootLocked = false;
	LockedLeftEffector = FVector::ZeroVector;
	LockedRightEffector = FVector::ZeroVector;
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
	bIsSwimming = (CharacterMovement->MovementMode == MOVE_Swimming);
	if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(CachedBaseCharacter.Get()))
	{
		// Our swimming implementation uses MOVE_Flying for 3D movement in water
		bIsSwimming = bIsSwimming || PlayerCharacter->IsSwimmingActive();
	}
	// Placeholder animation: while swimming, reuse prone/crawl state machine
	bIsProning = CharacterMovement->IsProning() || bIsSwimming;

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
        float LeftIKOffsetRaw = CachedBaseCharacter->GetIKLeftFootOffset();
        float RightIKOffsetRaw = CachedBaseCharacter->GetIKRightFootOffset();
        float LeftIKOffset = LeftIKOffsetRaw;
        float RightIKOffset = RightIKOffsetRaw;

		// Apply per-character strength multiplier (from PlayerCharacter)
		float StrengthMul = 1.0f;
		if (APlayerCharacter* PC = Cast<APlayerCharacter>(CachedBaseCharacter.Get()))
		{
			StrengthMul = FMath::Max(0.0f, PC->FootIKStrength);
		}
		LeftIKOffset *= StrengthMul;
		RightIKOffset *= StrengthMul;

		// Kill tiny jitter on flat ground (keeps stairs working)
		if (FMath::Abs(LeftIKOffset) < FootOffsetDeadZone) LeftIKOffset = 0.0f;
		if (FMath::Abs(RightIKOffset) < FootOffsetDeadZone) RightIKOffset = 0.0f;
        
        // Проверяем валидность данных
        if (FMath::IsFinite(LeftIKOffset) && FMath::IsFinite(RightIKOffset))
        {
			// We compute pelvis target AFTER IK alphas are updated this frame (order matters).
			float PelvisTarget = 0.0f;

            // Stable Foot IK targets for ABP (Component Space):
            // XY from foot socket, Z reconstructed as (TraceOriginZ + IKOffset + HeelLift).
            if (USkeletalMeshComponent* MeshComp = GetSkelMeshComponent())
            {
                const FName LName = CachedBaseCharacter->GetLeftFootSocketName();
                const FName RName = CachedBaseCharacter->GetRightFootSocketName();

				FVector LWorld = MeshComp->GetSocketLocation(LName);
				FVector RWorld = MeshComp->GetSocketLocation(RName);

				const float LOriginZ = CachedBaseCharacter->GetFootTraceOriginWorld(true).Z;
				const float ROriginZ = CachedBaseCharacter->GetFootTraceOriginWorld(false).Z;

				// Z = TraceOriginZ + IKOffset + HeelLift => should match HitZ + HeelLift
				LWorld.Z = LOriginZ + LeftIKOffset + FootIKHeelLift;
				RWorld.Z = ROriginZ + RightIKOffset + FootIKHeelLift;

                const FTransform CompXf = MeshComp->GetComponentTransform();
                FVector LTarget = CompXf.InverseTransformPosition(LWorld);
                FVector RTarget = CompXf.InverseTransformPosition(RWorld);

				const bool bForceIKOnDebug = CachedBaseCharacter->bDebugFootIK;

				if (bUseFootPlantCurves && !bForceIKOnDebug)
				{
					// Foot plant / lock from animation curves
					const float PlantL = FMath::Clamp(GetMaxCurveValue(this, LeftFootPlantCurveNames), 0.0f, 1.0f);
					const float PlantR = FMath::Clamp(GetMaxCurveValue(this, RightFootPlantCurveNames), 0.0f, 1.0f);

					// If curves are enabled but both feet have ~0 plant, it's usually a sign that
					// curves aren't present/working for this anim set.
					// In that case we fall back to always-on IK while grounded.
					const bool bAnyPlant = (PlantL > 0.001f) || (PlantR > 0.001f);

					if (!bAnyPlant)
					{
						bLeftFootLocked = false;
						bRightFootLocked = false;
						LeftFootEffectorLocation = FMath::VInterpTo(LeftFootEffectorLocation, LTarget, DeltaSeconds, FootIKEffectorInterpSpeed);
						RightFootEffectorLocation = FMath::VInterpTo(RightFootEffectorLocation, RTarget, DeltaSeconds, FootIKEffectorInterpSpeed);
						LeftFootIKAlpha = FMath::FInterpTo(LeftFootIKAlpha, 1.0f, DeltaSeconds, 10.0f);
						RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, 1.0f, DeltaSeconds, 10.0f);
					}
					else
					{
					// Hysteresis state updates
					if (!bLeftFootLocked && PlantL >= FootLockThreshold)
					{
						bLeftFootLocked = true;
						LockedLeftEffector = LTarget;
					}
					else if (bLeftFootLocked && PlantL <= FootUnlockThreshold)
					{
						bLeftFootLocked = false;
					}

					if (!bRightFootLocked && PlantR >= FootLockThreshold)
					{
						bRightFootLocked = true;
						LockedRightEffector = RTarget;
					}
					else if (bRightFootLocked && PlantR <= FootUnlockThreshold)
					{
						bRightFootLocked = false;
					}

					const FVector LDesired = bLeftFootLocked ? LockedLeftEffector : LTarget;
					const FVector RDesired = bRightFootLocked ? LockedRightEffector : RTarget;

					LeftFootEffectorLocation = FMath::VInterpTo(LeftFootEffectorLocation, LDesired, DeltaSeconds, FootIKEffectorInterpSpeed);
					RightFootEffectorLocation = FMath::VInterpTo(RightFootEffectorLocation, RDesired, DeltaSeconds, FootIKEffectorInterpSpeed);

					// Alpha: planted => ~1, swing => scaled down (often 0)
					const float AlphaLTarget = FMath::Lerp(SwingIKAlphaScale, 1.0f, PlantL);
					const float AlphaRTarget = FMath::Lerp(SwingIKAlphaScale, 1.0f, PlantR);
					LeftFootIKAlpha = FMath::FInterpTo(LeftFootIKAlpha, AlphaLTarget, DeltaSeconds, 12.0f);
					RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, AlphaRTarget, DeltaSeconds, 12.0f);
					}
				}
				else
				{
					// Fallback: no curves (or not configured) -> always-on IK while grounded
					bLeftFootLocked = false;
					bRightFootLocked = false;
					LeftFootEffectorLocation = FMath::VInterpTo(LeftFootEffectorLocation, LTarget, DeltaSeconds, FootIKEffectorInterpSpeed);
					RightFootEffectorLocation = FMath::VInterpTo(RightFootEffectorLocation, RTarget, DeltaSeconds, FootIKEffectorInterpSpeed);
					LeftFootIKAlpha = FMath::FInterpTo(LeftFootIKAlpha, 1.0f, DeltaSeconds, 10.0f);
					RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, 1.0f, DeltaSeconds, 10.0f);
				}
            }
            else
            {
                LeftFootEffectorLocation = FVector::ZeroVector;
                RightFootEffectorLocation = FVector::ZeroVector;
				LeftFootIKAlpha = FMath::FInterpTo(LeftFootIKAlpha, 0.0f, DeltaSeconds, 8.0f);
				RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, 0.0f, DeltaSeconds, 8.0f);
				bLeftFootLocked = false;
				bRightFootLocked = false;
            }

			// Pelvis correction AFTER IK alphas are updated this frame.
			// Using weighted offsets prevents stair feedback issues caused by stale alpha values.
			{
				const float EffectiveLeftOffset = (FMath::IsFinite(LeftFootIKAlpha) ? LeftIKOffset * LeftFootIKAlpha : 0.0f);
				const float EffectiveRightOffset = (FMath::IsFinite(RightFootIKAlpha) ? RightIKOffset * RightFootIKAlpha : 0.0f);

				float LEff = EffectiveLeftOffset;
				float REff = EffectiveRightOffset;

				if (FMath::Abs(LEff) < PelvisOffsetDeadZone) LEff = 0.0f;
				if (FMath::Abs(REff) < PelvisOffsetDeadZone) REff = 0.0f;

				// Pelvis is shared for both legs, so to keep foot corrections independent
				// we only move pelvis down when BOTH feet require downward correction.
				// If only one foot has an offset (stair-like unilateral change), we keep
				// PelvisTarget at 0 so the other leg doesn't get dragged by a shared pelvis move.
				if (LEff < 0.0f && REff < 0.0f)
				{
					PelvisTarget = FMath::Min(LEff, REff);
					PelvisTarget = FMath::Clamp(PelvisTarget, PelvisOffsetMinZ, 0.0f);
				}
				else
				{
					PelvisTarget = 0.0f;
				}
				PelvisOffsetZ = FMath::FInterpTo(PelvisOffsetZ, PelvisTarget, DeltaSeconds, PelvisIKInterpSpeed);

				// Do NOT compensate effector Z by PelvisOffsetZ here.
				// The pelvis offset should change the skeleton, while the foot effector
				// stays anchored to the ground contact so TwoBoneIK can bend knees.
			}

			// Debug (controlled by character flag)
			if (CachedBaseCharacter->bDebugFootIK && GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					920,
					0.0f,
					FColor::Cyan,
					FString::Printf(
						TEXT("IKDbg RawL=%.2f RawR=%.2f Str=%.2f DZf=%.2f L=%.2f R=%.2f Heel=%.1f EffLz=%.2f EffRz=%.2f PelT=%.2f Pel=%.2f DZp=%.1f PelMin=%.1f"),
						LeftIKOffsetRaw, RightIKOffsetRaw,
						StrengthMul, FootOffsetDeadZone,
						LeftIKOffset, RightIKOffset,
						FootIKHeelLift,
						LeftFootEffectorLocation.Z, RightFootEffectorLocation.Z,
						PelvisTarget, PelvisOffsetZ,
						PelvisOffsetDeadZone, PelvisOffsetMinZ
					)
				);
			}
            
			// Disable IK only when we are not grounded.
			// This keeps foot placement working on non-terrain walkable geometry (ramps/props).
			if (CharacterMovement && !CharacterMovement->IsMovingOnGround())
			{
				LeftFootIKAlpha = FMath::FInterpTo(LeftFootIKAlpha, 0.0f, DeltaSeconds, 6.0f);
				RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, 0.0f, DeltaSeconds, 6.0f);
				bLeftFootLocked = false;
				bRightFootLocked = false;
				PelvisOffsetZ = FMath::FInterpTo(PelvisOffsetZ, 0.0f, DeltaSeconds, PelvisIKInterpSpeed);
			}
        }
        else
        {
            // Некорректные данные - плавно сбрасываем ИК
            LeftFootEffectorLocation = FVector::ZeroVector;
            RightFootEffectorLocation = FVector::ZeroVector;
            PelvisOffsetZ = 0.0f;
            LeftFootIKAlpha = FMath::FInterpTo(LeftFootIKAlpha, 0.0f, DeltaSeconds, 5.0f);
            RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, 0.0f, DeltaSeconds, 5.0f);
			bLeftFootLocked = false;
			bRightFootLocked = false;
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
