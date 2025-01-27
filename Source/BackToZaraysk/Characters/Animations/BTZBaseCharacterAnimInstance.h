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
	bool bIsSprinting = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character animation||Stamina")
	bool bIsOutOfStamina = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character animation")
	bool bIsProning = false;

	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadonly, Category = "IK Settings")
	FVector RightFootEffectorLocation = FVector::ZeroVector;
	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadonly, Category = "IK Settings")
	FVector LeftFootEffectorLocation = FVector::ZeroVector;

private:
	TWeakObjectPtr<class ABTZBaseCharacter> CachedBaseCharacter;
};
