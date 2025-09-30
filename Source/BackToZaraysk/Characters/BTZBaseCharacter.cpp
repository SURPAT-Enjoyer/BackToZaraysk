// Fill out your copyright notice in the Description page of Project Settings.


#include "BTZBaseCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/SpringArmComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimBlueprintGeneratedClass.h"
#include "Animations/BTZBaseCharacterAnimInstance.h"

ABTZBaseCharacter::ABTZBaseCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UBTZBaseCharMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	BTZBaseCharMovementComponent = StaticCast<UBTZBaseCharMovementComponent*>(GetCharacterMovement());

	// Убираем масштабирование для более точных IK расчетов
	IKScale = 1.0f;

    // Default IK socket names for UE mannequins
    LeftFootSocketName = FName(TEXT("foot_l"));
    RightFootSocketName = FName(TEXT("foot_r"));

    // Set default IK parameters
    IKTraceExtendDistance = 30.0f;
    IKInterpSpeed = 12.0f; // Оптимальная скорость интерполяции для плавного IK
}

void ABTZBaseCharacter::ChangeCrouchState()
{
	if (BTZBaseCharMovementComponent->IsCrouching())
	{
		UnCrouch();
		//UnProne();
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, TEXT("Crouch OFF"));
	}
	else
	{
		Crouch();
		//UnProne();
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, TEXT("Crouch ON"));
	}
}

void ABTZBaseCharacter::ChangeProneState()
{
	if (BTZBaseCharMovementComponent->IsProning())
	{
		BTZBaseCharMovementComponent->UnProne();
		//UnCrouch();
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, TEXT("Prone OFF"));
	}
	else
	{
		BTZBaseCharMovementComponent->Prone();
		//UnCrouch();
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, TEXT("Prone ON"));
	}
}

void ABTZBaseCharacter::StartSprint()
{
	bIsSprintRequested = true;
	if (bIsCrouched)
	{
		UnCrouch();
	}
}

void ABTZBaseCharacter::StopSprint()
{
	bIsSprintRequested = false;
}


void ABTZBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	CurrentStamina = MaxStamina;
	//Capsule = GetCapsuleComponent();
	float test = GetMesh()->GetRelativeLocation().Z;
	GEngine->AddOnScreenDebugMessage(1, 1.0f, FColor::Red, FString::Printf(TEXT("%.2f"), test));
	// ИСПРАВЛЕНО: Оптимизированное расстояние трассировки для лучшего обнаружения
	// Используем большее расстояние для более надежного обнаружения неровностей
	IKTraceDistance = 100.0f; // Увеличено для лучшего обнаружения земли
}

void ABTZBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	BTZBaseCharMovementComponent->GetMaxSpeed();
	TryChangeSprintState(DeltaTime);
	GEngine->AddOnScreenDebugMessage(0, 1.0f, FColor::Yellow, FString::Printf(TEXT("Stamina: %.2f"), CurrentStamina));
	if (!BTZBaseCharMovementComponent->IsSprinting() || !BTZBaseCharMovementComponent->IsFalling())
	{
		CurrentStamina += StaminaRestoreVelocity * DeltaTime;
		CurrentStamina = FMath::Clamp(CurrentStamina, 0.0f, MaxStamina);
	}

	// ИСПРАВЛЕНО: Улучшенная логика ИК
	bool bIsOnGround = GetCharacterMovement()->IsMovingOnGround();
	bool bIsMoving = GetVelocity().Size() > 10.0f; // Движется ли персонаж
	
	// Получаем сырые смещения
	float RawLeftOffset = GetIKOffsetForASocket(LeftFootSocketName);
	float RawRightOffset = GetIKOffsetForASocket(RightFootSocketName);
	
	// Определяем скорость интерполяции в зависимости от состояния
	float CurrentIKSpeed;
	if (bIsOnGround)
	{
		CurrentIKSpeed = bIsMoving ? IKInterpSpeed : IKInterpSpeed * 0.3f; // Медленнее когда стоит
	}
	else
	{
		CurrentIKSpeed = IKInterpSpeed * 0.7f; // В воздухе тоже работает, но медленнее
	}

	// Применяем ИК с улучшенной логикой
	if (bIsOnGround)
	{
		// На земле - применяем рассчитанные смещения
		IKLeftFootOffset = FMath::FInterpTo(IKLeftFootOffset, RawLeftOffset, DeltaTime, CurrentIKSpeed);
		IKRightFootOffset = FMath::FInterpTo(IKRightFootOffset, RawRightOffset, DeltaTime, CurrentIKSpeed);
	}
	else
	{
		// В воздухе - не сбрасываем полностью, а интерполируем к нулю медленнее
		IKLeftFootOffset = FMath::FInterpTo(IKLeftFootOffset, 0.0f, DeltaTime, CurrentIKSpeed * 0.5f);
		IKRightFootOffset = FMath::FInterpTo(IKRightFootOffset, 0.0f, DeltaTime, CurrentIKSpeed * 0.5f);
	}

	// Передача IK данных в Animation Blueprint (только если не используется кастомный AnimInstance)
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!Cast<UBTZBaseCharacterAnimInstance>(AnimInstance))
	{
		UpdateAnimationBlueprintIK();
	}

	// Debug IK - show raw and interpolated values
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(2, 1.0f, FColor::White, FString::Printf(TEXT("IK Final - Left: %.2f, Right: %.2f"), IKLeftFootOffset, IKRightFootOffset));
		GEngine->AddOnScreenDebugMessage(3, 1.0f, FColor::Green, FString::Printf(TEXT("IK Scale: %.2f, TraceDist: %.2f"), IKScale, IKTraceDistance));

		// Check if sockets exist
		if (GetMesh())
		{
			bool bLeftSocketExists = GetMesh()->DoesSocketExist(LeftFootSocketName);
			bool bRightSocketExists = GetMesh()->DoesSocketExist(RightFootSocketName);
			GEngine->AddOnScreenDebugMessage(4, 1.0f, FColor::Orange, FString::Printf(TEXT("Sockets exist - Left: %s, Right: %s"),
				bLeftSocketExists ? TEXT("YES") : TEXT("NO"), bRightSocketExists ? TEXT("YES") : TEXT("NO")));
		}
	}
}

void ABTZBaseCharacter::OnSprintStart_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("ABTZBaseCharacter::OnSprintStart_Implementation()"));
}

void ABTZBaseCharacter::OnSprintEnd_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("ABTZBaseCharacter::OnSprintEnd_Implementation()"));
}

bool ABTZBaseCharacter::CanSprint()
{
	return true;
}

void ABTZBaseCharacter::OnProneStart()
{
	//Capsule = CharacterOwner->GetCapsuleComponent();
	//SetCapsuleSize(1, 1);
	//GEngine->AddOnScreenDebugMessage(0, 1.0f, FColor::Yellow, FString::Printf(TEXT("%.2f"), Capsule));
}

void ABTZBaseCharacter::TryChangeSprintState(float DeltaTime)
{
	if ((BTZBaseCharMovementComponent->IsSprinting() || BTZBaseCharMovementComponent->IsFalling()) && !BTZBaseCharMovementComponent->IsProning())
	{
		CurrentStamina -= SprintStaminaConsumptionVelocity * DeltaTime;
	}
	if (CurrentStamina <= StaminaTiredThreshold)
	{
		GEngine->AddOnScreenDebugMessage(1, 1.0f, FColor::Red, FString::Printf(TEXT("Stamina is 20%")));
		BTZBaseCharMovementComponent->SetIsOutOfStamina(true);
		BTZBaseCharMovementComponent->CanAttemptJump();
	}
	if (CurrentStamina >= StaminaCanSprintAndJumpThreshold && !BTZBaseCharMovementComponent->IsProning())
	{
		BTZBaseCharMovementComponent->SetIsOutOfStamina(false);
		if (bIsSprintRequested && !BTZBaseCharMovementComponent->IsSprinting() && CanSprint())
		{
			BTZBaseCharMovementComponent->StartSprint();
			OnSprintStart();
		}
	}
	if (!bIsSprintRequested && BTZBaseCharMovementComponent->IsSprinting() )
	{
		BTZBaseCharMovementComponent->StopSprint();
		OnSprintEnd();
	}
}

