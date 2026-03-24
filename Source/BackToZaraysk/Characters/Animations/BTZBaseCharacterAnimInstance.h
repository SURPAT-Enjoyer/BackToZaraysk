// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "BTZBaseCharacterAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class BACKTOZARAYSK_API UBTZBaseCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character animation")
	float Speed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character animation")
	bool bIsFalling = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character animation")
	bool bIsCrouching = false;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character animation")
    bool bIsProning = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character animation")
	bool bIsSprinting = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character animation||Stamina")
	bool bIsOutOfStamina = false;

	UPROPERTY(BlueprintReadOnly, Category = "IK Settings")
	FVector RightFootEffectorLocation = FVector::ZeroVector;
	
	/** Component space — Two Bone IK → Joint Target Location. Считается через FABRIK (2 кости) + плоскость pole. */
	UPROPERTY(BlueprintReadOnly, Category = "IK Settings")
	FVector LeftFootIKJointTargetLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "IK Settings")
	FVector RightFootIKJointTargetLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "IK Settings")
	FVector LeftFootEffectorLocation = FVector::ZeroVector;
	
	UPROPERTY(BlueprintReadOnly, Category = "IK Settings")
	float LeftFootIKAlpha = 0.0f;
	
	UPROPERTY(BlueprintReadOnly, Category = "IK Settings")
	float RightFootIKAlpha = 0.0f;

	// New stable Foot IK outputs (use these in ABP instead of absolute effector world positions)
	UPROPERTY(BlueprintReadOnly, Category = "IK Settings")
	float PelvisOffsetZ = 0.0f;

	// -------------------------------------------------------------------------
	// Epic UE5 Mannequin — те же имена, что в ABP_Manny + CR_Mannequin_BasicFootIK
	// (см. Content/Characters/Mannequins/...). В шаблоне в AnimGraph на CR заведён
	// в основном ShouldDoIKTrace; ZOffset_* / IKBlend_* — как в графе CR (для кастомного
	// рига с внешними входами или отладки).
	// Важно: ShouldDoIKTrace=true включает СФЕРИЧЕСКИЕ ТРЕЙСЫ внутри CR. Если одновременно
	// используете наши TwoBone IK с трейсами в C++, отключите один из слоёв (например
	// подайте в ноду CR ShouldDoIKTrace=false и используйте только наши эффекторы).
	// -------------------------------------------------------------------------
	UPROPERTY(BlueprintReadOnly, Category = "IK Settings|Epic Mannequin")
	bool ShouldDoIKTrace = false;

	/** Вертикальная поправка левой стопы (см), аналог внутреннего ZOffset_L в CR. */
	UPROPERTY(BlueprintReadOnly, Category = "IK Settings|Epic Mannequin")
	float ZOffset_L = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "IK Settings|Epic Mannequin")
	float ZOffset_R = 0.0f;

	/** Смещение таза по Z (см), как ZOffset_Pelvis в CR (наш знак: обычно ≤ 0 вниз). */
	UPROPERTY(BlueprintReadOnly, Category = "IK Settings|Epic Mannequin")
	float ZOffset_Pelvis = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "IK Settings|Epic Mannequin")
	float IKBlend_l = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "IK Settings|Epic Mannequin")
	float IKBlend_r = 0.0f;

	// Smoothing (tune in BP if needed)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings")
	float FootIKEffectorInterpSpeed = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings")
	float PelvisIKInterpSpeed = 12.0f;

	// Small constant Z-lift (cm) so feet don't visually sink into the ground
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings", meta=(ClampMin="0.0", ClampMax="12.0", UIMin="0.0", UIMax="8.0"))
	float FootIKHeelLift = 5.5f;

	// Extra Z (cm) for collision thickness / trace vs visible sole — добавляется к HeelLift
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings", meta=(ClampMin="0.0", ClampMax="8.0", UIMin="0.0", UIMax="4.0"))
	float FootIKSoleClearanceCm = 2.0f;

	// Доп. подъём на неровной опоре (ступеньки)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings", meta=(ClampMin="0.0", ClampMax="6.0", UIMin="0.0", UIMax="3.0"))
	float FootIKStairExtraZLiftCm = 1.25f;

	// Тазобедро — для плоскости ноги (hip → IK foot), иначе pole уезжает при стопе на ступеньке.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings|Knee Pole")
	FName LeftThighSocketForIK = FName(TEXT("thigh_l"));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings|Knee Pole")
	FName RightThighSocketForIK = FName(TEXT("thigh_r"));

	// Колено (Manny: calf_l / calf_r).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings|Knee Pole")
	FName LeftKneeRefSocketForIK = FName(TEXT("calf_l"));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings|Knee Pole")
	FName RightKneeRefSocketForIK = FName(TEXT("calf_r"));

	// Вынос pole вдоль «вперёд персонажа», спроецированного в плоскость сгиба ноги (см).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings|Knee Pole", meta=(ClampMin="-20.0", ClampMax="35.0", UIMin="0.0", UIMax="22.0"))
	float FootIKKneePoleForwardCm = 14.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings|Knee Pole", meta=(ClampMin="-15.0", ClampMax="15.0", UIMin="-8.0", UIMax="8.0"))
	float FootIKKneePoleRightCm = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings|Knee Pole", meta=(ClampMin="0.0", ClampMax="20.0", UIMin="0.0", UIMax="12.0"))
	float FootIKKneePoleUpCm = 5.0f;

	/** Доп. вперёд на ступеньках (к плоскости ноги уже применяется проекция). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings|Knee Pole", meta=(ClampMin="0.0", ClampMax="15.0", UIMin="0.0", UIMax="10.0"))
	float FootIKKneePoleStairForwardAddCm = 4.0f;

	/** Доп. вынести pole в сторону ActorRight для ноги, которая стоит выше (ступенька) — против flip колена. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings|Knee Pole", meta=(ClampMin="0.0", ClampMax="25.0", UIMin="0.0", UIMax="16.0"))
	float FootIKKneePoleOutwardWhenHigherCm = 9.0f;

	/** Доля «вперёд стопы» (ось X сокета foot_*) в направлении pole; остальное — проекция ActorForward в плоскости ноги. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings|Knee Pole", meta=(ClampMin="0.0", ClampMax="1.0", UIMin="0.0", UIMax="1.0"))
	float FootIKKneePoleFootForwardBlend = 0.72f;

	/** Сдвинуть pole вдоль направления стопы (в плоскости ┴ бедро–стопа) — колено «едет» за носком/курсом стопы. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings|Knee Pole", meta=(ClampMin="0.0", ClampMax="25.0", UIMin="0.0", UIMax="16.0"))
	float FootIKKneePoleAlongFootCm = 6.0f;

	// --- FABRIK (колено как суб-база цепи бедро–колено–стопа) ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings|FABRIK", meta=(ClampMin="1", ClampMax="32", UIMin="2", UIMax="16"))
	int32 FootIKFabrikMaxIterations = 8;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings|FABRIK", meta=(ClampMin="0.01", ClampMax="2.0", UIMin="0.05", UIMax="0.5"))
	float FootIKFabrikToleranceCm = 0.15f;

	// Сместить цель IK к пятке; по умолчанию 0 — при проблемах с коленом сначала настрой pole, не пятку.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings", meta=(ClampMin="0.0", ClampMax="20.0", UIMin="0.0", UIMax="12.0"))
	float FootIKEffectorRetreatToHeelCm = 0.0f;

	// Ignore tiny offsets for pelvis to avoid visible bouncing
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings", meta=(ClampMin="0.0", ClampMax="20.0", UIMin="0.0", UIMax="10.0"))
	float PelvisOffsetDeadZone = 3.0f;

	// Ignore tiny foot offsets to reduce jitter on flat ground
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings", meta=(ClampMin="0.0", ClampMax="10.0", UIMin="0.0", UIMax="5.0"))
	float FootOffsetDeadZone = 1.5f;

	// Clamp pelvis movement range in cm (downwards only, negative values)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings", meta=(ClampMin="-30.0", ClampMax="0.0", UIMin="-15.0", UIMax="0.0"))
	float PelvisOffsetMinZ = -12.0f;

	// Allow a small upward pelvis correction for stair steps (cm).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings", meta=(ClampMin="-10.0", ClampMax="30.0", UIMin="0.0", UIMax="20.0"))
	float PelvisOffsetMaxZ = 8.0f;

	// Foot plant / lock curves. If these curves exist in locomotion animations, they prevent foot jitter and pulling while moving.
	// Convention: 1 = planted, 0 = swing.
	// If disabled, IK falls back to always-on while grounded (no curve dependency).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings|FootLock")
	bool bUseFootPlantCurves = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings|FootLock")
	TArray<FName> LeftFootPlantCurveNames { FName(TEXT("FootPlant_L")), FName(TEXT("FootLock_L")), FName(TEXT("Enable_FootIK_L")) };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings|FootLock")
	TArray<FName> RightFootPlantCurveNames { FName(TEXT("FootPlant_R")), FName(TEXT("FootLock_R")), FName(TEXT("Enable_FootIK_R")) };

	// Hysteresis: lock when >= LockThreshold, unlock when <= UnlockThreshold
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings|FootLock", meta=(ClampMin="0.0", ClampMax="1.0", UIMin="0.0", UIMax="1.0"))
	float FootLockThreshold = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings|FootLock", meta=(ClampMin="0.0", ClampMax="1.0", UIMin="0.0", UIMax="1.0"))
	float FootUnlockThreshold = 0.3f;

	// When a foot is not planted, fade its IK alpha by the plant curve
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings|FootLock", meta=(ClampMin="0.0", ClampMax="1.0", UIMin="0.0", UIMax="1.0"))
	float SwingIKAlphaScale = 0.0f;

	// Optional run-time simplification:
	// reduce IK influence at higher speed to avoid per-step jitter/twist artifacts.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings|Runtime", meta=(AllowPrivateAccess="true"))
	bool bSimplifyIKAtRun = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings|Runtime", meta=(ClampMin="0.0", UIMin="0.0", AllowPrivateAccess="true"))
	float IKRunSimplifyStartSpeed = 160.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings|Runtime", meta=(ClampMin="1.0", UIMin="1.0", AllowPrivateAccess="true"))
	float IKRunSimplifyFullSpeed = 320.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings|Runtime", meta=(ClampMin="0.0", ClampMax="1.0", UIMin="0.0", UIMax="1.0", AllowPrivateAccess="true"))
	float IKMinAlphaAtFullRun = 0.08f;

	// Slower effector follow at high speed = less foot twitch during sprint.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings|Runtime", meta=(ClampMin="0.05", ClampMax="1.0", UIMin="0.1", UIMax="1.0", AllowPrivateAccess="true"))
	float RunEffectorInterpMinScale = 0.22f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings|Runtime", meta=(ClampMin="0.0", UIMin="0.0", AllowPrivateAccess="true"))
	float RunEffectorBlendStartSpeed = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings|Runtime", meta=(ClampMin="1.0", UIMin="1.0", AllowPrivateAccess="true"))
	float RunEffectorBlendFullSpeed = 480.0f;

	// Extra smoothing on right foot effector at run (often the twitchy side). Too low vs left reads as twitch.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings|Runtime", meta=(ClampMin="0.2", ClampMax="1.0", UIMin="0.55", UIMax="1.0", AllowPrivateAccess="true"))
	float RightFootEffectorSmoothMulAtRun = 0.78f;

	// Short-move anti-jitter gate:
	// IK enables with a small delay and uses hysteresis on speed start/stop.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings|Runtime", meta=(ClampMin="0.0", UIMin="0.0", AllowPrivateAccess="true"))
	float IKStartSpeedThreshold = 95.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings|Runtime", meta=(ClampMin="0.0", UIMin="0.0", AllowPrivateAccess="true"))
	float IKStopSpeedThreshold = 55.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings|Runtime", meta=(ClampMin="0.0", UIMin="0.0", AllowPrivateAccess="true"))
	float IKStartDelaySeconds = 0.10f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings|Runtime", meta=(ClampMin="0.1", UIMin="0.1", AllowPrivateAccess="true"))
	float IKGateBlendSpeed = 12.0f;

	// Stair / uneven feet: detection + hysteresis live on ABTZBaseCharacter (IsFootIKStairLike).

	// Additional damping of foot effectors while stair-like (trace noise while standing/moving on steps).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings|Runtime", meta=(ClampMin="0.2", ClampMax="1.0", UIMin="0.35", UIMax="1.0", AllowPrivateAccess="true"))
	float StairFootEffectorInterpMul = 0.52f;

	/** Ниже этой 2D-скорости (см/с) на ступеньках не замедлять эффекторы — иначе в покое стопа «плывёт» над ступенькой. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings|Runtime", meta=(ClampMin="0.0", UIMin="0.0", AllowPrivateAccess="true"))
	float StairEffectorSlowMinSpeed2D = 35.0f;

	// Smaller pelvis dead zone on stairs so min(L,R) doesn't flicker around the threshold.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings|Runtime", meta=(ClampMin="0.15", ClampMax="1.0", UIMin="0.25", UIMax="1.0", AllowPrivateAccess="true"))
	float PelvisOffsetDeadZoneStairScale = 0.42f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings|Runtime", meta=(ClampMin="0.1", ClampMax="1.0", UIMin="0.2", UIMax="1.0", AllowPrivateAccess="true"))
	float StairFootOffsetDeadZoneScale = 0.35f;

	// Strafe animation variables
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character animation")
	bool bIsStrafing = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character animation")
	float StrafeDirection = 0.0f;

	// === CLIMBING ANIMATION VARIABLES (Новая система) ===
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Climbing Animation")
	bool bIsClimbing = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Climbing Animation")
	bool bIsVaulting = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Climbing Animation")
	bool bIsMantling = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Climbing Animation")
	bool bIsClimbing_High = false;  // Высокое лазание

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Climbing Animation")
	int32 ClimbingType = 0;  // 0=None, 1=Vault, 2=Mantle, 3=Climb

	// Placeholder: treat swimming as prone to reuse crawling animations
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Swimming")
	bool bIsSwimming = false;

private:
	TWeakObjectPtr<class ABTZBaseCharacter> CachedBaseCharacter;

	// Runtime foot lock state
	bool bLeftFootLocked = false;
	bool bRightFootLocked = false;
	FVector LockedLeftEffector = FVector::ZeroVector;
	FVector LockedRightEffector = FVector::ZeroVector;

	// Runtime locomotion IK gate state
	bool bIKLocomotionActive = false;
	float IKStartDelayRemaining = 0.0f;
	float IKGateAlpha = 0.0f;
};
