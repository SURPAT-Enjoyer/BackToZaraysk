// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlatformInvokator.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(OnInvokedSignature, bool);

UCLASS( ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BACKTOZARAYSK_API APlatformInvokator : public AActor
{
	GENERATED_BODY()
	
public:	
	OnInvokedSignature OnInvoked;

protected:
	UFUNCTION(BlueprintCallable)
	void Invoke();
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform state")
	bool bIsActorInside = true;
};
