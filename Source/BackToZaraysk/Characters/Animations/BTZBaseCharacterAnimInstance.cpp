// Fill out your copyright notice in the Description page of Project Settings.


#include "BTZBaseCharacterAnimInstance.h"
#include "../BTZBaseCharacter.h"
#include "../../Components/MovementComponents/BTZBaseCharMovementComponent.h"

void UBTZBaseCharacterAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
	checkf(TryGetPawnOwner()->IsA<ABTZBaseCharacter>(), TEXT("UBTZBaseCharacterAnimInstance::NativeBeginPlay() can be used only with ABTZBaseCharacter"));
	CachedBaseCharacter = StaticCast<ABTZBaseCharacter*>(TryGetPawnOwner());
}

void UBTZBaseCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (!CachedBaseCharacter.IsValid())
	{
		return;
	}

	UBTZBaseCharMovementComponent* CharacterMovement = CachedBaseCharacter->GetBaseCharacterMovementComponent();
	Speed = CharacterMovement->Velocity.Size();
	bIsFalling = CharacterMovement->IsFalling();
	bIsCrouching = CharacterMovement->IsCrouching();

	bIsSprinting = CharacterMovement->IsSprinting();
	bIsOutOfStamina = CharacterMovement->IsOutOfStamina();
	bIsProning = CharacterMovement->IsProning();

	RightFootEffectorLocation = FVector(CachedBaseCharacter->GetIKRightFootOffset(), 0.0f, 0.0f);
	LeftFootEffectorLocation = FVector(CachedBaseCharacter->GetIKLeftFootOffset(), 0.0f, 0.0f);
}
