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

	float GetIKOffsetForASocket(const FName& SocketName);
	void UpdateAnimationBlueprintIK();

private:
	bool bIsSprintRequested = false;
	
	float bCachedCrouchState = 0;
	float bCachedProneState = 0;
};
