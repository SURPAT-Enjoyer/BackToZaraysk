// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BackToZarayskBasePawn.h"
#include "SpiderPawn.generated.h"

/**
 * 
 */
UCLASS()
class BACKTOZARAYSK_API ASpiderPawn : public ABackToZarayskBasePawn
{
	GENERATED_BODY()

public:
	ASpiderPawn();

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE float GetIKRightFrontFootOffset() const { return IKRightFrontFootOffset; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE float GetIKLeftFrontFootOffset() const { return IKLeftFrontFootOffset; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE float GetIKRightRearFootOffset() const { return IKRightRearFootOffset; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE float GetIKLeftRearFootOffset() const { return IKLeftRearFootOffset; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider bot")
	class USkeletalMeshComponent* SkeletalMeshComponent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider bot|IK settings")
	FName RightFrontFootSocketName;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider bot|IK settings")
	FName RightRearFootSocketName;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider bot|IK settings")
	FName LeftRearFootSocketName;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider bot|IK settings")
	FName LeftFrontFootSocketName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spider bot|IK settings", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float IKTraceExtandDistance = 30.0f;
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Spider bot|IK settings", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float IKInterpSpeed = 20.0f;

private:
	float GetIKOffsetForASocket(const FName& SocketName);

	float IKTraceDistance = 0.0f;
	float IKRightFrontFootOffset = 0.0f;
	float IKRightRearFootOffset = 0.0f;
	float IKLeftFrontFootOffset = 0.0f;
	float IKLeftRearFootOffset = 0.0f;
	float IKScale = 0.f;
};