float ABTZBaseCharacter::GetIKOffsetForASocket(const FName& SocketName)
{
	float Result = 0.0f;

	if (!GetMesh())
	{
		return Result;
	}

	FVector SocketLocation = GetMesh()->GetSocketLocation(SocketName);

	// Check if socket exists - use a more reliable method
	if (!GetMesh()->DoesSocketExist(SocketName))
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(3, 1.0f, FColor::Red, FString::Printf(TEXT("IK Socket %s not found!"), *SocketName.ToString()));
		}
		return Result;
	}

    // ИСПРАВЛЕНО: Оптимизированная трассировка для лучшего обнаружения земли
    // Начинаем немного выше сокета и трассируем вниз
    FVector TraceStart = SocketLocation + FVector(0.f, 0.f, 15.0f); // Увеличено для надежности
    FVector TraceEnd = SocketLocation - FVector(0.f, 0.f, IKTraceDistance);

    // Debug trace visualization
    if (GEngine && SocketName == LeftFootSocketName)
    {
    	GEngine->AddOnScreenDebugMessage(9, 1.0f, FColor::Purple, FString::Printf(TEXT("Trace: Start(%.1f, %.1f, %.1f) -> End(%.1f, %.1f, %.1f)"),
    		TraceStart.X, TraceStart.Y, TraceStart.Z, TraceEnd.X, TraceEnd.Y, TraceEnd.Z));
    	GEngine->AddOnScreenDebugMessage(12, 1.0f, FColor::Yellow, FString::Printf(TEXT("Socket Location: (%.1f, %.1f, %.1f)"), 
    		SocketLocation.X, SocketLocation.Y, SocketLocation.Z));
    }
    
	FHitResult HitResult;
	// ИСПРАВЛЕНО: Используем более широкий диапазон коллизий для лучшего обнаружения
	ETraceTypeQuery TraceType = UEngineTypes::ConvertToTraceType(ECC_WorldStatic);

	// Исключаем персонажа из трассировки
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);

    if (UKismetSystemLibrary::LineTraceSingle(GetWorld(), TraceStart, TraceEnd, TraceType, true, ActorsToIgnore, EDrawDebugTrace::ForOneFrame, HitResult, true))
	{
        // ИСПРАВЛЕНО: Правильный расчет смещения для ИК
        // Если земля ниже сокета - нога должна опуститься (отрицательное смещение)
        // Если земля выше сокета - нога должна подняться (положительное смещение)
        float RawOffset = SocketLocation.Z - HitResult.Location.Z;
        Result = RawOffset;

        // Clamp result to reasonable bounds (в сантиметрах) - увеличены границы
        Result = FMath::Clamp(Result, -25.0f, 25.0f);

        if (GEngine && SocketName == LeftFootSocketName)
        {
        	GEngine->AddOnScreenDebugMessage(4, 1.0f, FColor::Blue, FString::Printf(TEXT("IK Hit: SocketZ=%.1f, HitZ=%.1f, RawOffset=%.1f, Result=%.2f"),
        		SocketLocation.Z, HitResult.Location.Z, RawOffset, Result));
        }
	}
    else
    {
    	// ИСПРАВЛЕНО: Улучшенный fallback trace с большим расстоянием
    	FVector FallbackTraceEnd = SocketLocation - FVector(0.f, 0.f, 80.0f); // Увеличено для лучшего обнаружения
    	if (UKismetSystemLibrary::LineTraceSingle(GetWorld(), TraceStart, FallbackTraceEnd, TraceType, true, ActorsToIgnore, EDrawDebugTrace::ForOneFrame, HitResult, true))
    	{
    		float RawOffset = SocketLocation.Z - HitResult.Location.Z;
    		Result = RawOffset;

    		// Clamp result to reasonable bounds
    		Result = FMath::Clamp(Result, -25.0f, 25.0f);

    		if (GEngine && SocketName == LeftFootSocketName)
    		{
    			GEngine->AddOnScreenDebugMessage(5, 1.0f, FColor::Orange, FString::Printf(TEXT("IK Fallback Hit: RawOffset=%.1f, Result=%.2f"), RawOffset, Result));
    		}
    	}
    	else
    	{
    		if (GEngine && SocketName == LeftFootSocketName)
    		{
    			GEngine->AddOnScreenDebugMessage(6, 1.0f, FColor::Red, TEXT("IK No hit at all!"));
    		}
    		// Если не найдена земля, возвращаем 0 - нога остается в исходном положении
    		Result = 0.0f;
    	}
    }

	return Result;
}

