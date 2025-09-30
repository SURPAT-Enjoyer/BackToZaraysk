// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "BackToZarayskBasePawn.generated.h"

UCLASS()
class BACKTOZARAYSK_API ABackToZarayskBasePawn : public APawn
{
	GENERATED_BODY()




public:
	// Sets default values for this pawn's properties
	ABackToZarayskBasePawn();

	UPROPERTY(VisibleAnywhere)
	class UPawnMovementComponent* MovementComponent;

	UPROPERTY(VisibleAnywhere)
	class USphereComponent* CollisionComponent;



	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void Jump();
	void OpenInventoryAction();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetInputForward() { return InputForward; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetInputRight() { return InputRight; }

private:
	float InputForward = 0.0f;
	float InputRight = 0.0f;

protected:
	UPROPERTY(EditDefaultsonly, BlueprintReadOnly, Category = "Base Pawn")
	float CollisionSphereRadius = 50.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Base pawn")
	class USpringArmComponent* SpringArmComponent;

#if	WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Base pawn")
	class UArrowComponent* ArrowComponent;
#endif
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Base pawn")
	class UCameraComponent* CameraComponent;
};
