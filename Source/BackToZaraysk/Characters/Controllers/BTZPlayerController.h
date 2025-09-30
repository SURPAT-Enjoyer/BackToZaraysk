// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BTZPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class BACKTOZARAYSK_API ABTZPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
    ABTZPlayerController();
	virtual void SetPawn(APawn* InPawn) override;
    UFUNCTION(BlueprintCallable)
    void RequestToggleInventory();

protected:
	virtual void SetupInputComponent() override;

private:
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void Jump();
	void ChangeCrouchState();
	void StartSprint();
	void StopSprint();
    void ChangeProneState();
    void ToggleInventory();
    void Interact();
    void DropItem();
    void TryClimbObstacle();

    // Lean control relays
    void StartLeanLeft();
    void StopLeanLeft();
    void StartLeanRight();
    void StopLeanRight();

	TSoftObjectPtr<class ABTZBaseCharacter> CachedBaseCharacter;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UInventoryWidget> InventoryWidgetClass;

private:
	UPROPERTY()
	class UInventoryWidget* InventoryWidgetInstance = nullptr;
};