void ABTZBaseCharacter::UpdateAnimationBlueprintIK()
{
	// Получаем AnimInstance
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance)
	{
		return;
	}

	// Проверяем, является ли это нашим кастомным AnimInstance
	UBTZBaseCharacterAnimInstance* BTZAnimInstance = Cast<UBTZBaseCharacterAnimInstance>(AnimInstance);
	if (BTZAnimInstance)
	{
		// Если это наш AnimInstance, IK данные уже обновляются в NativeUpdateAnimation
		// Ничего дополнительного делать не нужно
		return;
	}

	// Для других AnimInstance - используем стандартный метод поиска свойств
	UAnimBlueprintGeneratedClass* AnimBPClass = Cast<UAnimBlueprintGeneratedClass>(AnimInstance->GetClass());
	if (!AnimBPClass)
	{
		return;
	}

	// Находим свойства для IK - используем стандартные имена
	FProperty* LeftFootProperty = AnimBPClass->FindPropertyByName(TEXT("LeftFootEffectorLocation"));
	FProperty* RightFootProperty = AnimBPClass->FindPropertyByName(TEXT("RightFootEffectorLocation"));

	if (LeftFootProperty && RightFootProperty)
	{
		// Получаем указатели на свойства
		FVector* LeftFootPtr = LeftFootProperty->ContainerPtrToValuePtr<FVector>(AnimInstance);
		FVector* RightFootPtr = RightFootProperty->ContainerPtrToValuePtr<FVector>(AnimInstance);

		if (LeftFootPtr && RightFootPtr)
		{
			// Устанавливаем значения IK
			LeftFootPtr->Z = IKLeftFootOffset;
			RightFootPtr->Z = IKRightFootOffset;

			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(7, 1.0f, FColor::Magenta, 
					FString::Printf(TEXT("IK Updated in ABP - Left: %.2f, Right: %.2f"), 
						IKLeftFootOffset, IKRightFootOffset));
			}
		}
	}
	else
	{
		// Альтернативный метод - попробуем вызвать Blueprint функции
		UFunction* SetLeftFootIK = AnimInstance->GetClass()->FindFunctionByName(TEXT("SetLeftFootIK"));
		UFunction* SetRightFootIK = AnimInstance->GetClass()->FindFunctionByName(TEXT("SetRightFootIK"));
		
		if (SetLeftFootIK && SetRightFootIK)
		{
			// Вызываем Blueprint функции
			AnimInstance->ProcessEvent(SetLeftFootIK, &IKLeftFootOffset);
			AnimInstance->ProcessEvent(SetRightFootIK, &IKRightFootOffset);
			
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(11, 1.0f, FColor::Cyan, 
					FString::Printf(TEXT("IK Updated via Blueprint functions - Left: %.2f, Right: %.2f"), 
						IKLeftFootOffset, IKRightFootOffset));
			}
		}
		else
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(8, 1.0f, FColor::Red, 
					TEXT("IK Properties not found in Animation Blueprint! Need to create LeftFootEffectorLocation and RightFootEffectorLocation variables."));
			}
		}
	}
}

void ABTZBaseCharacter::TryClimbObstacle()
{
	// Базовая реализация - ничего не делает
	// Переопределяется в PlayerCharacter
}