// Fill out your copyright notice in the Description page of Project Settings.


//#include "Characters/Animations/BTZBasePawnAnimInstance.h"
#include "BTZBasePawnAnimInstance.h"
#include "../BackToZarayskBasePawn.h"
#include "../../Components/MovementComponents/BTZPawnMovementComponent.h"

void UBTZBasePawnAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
	checkf(TryGetPawnOwner()->IsA<ABackToZarayskBasePawn>(), TEXT("UBTZBasePawnAnimInstance::NativeBeginPlay() only works with ABackToZarayskBasePawn"));
	CachedBasePawn = StaticCast<ABackToZarayskBasePawn*>(TryGetPawnOwner());
}

void UBTZBasePawnAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (!CachedBasePawn.IsValid())
	{
		return;
	}
	InputForward = CachedBasePawn->GetInputForward();
	InputRight = CachedBasePawn->GetInputRight();
	bIsInAir = CachedBasePawn->GetMovementComponent()->IsFalling();
}
