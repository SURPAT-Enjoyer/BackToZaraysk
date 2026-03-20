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
	
	UPROPERTY(BlueprintReadOnly, Category = "IK Settings")
	FVector LeftFootEffectorLocation = FVector::ZeroVector;
	
	UPROPERTY(BlueprintReadOnly, Category = "IK Settings")
	float LeftFootIKAlpha = 0.0f;
	
	UPROPERTY(BlueprintReadOnly, Category = "IK Settings")
	float RightFootIKAlpha = 0.0f;

	// New stable Foot IK outputs (use these in ABP instead of absolute effector world positions)
	UPROPERTY(BlueprintReadOnly, Category = "IK Settings")
	float PelvisOffsetZ = 0.0f;

	// Smoothing (tune in BP if needed)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings")
	float FootIKEffectorInterpSpeed = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings")
	float PelvisIKInterpSpeed = 12.0f;

	// Small constant Z-lift (cm) so feet don't visually sink into the ground
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings", meta=(ClampMin="0.0", ClampMax="10.0", UIMin="0.0", UIMax="5.0"))
	float FootIKHeelLift = 4.0f;

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
};
