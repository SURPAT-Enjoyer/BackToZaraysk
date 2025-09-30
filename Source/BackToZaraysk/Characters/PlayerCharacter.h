// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BTZBaseCharacter.h"
#include <Components/TimelineComponent.h>
#include "../Components/ObstacleClimbingComponent.h"
#include "PlayerCharacter.generated.h"


/**
 * 
 */
UCLASS(Blueprintable)
class BACKTOZARAYSK_API APlayerCharacter : public ABTZBaseCharacter
{
	GENERATED_BODY()
	
public:
	APlayerCharacter(const FObjectInitializer& ObjectInitializer);
	virtual void MoveForward(float Value) override;
	virtual void MoveRight(float Value) override;
	virtual void Turn(float Value) override;
	virtual void LookUp(float Value) override;

	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	virtual bool CanJumpInternal_Implementation() const override;
	virtual void OnJumped_Implementation() override;

	virtual void StartSprint();
	virtual void StopSprint();

	// Преодоление препятствий
	void TryClimbObstacle();

	virtual void BeginPlay();
	virtual void Tick(float DeltaTime);

	// Lean control API
	UFUNCTION(BlueprintCallable, Category="Lean")
	void SetLeanTarget(float AngleDeg);

	UFUNCTION(BlueprintCallable, Category="Lean")
	void ResetLean();


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Sprint|camera")
	UCurveFloat* CameraCurve;

	// Инвентарь
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	class UInventoryComponent* InventoryComponent;

	// Компонент преодоления препятствий
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Obstacle Climbing")
	class UObstacleClimbingComponent* ObstacleClimbingComponent;

	// Настройки IK для ног
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="IK Settings")
	bool bEnableFootIK = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="IK Settings")
	float FootIKStrength = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="IK Settings")
	float FootIKBlendSpeed = 8.0f;

	// IK переменные для Blueprint доступа (наследуются из базового класса)
	// Удалено дублирование - используем GetIKLeftFootOffset() и GetIKRightFootOffset()

	// Lean settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lean")
	float MaxLeanAngleDeg = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lean")
	float LeanInterpSpeed = 8.0f;

	// Максимальное боковое смещение (см) для выглядывания
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lean")
	float MaxLeanSideOffsetCm = 25.0f;


protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | Camera")
	class UCameraComponent* CameraComponent;
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | SpringArm")
	//class USpringArmComponent* SpringArmComponent;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | SpringArm|length")
	float TargetLength = 400;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | SpringArm|length")
	float DefaultLength = 300;
	float CurrentLength;

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Character IK")
	//class USkeletalMeshComponent* SkeletalMeshComponent;

	FTimeline CameraMoveTimeline;
	FORCEINLINE void BeginCameraMove() { CameraMoveTimeline.Play(); }
	UFUNCTION()
	void UpdateCameraMove(float Value);
	FORCEINLINE void EndCameraMove() { CameraMoveTimeline.Reverse(); }

private:
    float SpringArmEyeZStanding = 0.0f;
    FName HeadSocketName = TEXT("head");
    FVector HeadOffset = FVector::ZeroVector;
    void UpdateCameraPivotToHead();

    // Lean state
    float TargetLeanAngleDeg = 0.0f;
    UPROPERTY(BlueprintReadOnly, Category="Lean", meta=(AllowPrivateAccess="true"))
    float CurrentLeanAngleDeg = 0.0f;
    UPROPERTY(BlueprintReadOnly, Category="Lean", meta=(AllowPrivateAccess="true"))
    float CurrentLeanSideOffset = 0.0f;
	
};
