// Fill out your copyright notice in the Description page of Project Settings.


#include "BTZBaseCharacterAnimInstance.h"
#include "BTZFabrikLegIK.h"
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
	bIKLocomotionActive = false;
	IKStartDelayRemaining = 0.0f;
	IKGateAlpha = 0.0f;
	LeftFootIKJointTargetLocation = FVector::ZeroVector;
	RightFootIKJointTargetLocation = FVector::ZeroVector;
	ShouldDoIKTrace = false;
	ZOffset_L = ZOffset_R = ZOffset_Pelvis = 0.0f;
	IKBlend_l = IKBlend_r = 0.0f;
}

void UBTZBaseCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (!CachedBaseCharacter.IsValid())
	{
		LeftFootIKJointTargetLocation = FVector::ZeroVector;
		RightFootIKJointTargetLocation = FVector::ZeroVector;
		ShouldDoIKTrace = false;
		ZOffset_L = ZOffset_R = ZOffset_Pelvis = 0.0f;
		IKBlend_l = IKBlend_r = 0.0f;
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

		const float Speed2D = CharacterMovement ? CharacterMovement->Velocity.Size2D() : 0.0f;
		const bool bGrounded = CharacterMovement && CharacterMovement->IsMovingOnGround();
		// Stair mode uses character-side hysteresis on *raw* traces (avoids Anim/CMC desync and borderline flicker).
		const bool bStairLikeFeet = bGrounded && CachedBaseCharacter->IsFootIKStairLike();

		// Apply per-character strength multiplier (from PlayerCharacter)
		float StrengthMul = 1.0f;
		if (APlayerCharacter* PC = Cast<APlayerCharacter>(CachedBaseCharacter.Get()))
		{
			StrengthMul = FMath::Max(0.0f, PC->FootIKStrength);
		}
		LeftIKOffset *= StrengthMul;
		RightIKOffset *= StrengthMul;

		// Locomotion gate with hysteresis + start delay:
		// prevents IK from overreacting on very short taps/starts.
		if (bGrounded)
		{
			if (!bIKLocomotionActive && (Speed2D >= IKStartSpeedThreshold || bStairLikeFeet))
			{
				bIKLocomotionActive = true;
				IKStartDelayRemaining = bStairLikeFeet ? 0.0f : IKStartDelaySeconds;
			}
			else if (bIKLocomotionActive && Speed2D <= IKStopSpeedThreshold && !bStairLikeFeet)
			{
				bIKLocomotionActive = false;
				IKStartDelayRemaining = 0.0f;
			}
		}
		else
		{
			bIKLocomotionActive = false;
			IKStartDelayRemaining = 0.0f;
		}

		if (IKStartDelayRemaining > 0.0f)
		{
			IKStartDelayRemaining = FMath::Max(0.0f, IKStartDelayRemaining - DeltaSeconds);
		}
		if (bStairLikeFeet)
		{
			IKStartDelayRemaining = 0.0f;
			bIKLocomotionActive = true;
		}

		const bool bGateOpen = bGrounded && bIKLocomotionActive && IKStartDelayRemaining <= 0.0f;
		// Full IK on stairs / split stance while idle (otherwise gate -> 0 fights animation at low alpha).
		const float IKGateTarget = (bGrounded && (bGateOpen || bStairLikeFeet)) ? 1.0f : 0.0f;
		IKGateAlpha = FMath::FInterpTo(IKGateAlpha, IKGateTarget, DeltaSeconds, IKGateBlendSpeed);
		const auto ComputeTargetIKAlpha = [&](float InSpeed2D) -> float
		{
			float Alpha = 1.0f;
			if (InSpeed2D > 220.0f) Alpha = 0.22f;
			else if (InSpeed2D > 90.0f) Alpha = 0.48f;

			if (bSimplifyIKAtRun && !bStairLikeFeet)
			{
				const float FullSpeed = FMath::Max(IKRunSimplifyFullSpeed, IKRunSimplifyStartSpeed + 1.0f);
				const float T = FMath::GetMappedRangeValueClamped(
					FVector2D(IKRunSimplifyStartSpeed, FullSpeed),
					FVector2D(0.0f, 1.0f),
					InSpeed2D
				);
				const float SimplifiedAlpha = FMath::Lerp(1.0f, IKMinAlphaAtFullRun, T);
				Alpha = FMath::Min(Alpha, SimplifiedAlpha);
			}
			// Standing on uneven ground: keep IK strong even at Vel=0.
			if (bStairLikeFeet && InSpeed2D <= IKStopSpeedThreshold)
			{
				Alpha = 1.0f;
			}
			return FMath::Clamp(Alpha * IKGateAlpha, 0.0f, 1.0f);
		};

		// At sprint, follow foot targets more slowly to kill high-frequency twitch.
		// At run, always ease effector follow (reduces twitch). On stairs, add extra damping — skipping run ease was too snappy.
		float EffectorInterp = FootIKEffectorInterpSpeed;
		if (Speed2D > RunEffectorBlendStartSpeed)
		{
			const float FullSpd = FMath::Max(RunEffectorBlendFullSpeed, RunEffectorBlendStartSpeed + 1.0f);
			const float TEff = FMath::GetMappedRangeValueClamped(
				FVector2D(RunEffectorBlendStartSpeed, FullSpd),
				FVector2D(0.0f, 1.0f),
				Speed2D
			);
			EffectorInterp *= FMath::Lerp(1.0f, RunEffectorInterpMinScale, TEff);
		}
		// На ступеньках при ходьбе/беге — чуть глушим эффектор; стоя (низкая скорость) — полная скорость, чёткий контакт.
		if (bStairLikeFeet && Speed2D > StairEffectorSlowMinSpeed2D)
		{
			EffectorInterp *= StairFootEffectorInterpMul;
		}
		const float EffectorInterpRight = EffectorInterp * ((Speed2D > RunEffectorBlendStartSpeed) ? RightFootEffectorSmoothMulAtRun : 1.0f);

		// Kill tiny jitter on flat ground; on stairs use a smaller dead zone so both feet can diverge.
		const float DeadScale = bStairLikeFeet ? StairFootOffsetDeadZoneScale : 1.0f;
		const float DeadL = FootOffsetDeadZone * DeadScale;
		const float DeadR = FootOffsetDeadZone * DeadScale;
		if (FMath::Abs(LeftIKOffset) < DeadL) LeftIKOffset = 0.0f;
		if (FMath::Abs(RightIKOffset) < DeadR) RightIKOffset = 0.0f;
        
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

				// Use character trace origins (stable XY) to avoid IK->socket feedback loop
				// that can gradually pull feet toward center when idling.
				FVector LWorld = CachedBaseCharacter->GetFootTraceOriginWorld(true);
				FVector RWorld = CachedBaseCharacter->GetFootTraceOriginWorld(false);

				const float LOriginZ = LWorld.Z;
				const float ROriginZ = RWorld.Z;

				// Z: trace plane + IK offset + подъёмы (подошва / коллизия / ступеньки)
				const float TotalZLift = FootIKHeelLift + FootIKSoleClearanceCm
					+ (bStairLikeFeet ? FootIKStairExtraZLiftCm : 0.0f);
				LWorld.Z = LOriginZ + LeftIKOffset + TotalZLift;
				RWorld.Z = ROriginZ + RightIKOffset + TotalZLift;

				// Цель IK чуть ближе к пятке (вдоль локального X стопы): меньше постановки «на мысках».
				if (!FMath::IsNearlyZero(FootIKEffectorRetreatToHeelCm))
				{
					auto ApplyHeelRetreat = [&](FVector& FootWorld, const FName& SockName)
					{
						FName UseName = SockName;
						if (UseName == FName(TEXT("ik_foot_l"))) UseName = FName(TEXT("foot_l"));
						else if (UseName == FName(TEXT("ik_foot_r"))) UseName = FName(TEXT("foot_r"));
						if (!UseName.IsNone() && MeshComp->DoesSocketExist(UseName))
						{
							const FTransform FootXf = MeshComp->GetSocketTransform(UseName, RTS_World);
							FootWorld -= FootXf.GetUnitAxis(EAxis::X) * FootIKEffectorRetreatToHeelCm;
						}
					};
					ApplyHeelRetreat(LWorld, LName);
					ApplyHeelRetreat(RWorld, RName);
				}

				const FTransform CompXf = MeshComp->GetComponentTransform();

				// Joint Target: FABRIK на цепи бедро–колено–стопа (см. Habr FABRIK), затем колено в плоскости (hip, foot, pole).
				{
					const FVector ActorFwd = CachedBaseCharacter->GetActorForwardVector().GetSafeNormal();
					const FVector CharRight = CachedBaseCharacter->GetActorRightVector().GetSafeNormal();
					const FVector UpW = FVector::UpVector;
					const float ForwardPull = FootIKKneePoleForwardCm + (bStairLikeFeet ? FootIKKneePoleStairForwardAddCm : 0.0f);

					auto AnatomicalFootSocket = [&](const FName& ConfigFootName) -> FName
					{
						if (ConfigFootName == FName(TEXT("ik_foot_l"))) return FName(TEXT("foot_l"));
						if (ConfigFootName == FName(TEXT("ik_foot_r"))) return FName(TEXT("foot_r"));
						return ConfigFootName;
					};

					auto FabrikKneeComponentSpace = [&](bool bLeftFoot, const FVector& FootTargetW) -> FVector
					{
						const FName ThighSock = bLeftFoot ? LeftThighSocketForIK : RightThighSocketForIK;
						const FName KneeSock = bLeftFoot ? LeftKneeRefSocketForIK : RightKneeRefSocketForIK;
						const FName FootSock = AnatomicalFootSocket(bLeftFoot ? LName : RName);

						if (ThighSock.IsNone() || KneeSock.IsNone() || FootSock.IsNone()
							|| !MeshComp->DoesSocketExist(ThighSock) || !MeshComp->DoesSocketExist(KneeSock)
							|| !MeshComp->DoesSocketExist(FootSock))
						{
							const FVector& FW = bLeftFoot ? LWorld : RWorld;
							const FVector HintW = FW + ActorFwd * 8.0f + UpW * 12.0f;
							return CompXf.InverseTransformPosition(HintW);
						}

						const FVector HipW = MeshComp->GetSocketLocation(ThighSock);
						const FVector KneeW = MeshComp->GetSocketLocation(KneeSock);

						float UpperLen = FVector::Distance(HipW, KneeW);
						float LowerLen = FVector::Distance(KneeW, MeshComp->GetSocketLocation(FootSock));
						if (UpperLen < 1.0f || LowerLen < 1.0f)
						{
							const FVector HintW = KneeW + ActorFwd * 10.0f + UpW * FootIKKneePoleUpCm;
							return CompXf.InverseTransformPosition(HintW);
						}

						const FVector LegVec = FootTargetW - HipW;
						const float LegLenSq = LegVec.SizeSquared();
						FVector PoleDir = ActorFwd;
						FVector LegDir = ActorFwd;
						if (LegLenSq >= 1.0f)
						{
							LegDir = LegVec * FMath::InvSqrt(LegLenSq);
							PoleDir = ActorFwd - LegDir * FVector::DotProduct(ActorFwd, LegDir);
							if (!PoleDir.Normalize())
							{
								PoleDir = FVector::CrossProduct(UpW, LegDir).GetSafeNormal();
							}
							if (FVector::DotProduct(PoleDir, ActorFwd) < 0.0f)
							{
								PoleDir *= -1.0f;
							}
						}
						else
						{
							LegDir = ActorFwd;
						}

						// Направление стопы в мире (как у retreat к пятке): ось X сокета foot — «вперёд» стопы.
						const FTransform FootXfW = MeshComp->GetSocketTransform(FootSock, RTS_World);
						FVector FootFwdW = FootXfW.GetUnitAxis(EAxis::X);
						FVector FootFwdPlanar = FootFwdW - LegDir * FVector::DotProduct(FootFwdW, LegDir);
						if (FootFwdPlanar.Normalize())
						{
							// Та же полусфера, что и у actor-pole, чтобы не переворачивать колено через 180°.
							if (FVector::DotProduct(FootFwdPlanar, PoleDir) < 0.0f)
							{
								FootFwdPlanar *= -1.0f;
							}
							const float B = FMath::Clamp(FootIKKneePoleFootForwardBlend, 0.0f, 1.0f);
							const FVector Blended = FVector::VectorPlaneProject(
								PoleDir * (1.0f - B) + FootFwdPlanar * B,
								LegDir
							).GetSafeNormal();
							if (Blended.SizeSquared() > 0.01f)
							{
								// Не форсируем ActorForward после бленда — иначе ломается «курс стопы».
								PoleDir = Blended;
							}
						}

						const float SideSign = bLeftFoot ? -1.0f : 1.0f;
						const bool bThisFootHigher = bLeftFoot
							? (FootTargetW.Z > RWorld.Z + 2.0f)
							: (FootTargetW.Z > LWorld.Z + 2.0f);
						const float Outward = FootIKKneePoleRightCm * SideSign
							+ (bThisFootHigher ? FootIKKneePoleOutwardWhenHigherCm * SideSign : 0.0f);

						FVector PoleW = KneeW + PoleDir * ForwardPull + UpW * FootIKKneePoleUpCm + CharRight * Outward;

						// Смещение pole вдоль курса стопы (в плоскости ┴ на бедро–стопа).
						if (FootIKKneePoleAlongFootCm > KINDA_SMALL_NUMBER)
						{
							FVector AlongFoot = FootFwdW - LegDir * FVector::DotProduct(FootFwdW, LegDir);
							if (AlongFoot.Normalize())
							{
								if (FVector::DotProduct(AlongFoot, PoleDir) < 0.0f)
								{
									AlongFoot *= -1.0f;
								}
								PoleW += AlongFoot * FootIKKneePoleAlongFootCm;
							}
						}

						const FVector SolvedKneeW = FBTZFabrikLegIK::SolveKneeWorld(
							HipW,
							KneeW,
							FootTargetW,
							UpperLen,
							LowerLen,
							PoleW,
							FootIKFabrikMaxIterations,
							FootIKFabrikToleranceCm);

						return CompXf.InverseTransformPosition(SolvedKneeW);
					};

					LeftFootIKJointTargetLocation = FabrikKneeComponentSpace(true, LWorld);
					RightFootIKJointTargetLocation = FabrikKneeComponentSpace(false, RWorld);
				}

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
						LeftFootEffectorLocation = FMath::VInterpTo(LeftFootEffectorLocation, LTarget, DeltaSeconds, EffectorInterp);
						RightFootEffectorLocation = FMath::VInterpTo(RightFootEffectorLocation, RTarget, DeltaSeconds, EffectorInterpRight);

						// No plant curves detected: keep IK strong while idle, but reduce while moving
						// so gait animation can drive knees/hips naturally.
						const float TargetAlpha = ComputeTargetIKAlpha(Speed2D);

						LeftFootIKAlpha = FMath::FInterpTo(LeftFootIKAlpha, TargetAlpha, DeltaSeconds, 10.0f);
						RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, TargetAlpha, DeltaSeconds, 10.0f);
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

					LeftFootEffectorLocation = FMath::VInterpTo(LeftFootEffectorLocation, LDesired, DeltaSeconds, EffectorInterp);
					RightFootEffectorLocation = FMath::VInterpTo(RightFootEffectorLocation, RDesired, DeltaSeconds, EffectorInterpRight);

					// Alpha: planted => ~1, swing => scaled down (often 0)
					const float AlphaLTarget = FMath::Lerp(SwingIKAlphaScale, 1.0f, PlantL);
					const float AlphaRTarget = FMath::Lerp(SwingIKAlphaScale, 1.0f, PlantR);
					LeftFootIKAlpha = FMath::FInterpTo(LeftFootIKAlpha, AlphaLTarget, DeltaSeconds, 12.0f);
					RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, AlphaRTarget, DeltaSeconds, 12.0f);
					}
				}
				else
				{
					// Fallback: no curves (or not configured).
					// Keep full IK at idle and soften it in locomotion to avoid "frozen legs".
					bLeftFootLocked = false;
					bRightFootLocked = false;
					LeftFootEffectorLocation = FMath::VInterpTo(LeftFootEffectorLocation, LTarget, DeltaSeconds, EffectorInterp);
					RightFootEffectorLocation = FMath::VInterpTo(RightFootEffectorLocation, RTarget, DeltaSeconds, EffectorInterpRight);

					const float TargetAlpha = ComputeTargetIKAlpha(Speed2D);

					LeftFootIKAlpha = FMath::FInterpTo(LeftFootIKAlpha, TargetAlpha, DeltaSeconds, 10.0f);
					RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, TargetAlpha, DeltaSeconds, 10.0f);
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
				LeftFootIKJointTargetLocation = FVector::ZeroVector;
				RightFootIKJointTargetLocation = FVector::ZeroVector;
            }

			// Pelvis correction AFTER IK alphas are updated this frame.
			// Using weighted offsets prevents stair feedback issues caused by stale alpha values.
			{
				const float EffectiveLeftOffset = (FMath::IsFinite(LeftFootIKAlpha) ? LeftIKOffset * LeftFootIKAlpha : 0.0f);
				const float EffectiveRightOffset = (FMath::IsFinite(RightFootIKAlpha) ? RightIKOffset * RightFootIKAlpha : 0.0f);

				float LEff = EffectiveLeftOffset;
				float REff = EffectiveRightOffset;

				const float PelvisDead = PelvisOffsetDeadZone * (bStairLikeFeet ? PelvisOffsetDeadZoneStairScale : 1.0f);
				if (FMath::Abs(LEff) < PelvisDead) LEff = 0.0f;
				if (FMath::Abs(REff) < PelvisDead) REff = 0.0f;

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
			if (!bGrounded)
			{
				// Hard-off in air to avoid spiral/twist artifacts on jump/fall frames.
				LeftFootIKAlpha = 0.0f;
				RightFootIKAlpha = 0.0f;
				bLeftFootLocked = false;
				bRightFootLocked = false;
				PelvisOffsetZ = 0.0f;
				LeftFootIKJointTargetLocation = FVector::ZeroVector;
				RightFootIKJointTargetLocation = FVector::ZeroVector;
				// Epic Mannequin (ABP_Manny / CR_Mannequin_BasicFootIK)
				ShouldDoIKTrace = false;
				ZOffset_L = ZOffset_R = ZOffset_Pelvis = 0.0f;
				IKBlend_l = IKBlend_r = 0.0f;
			}
			else
			{
				// Flat idle lockout:
				// when character is nearly stopped and offsets are tiny, disable IK influence
				// so feet don't get gradually pulled toward center.
				const bool bNearIdle = Speed2D <= 8.0f;
				const bool bFlatOffsets = FMath::Abs(LeftIKOffset) <= 1.5f && FMath::Abs(RightIKOffset) <= 1.5f;
				if (bNearIdle && bFlatOffsets && !bStairLikeFeet)
				{
					LeftFootIKAlpha = 0.0f;
					RightFootIKAlpha = 0.0f;
					PelvisOffsetZ = FMath::FInterpTo(PelvisOffsetZ, 0.0f, DeltaSeconds, PelvisIKInterpSpeed);
					bLeftFootLocked = false;
					bRightFootLocked = false;
				}

				// Epic Mannequin mirrors — те же имена, что в Control Rig / шаблоне Third Person.
				bool bClimbIKBlock = false;
				if (APlayerCharacter* PC = Cast<APlayerCharacter>(CachedBaseCharacter.Get()))
				{
					if (PC->ObstacleClimbingComponent)
					{
						bClimbIKBlock = PC->ObstacleClimbingComponent->bIsClimbing;
					}
				}
				ShouldDoIKTrace = !bIsSwimming && !bClimbIKBlock;
				ZOffset_L = LeftIKOffset;
				ZOffset_R = RightIKOffset;
				ZOffset_Pelvis = PelvisOffsetZ;
				IKBlend_l = LeftFootIKAlpha;
				IKBlend_r = RightFootIKAlpha;
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
			LeftFootIKJointTargetLocation = FVector::ZeroVector;
			RightFootIKJointTargetLocation = FVector::ZeroVector;
			ShouldDoIKTrace = false;
			ZOffset_L = ZOffset_R = ZOffset_Pelvis = 0.0f;
			IKBlend_l = IKBlend_r = 0.0f;
        }

        // Debug information - улучшенное отображение
        if (CachedBaseCharacter->bDebugFootIK && GEngine)
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
		LeftFootIKJointTargetLocation = FVector::ZeroVector;
		RightFootIKJointTargetLocation = FVector::ZeroVector;
		ShouldDoIKTrace = false;
		ZOffset_L = ZOffset_R = ZOffset_Pelvis = 0.0f;
		IKBlend_l = IKBlend_r = 0.0f;
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
