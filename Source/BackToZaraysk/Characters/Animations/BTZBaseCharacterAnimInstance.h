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
	float LeftFootIKAlpha = 1.0f;
	
	UPROPERTY(BlueprintReadOnly, Category = "IK Settings")
	float RightFootIKAlpha = 1.0f;

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

private:
	TWeakObjectPtr<class ABTZBaseCharacter> CachedBaseCharacter;
};
