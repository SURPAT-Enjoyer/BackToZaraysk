// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BTZBaseCharacter.h"
#include <Components/TimelineComponent.h>
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

	virtual void BeginPlay();
	virtual void Tick(float DeltaTime);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Sprint|camera")
	UCurveFloat* CameraCurve;


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
	
};
