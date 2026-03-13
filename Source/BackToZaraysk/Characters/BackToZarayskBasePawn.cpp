// Fill out your copyright notice in the Description page of Project Settings.


#include "BackToZarayskBasePawn.h"
#include "Components/SphereComponent.h"
#include "Engine/CollisionProfile.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "../Components/MovementComponents/BTZPawnMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/ArrowComponent.h"
#include "BackToZaraysk/Characters/Controllers/BTZPlayerController.h"


// Sets default values
ABackToZarayskBasePawn::ABackToZarayskBasePawn()
{
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	CollisionComponent->SetSphereRadius(CollisionSphereRadius);
	CollisionComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
	RootComponent = CollisionComponent;

	MovementComponent = CreateDefaultSubobject<UPawnMovementComponent, UBTZPawnMovementComponent>(TEXT("Movement component"));
	MovementComponent->SetUpdatedComponent(CollisionComponent);

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring arm"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->bUsePawnControlRotation = 1;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(SpringArmComponent);

#if	WITH_EDITORONLY_DATA
	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	ArrowComponent->SetupAttachment(RootComponent);
#endif
}

// Called to bind functionality to input
void ABackToZarayskBasePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("MoveForward", this, &ABackToZarayskBasePawn::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABackToZarayskBasePawn::MoveRight);
    PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &ABackToZarayskBasePawn::Jump);
    PlayerInputComponent->BindAction("OpenInventory", EInputEvent::IE_Pressed, this, &ABackToZarayskBasePawn::OpenInventoryAction);
}

void ABackToZarayskBasePawn::MoveForward(float Value)
{
	InputForward = Value;
	if (Value != 0.f)
	{
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void ABackToZarayskBasePawn::MoveRight(float Value)
{
	InputRight = Value;
	if (Value != 0.f)
	{
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void ABackToZarayskBasePawn::Jump()
{
	//checkf(MovementComponent->IsA<UBTZPawnMovementComponent>(), TEXT("ABackToZarayskBasePawn::Jump() Jump can work only with UBTZPawnMovementComponent"));
	UBTZPawnMovementComponent* BaseMovement = StaticCast<UBTZPawnMovementComponent*>(MovementComponent);
	BaseMovement->JumpStart();
}

void ABackToZarayskBasePawn::OpenInventoryAction()
{
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (ABTZPlayerController* BTZPC = Cast<ABTZPlayerController>(PC))
        {
            BTZPC->RequestToggleInventory();
        }
    }
}

