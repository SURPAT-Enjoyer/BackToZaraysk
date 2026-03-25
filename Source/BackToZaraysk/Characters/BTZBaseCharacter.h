// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../Components/MovementComponents/BTZBaseCharMovementComponent.h"
#include "BTZBaseCharacter.generated.h"


//class UBTZBaseCharMovementComponent;
UCLASS(Abstract, NotBlueprintable)
class BACKTOZARAYSK_API ABTZBaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABTZBaseCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void MoveForward(float Value) {};
	virtual void MoveRight(float Value) {};
	virtual void Turn(float Value);
	virtual void LookUp(float Value);
	
	// Функции для блокировки поворота при FreeLook
	void SetRotationBlocked(bool bBlocked);
	bool IsRotationBlocked() const { return bRotationBlocked; }
	
	// Функции для управления поворотом головы
	void SetHeadRotation(float Yaw, float Pitch);
	void ResetHeadRotation();
	virtual void ChangeCrouchState();
	virtual void ChangeProneState();
	virtual void StartSprint();
	virtual void StopSprint();

	// Преодоление препятствий
	virtual void TryClimbObstacle();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	FORCEINLINE UBTZBaseCharMovementComponent* GetBaseCharacterMovementComponent() { return BTZBaseCharMovementComponent; }

	// IK getters
	FORCEINLINE FName GetLeftFootSocketName() const { return LeftFootSocketName; }
	FORCEINLINE FName GetRightFootSocketName() const { return RightFootSocketName; }

	UPROPERTY(VisibleAnywhere)
	class UCapsuleComponent* Capsule;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | SpringArm")
	class USpringArmComponent* SpringArmComponent;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE float GetIKLeftFootOffset() const { return IKLeftFootOffset; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE float GetIKRightFootOffset() const { return IKRightFootOffset; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="IK Settings")
	FORCEINLINE bool HadLastFootHit(bool bLeft) const { return bLeft ? bLastLeftFootHadHit : bLastRightFootHadHit; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="IK Settings")
	FORCEINLINE float GetLastFootHitZ(bool bLeft) const { return bLeft ? LastLeftFootHitZ : LastRightFootHitZ; }

	// Stable foot trace origin used for Foot IK (world space).
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="IK Settings")
	FVector GetFootTraceOriginWorld(bool bLeft) const;

	/** True while feet are treated as stair/uneven (sticky hysteresis — avoids gate/smoothing flicker). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="IK Settings")
	FORCEINLINE bool IsFootIKStairLike() const { return bFootIKStairSticky; }

	// Включает отладочный вывод: какой AnimInstance/ABP реально используется и какие IK оффсеты считаются.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="IK Settings|Debug")
	bool bDebugFootIK = false;


	//UPROPERTY(BlueprintReadOnly, Transient, DuplicateTransient, Category = MovementComponent)
	//TObjectPtr<USceneComponent> UpdatedComponent;

protected:

	bool FirstProning = true; //TODO find right solution for mesh clipping through floor

	UFUNCTION(BlueprintNativeEvent, Category = "Character | Movement")
	void OnSprintStart();
	virtual void OnSprintStart_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "Character | Movement")
	void OnSprintEnd();
	virtual void OnSprintEnd_Implementation();
protected:

	

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Stamina", meta = (AllowPrivateAccess = "true"))
    float MaxStamina = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Stamina", meta = (AllowPrivateAccess = "true"))
	float StaminaRestoreVelocity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Stamina", meta = (AllowPrivateAccess = "true"))
	float SprintStaminaConsumptionVelocity = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Stamina", meta = (AllowPrivateAccess = "true"))
	float StaminaTiredThreshold = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Stamina", meta = (AllowPrivateAccess = "true"))
	float StaminaCanSprintAndJumpThreshold = 50.0f;

    // Блокировка поворота для FreeLook
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|FreeLook", meta = (AllowPrivateAccess = "true"))
    bool bRotationBlocked = false;
    
    // Управление поворотом головы для FreeLook (Transform Modify Bone)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|FreeLook", meta = (AllowPrivateAccess = "true"))
    float HeadYawRotation = 0.0f;  // Поворот головы влево/вправо (в градусах)
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|FreeLook", meta = (AllowPrivateAccess = "true"))
    float HeadPitchRotation = 0.0f;  // Наклон головы вверх/вниз (в градусах)

	// Duplicate prone state removed; rely on movement component's IsProning()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Character IK", meta = (AllowPrivateAccess = "true"))
	FName LeftFootSocketName;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Character IK", meta = (AllowPrivateAccess = "true"))
	FName RightFootSocketName;
	
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player Character IK", meta = (ClampMin = 0.0f, UIMin = 0.0f, AllowPrivateAccess = "true"))
	float IKTraceExtendDistance = 30.0f;

    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Player Character IK", meta = (ClampMin = 0.0f, UIMin = 0.0f, AllowPrivateAccess = "true"))
	float IKInterpSpeed = 20.0f;

	// When horizontal speed exceeds this, IK offsets follow traces more slowly (reduces run jitter).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings", meta=(ClampMin="0.0", UIMin="0.0", AllowPrivateAccess="true"))
	float RunIKSmoothSpeedThreshold = 240.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings", meta=(ClampMin="0.05", ClampMax="1.0", UIMin="0.1", UIMax="1.0", AllowPrivateAccess="true"))
	float RunIKInterpSpeedScale = 0.32f;

	// Множитель к IKInterpSpeed в покое на ровной земле (меньше — меньше дёрганья, но медленнее к цели).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings", meta=(ClampMin="0.05", ClampMax="1.0", UIMin="0.1", UIMax="0.6", AllowPrivateAccess="true"))
	float FootIKIdleInterpMul = 0.3f;

	// В покое на ступеньках/разном уровне ног — почти полная скорость, иначе стопа «не догоняет» raw (висит над ступенькой).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings", meta=(ClampMin="0.2", ClampMax="1.0", UIMin="0.5", UIMax="1.0", AllowPrivateAccess="true"))
	float FootIKIdleInterpMulStairs = 0.92f;

	// If |raw left IK - raw right IK| exceeds this (cm), treat as stairs / uneven feet (full IK responsiveness).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings", meta=(ClampMin="4.0", UIMin="6.0", AllowPrivateAccess="true"))
	float StairFeetRawDeltaCm = 8.0f;

	// Also treat as stair-like when one foot has large |raw| and feet disagree (catches 13.8 vs 14 threshold gaps).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings", meta=(ClampMin="2.0", UIMin="4.0", AllowPrivateAccess="true"))
	float StairAsymmetricMinRawDeltaCm = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings", meta=(ClampMin="4.0", UIMin="6.0", AllowPrivateAccess="true"))
	float StairAsymmetricSingleFootAbsCm = 9.0f;

	// Hysteresis exit: both feet nearly neutral — then leave stair IK mode (prevents run twitch from borderline raw noise).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings", meta=(ClampMin="2.0", UIMin="3.0", AllowPrivateAccess="true"))
	float StairIKExitMaxFootAbsCm = 5.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings", meta=(ClampMin="1.0", UIMin="2.0", AllowPrivateAccess="true"))
	float StairIKExitMaxDeltaCm = 4.0f;

    //ABTZBaseCharacter* CharacterOwner;

    virtual bool CanSprint();

    void OnProneStart();
    void OnProneEnd(float HeightAdjust, float ScaledHeightAdjust);
    
    //UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
    UBTZBaseCharMovementComponent* BTZBaseCharMovementComponent;

    void TryChangeSprintState(float DeltaTime);
    float CurrentStamina;

	// IK переменные (перемещены в protected для доступа из наследников)
	float IKLeftFootOffset = 0;
	float IKRightFootOffset = 0;
	float IKTraceDistance = 0.0f;
	float IKScale = 0.0f;

	// Foot IK trace origin baseline (stable Z from capsule, XY from foot).
	// Lateral offsets help when foot bone is centered.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings", meta=(AllowPrivateAccess="true"))
	float FootTraceForwardOffsetCm = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings", meta=(AllowPrivateAccess="true"))
	float FootTraceLateralOffsetCm = 0.0f; // applied as -Y for left, +Y for right

	// Z baseline: distance above capsule bottom to place trace origin.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings", meta=(ClampMin="0.0", UIMin="0.0", AllowPrivateAccess="true"))
	float FootTraceBaseAboveCapsuleBottomCm = 0.0f;

	// Below this horizontal speed (cm/s) trace Z stays stabilized (capsule/foot max) to reduce idle bob.
	// Above FootTraceZPerFootBlendEndSpeed, trace Z follows each foot independently (stops sync leg bounce when IK is on).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings", meta=(ClampMin="0.0", UIMin="0.0", AllowPrivateAccess="true"))
	float FootTraceZPerFootBlendStartSpeed = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings", meta=(ClampMin="1.0", UIMin="5.0", AllowPrivateAccess="true"))
	float FootTraceZPerFootBlendEndSpeed = 48.0f;

	// Additional offset from the foot socket towards the "bottom of the foot" (cm).
	// Used when computing trace origin on the mesh surface.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings", meta=(ClampMin="0.0", UIMin="0.0", AllowPrivateAccess="true"))
	float FootTraceSocketToFootBottomOffsetCm = 0.0f;

	// When the trace origin is inside collision (foot clipping),
	// we bias the IK result upwards by this amount to resolve penetration.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings", meta=(ClampMin="0.0", UIMin="0.0", AllowPrivateAccess="true"))
	float FootPenetrationResolveLiftCm = 2.0f;

	// Radius for the overlap test that detects whether foot origin is inside collision.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings", meta=(ClampMin="0.1", UIMin="0.1", AllowPrivateAccess="true"))
	float FootPenetrationTestRadiusCm = 1.5f;

	// If the computed foot offset is within +/- this value (cm),
	// treat it as FLAT and suppress micro jitter.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings", meta=(ClampMin="0.0", UIMin="0.0", AllowPrivateAccess="true"))
	float FootFlatOffsetEpsilonCm = 6.0f;

	// FLAT suppression is only applied when the contact point (HitZ) is stable
	// frame-to-frame. This prevents killing legitimate small offsets on steps.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings", meta=(ClampMin="0.0", UIMin="0.0", AllowPrivateAccess="true"))
	float FootFlatHitZDeltaEpsilonCm = 2.0f;

	// While moving onto/over a step, we don't want FLAT to kill the step reaction.
	// So we only apply FLAT when the character is nearly stopped.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings", meta=(ClampMin="0.0", UIMin="0.0", AllowPrivateAccess="true"))
	float FootFlatMaxSpeedCmPerSec = 10.0f;

	// If foot bone is this much above capsule baseline (cm), do not FLAT-zero (one foot on step, other low).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings", meta=(ClampMin="0.0", UIMin="0.0", AllowPrivateAccess="true"))
	float FootFlatSkipFootElevAboveCapsuleCm = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="IK Settings", meta=(ClampMin="0.0", UIMin="0.0", AllowPrivateAccess="true"))
	float FootTraceStartAboveCm = 25.0f;

	// Last ground hit Z per foot (world space)
	UPROPERTY(BlueprintReadOnly, Category="IK Settings")
	float LastLeftFootHitZ = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category="IK Settings")
	float LastRightFootHitZ = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category="IK Settings")
	bool bLastLeftFootHadHit = false;
	UPROPERTY(BlueprintReadOnly, Category="IK Settings")
	bool bLastRightFootHadHit = false;

	float GetIKOffsetForFoot(bool bLeft);
	void UpdateAnimationBlueprintIK();

private:
	bool bIsSprintRequested = false;
	bool bFootIKStairSticky = false;
};
