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
	virtual void Turn(float Value) {};
	virtual void LookUp(float Value) {};
	virtual void ChangeCrouchState();
	virtual void ChangeProneState();
	virtual void StartSprint();
	virtual void StopSprint();


	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	FORCEINLINE UBTZBaseCharMovementComponent* GetBaseCharacterMovementComponent() { return BTZBaseCharMovementComponent; }

	UPROPERTY(VisibleAnywhere)
	class UCapsuleComponent* Capsule;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | SpringArm")
	class USpringArmComponent* SpringArmComponent;

	UFUNCTION(Blueprintcallable, BlueprintPure)
	FORCEINLINE float GetIKLeftFootOffset() const { return IKLeftFootOffset; }
	UFUNCTION(Blueprintcallable, BlueprintPure)
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

	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Stamina")
	float MaxStamina = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Stamina")
	float StaminaRestoreVelocity = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Stamina")
	float SprintStaminaConsumptionVelocity = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Stamina")
	float StaminaTiredThreshold = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Stamina")
	float StaminaCanSprintAndJumpThreshold = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Movement|Prone")
	bool bIsProned;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Character IK")
	FName LeftFootSocketName;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Character IK")
	FName RightFootSocketName;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player Character IK", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float IKTraceExtandDistance = 30.0f;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Player Character IK", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float IKInterpSpeed = 20.0f;

	//UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Player Character IK", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	//float IKTraceDistance = 90.0f;

	//ABTZBaseCharacter* CharacterOwner;

	virtual bool CanSprint();

	void OnProneStart();
	void OnProneEnd(float HeightAdjust, float ScaledHeightAdjust);
	
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
	UBTZBaseCharMovementComponent* BTZBaseCharMovementComponent;

	void TryChangeSprintState(float DeltaTime);
	float CurrentStamina;
private:
	bool bIsSprintRequested = false;
	
	float IKLeftFootOffset = 0;
	float IKRightFootOffset = 0;

	float GetIKOffsetForASocket(const FName& SocketName);

	float IKTraceDistance = 0.0f;
	float IKScale = 0.0f;

	float bCachedCrouchState = 0;
	float bCachedProneState = 0;
};
