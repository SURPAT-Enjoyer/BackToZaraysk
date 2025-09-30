// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BTZBaseCharMovementComponent.generated.h"


/**
 * 
 */
UCLASS()
class BACKTOZARAYSK_API UBTZBaseCharMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	FORCEINLINE bool IsSprinting() { return bIsSprinting; }
	FORCEINLINE bool IsOutOfStamina() const { return bIsOutOfStamina; }
	bool IsProning();
	bool bIsProning = false;

	virtual float GetMaxSpeed() const override;
	virtual bool CanAttemptJump() const override;
	void StartSprint();
	void StopSprint();
	void SetIsOutOfStamina(bool bIsOutOfStamina_In);
	virtual void Prone();
	virtual void UnProne();
	virtual void BeginPlay() override;

	virtual void Crouch(bool bClientSimulation) override;
	virtual void UnCrouch(bool bClientSimulation) override;
	FCollisionShape GetCollisionShape();
	FCollisionShape StandingCapsuleShape;


	UPROPERTY(Transient, DuplicateTransient)
	TObjectPtr<ACharacter> BTZCharacterOwner;

	//void OnProne();
	//void OnUnProne();
	UPROPERTY(EditAnywhere, BlueprintreadWrite, Category = "Character|Movement|Prone")
	float ProneCapsuleRadius = 30.0f;
	UPROPERTY(EditAnywhere, BlueprintreadWrite, Category = "Character|Movement|Prone")
	float ProneCapsuleHalfHeight = 30.0f;
	UPROPERTY(EditAnywhere, BlueprintreadWrite, Category = "Character|Capsule cache")
	float DefaultCapsuleRadius;
	UPROPERTY(EditAnywhere, BlueprintreadWrite, Category = "Character|Capsule cahce")
	float DefaultCapsuleHalfHeight;


protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character movement: sprint", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float SprintSpeed = 1200.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character movement: out of stamina", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float TiredSpeed = 100.0f;
	UPROPERTY(EditAnywhere, BlueprintreadWrite, Category = "Character|Movement")
	float RunSpeed = 800.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character movement|Prone", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float ProneSpeed = 100.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character movement|Crouch", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float CrouchSpeed = 125.0f;


	// Removed camera spring arm reference from movement component to avoid null access and coupling

	//UPROPERTY(Transient, DuplicateTransient)
	//TObjectPtr<ABTZBaseCharacter> CharacterOwner;
	
private:
	bool bIsOutOfStamina;
	bool bIsSprinting;

    // Extra mesh vertical offset (added on top of capsule half-height delta) for prone
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character movement|Prone", meta = (AllowPrivateAccess = "true"))
    float ProneMeshExtraZOffset = 2.0f;

    // Amount of Z lift applied to mesh when entering prone (to undo on exit)
    float ProneMeshAppliedLift = 0.0f;

    // Extra mesh vertical offset for crouch
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character movement|Crouch", meta = (AllowPrivateAccess = "true"))
    float CrouchMeshExtraZOffset = 0.0f;

    // Amount of Z lift applied to mesh when entering crouch (to undo on exit)
    float CrouchMeshAppliedLift = 0.0f;
	
};
