// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "../Actors/PlatformInvokator.h"
#include "InvokatorClientComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInvocatorActivated, bool, bIsActorInside);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BACKTOZARAYSK_API UInvokatorClientComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditInstanceOnly)
	APlatformInvokator* Invokator;

	UPROPERTY(BlueprintAssignable)
	FOnInvocatorActivated OnInvoctorActivated;

	UFUNCTION()
	void OnInvoked(bool bIsActorInside);

	virtual void BeginPlay() override;	

};
