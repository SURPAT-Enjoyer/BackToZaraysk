// Fill out your copyright notice in the Description page of Project Settings.


#include "BTZBaseCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/SpringArmComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimBlueprintGeneratedClass.h"
#include "Animations/BTZBaseCharacterAnimInstance.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ABTZBaseCharacter::ABTZBaseCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UBTZBaseCharMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	BTZBaseCharMovementComponent = StaticCast<UBTZBaseCharMovementComponent*>(GetCharacterMovement());

	// Убираем масштабирование для более точных IK расчетов
	IKScale = 1.0f;

    // Trace / IK socket names must match ABP usage.
    // We use the mannequin convention bones: ik_foot_l / ik_foot_r.
    LeftFootSocketName = FName(TEXT("ik_foot_l"));
    RightFootSocketName = FName(TEXT("ik_foot_r"));

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
        // debug off
	}
	else
	{
		Crouch();
		//UnProne();
        // debug off
	}
}

void ABTZBaseCharacter::ChangeProneState()
{
	if (BTZBaseCharMovementComponent->IsProning())
	{
		BTZBaseCharMovementComponent->UnProne();
		//UnCrouch();
        // debug off
	}
	else
	{
		BTZBaseCharMovementComponent->Prone();
		//UnCrouch();
        // debug off
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
    // debug off
	// ИСПРАВЛЕНО: Оптимизированное расстояние трассировки для лучшего обнаружения
	// Используем большее расстояние для более надежного обнаружения неровностей
	IKTraceDistance = 100.0f; // Увеличено для лучшего обнаружения земли

	// Startup guard: clear IK offsets/hit cache so PIE start doesn't apply stale effector targets.
	IKLeftFootOffset = 0.0f;
	IKRightFootOffset = 0.0f;
	LastLeftFootHitZ = 0.0f;
	LastRightFootHitZ = 0.0f;
	bLastLeftFootHadHit = false;
	bLastRightFootHadHit = false;

	if (bDebugFootIK && GEngine && GetMesh())
	{
		// Users can disable on-screen messages via console command (DisableAllScreenMessages).
		// For IK debugging we force-enable them so we always see HIT/MISS diagnostics.
		GEngine->bEnableOnScreenDebugMessages = true;
		GEngine->bEnableOnScreenDebugMessagesDisplay = true;

		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			const bool bIsBTZ = (Cast<UBTZBaseCharacterAnimInstance>(AnimInstance) != nullptr);
			GEngine->AddOnScreenDebugMessage(901, 8.0f, FColor::Cyan,
				FString::Printf(TEXT("IK Debug: AnimInstance=%s (BTZ=%s)"),
					*AnimInstance->GetClass()->GetName(),
					bIsBTZ ? TEXT("YES") : TEXT("NO")));
			UE_LOG(LogTemp, Warning, TEXT("IK Debug: AnimInstance=%s (BTZ=%s)"),
				*AnimInstance->GetClass()->GetName(),
				bIsBTZ ? TEXT("YES") : TEXT("NO"));
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(901, 8.0f, FColor::Red, TEXT("IK Debug: Mesh has no AnimInstance"));
			UE_LOG(LogTemp, Warning, TEXT("IK Debug: Mesh has no AnimInstance"));
		}
	}
}

void ABTZBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	BTZBaseCharMovementComponent->GetMaxSpeed();
	TryChangeSprintState(DeltaTime);
    // debug off
	// Восстанавливаем выносливость только когда персонаж не спринтует и не падает
	if (!BTZBaseCharMovementComponent->IsSprinting() && !BTZBaseCharMovementComponent->IsFalling())
	{
		CurrentStamina += StaminaRestoreVelocity * DeltaTime;
		CurrentStamina = FMath::Clamp(CurrentStamina, 0.0f, MaxStamina);
	}

	// ИСПРАВЛЕНО: Улучшенная логика ИК
	bool bIsOnGround = GetCharacterMovement()->IsMovingOnGround();
	bool bIsMoving = GetVelocity().Size() > 10.0f; // Движется ли персонаж
	
	// Получаем сырые смещения
	float RawLeftOffset = GetIKOffsetForFoot(true);
	float RawRightOffset = GetIKOffsetForFoot(false);
	
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
	if (bDebugFootIK && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(902, 10.0f, FColor::Yellow,
			FString::Printf(TEXT("IK Debug: L=%.2f R=%.2f OnGround=%d Vel=%.1f"),
				IKLeftFootOffset, IKRightFootOffset,
				GetCharacterMovement() ? (int32)GetCharacterMovement()->IsMovingOnGround() : 0,
				GetVelocity().Size()));
		UE_LOG(LogTemp, Warning, TEXT("IK Debug Tick: L=%.2f R=%.2f OnGround=%d Vel=%.1f"),
			IKLeftFootOffset, IKRightFootOffset,
			GetCharacterMovement() ? (int32)GetCharacterMovement()->IsMovingOnGround() : 0,
			GetVelocity().Size());
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
        // debug off
		BTZBaseCharMovementComponent->SetIsOutOfStamina(true);
		// CanAttemptJump() вызывается автоматически движком при попытке прыжка
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

FVector ABTZBaseCharacter::GetFootTraceOriginWorld(bool bLeft) const
{
	// Trace origin anchored to the mesh foot location (pre-IK) so stairs can be detected correctly.
	const USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp)
	{
		return GetActorLocation();
	}

	const FName FootName = bLeft ? LeftFootSocketName : RightFootSocketName;
	if (FootName.IsNone())
	{
		return GetActorLocation();
	}

	const FVector FootWorld = MeshComp->GetSocketLocation(FootName);

	FVector Origin = FootWorld;
	Origin.X += FootTraceForwardOffsetCm;
	Origin.Y += (bLeft ? -FootTraceLateralOffsetCm : FootTraceLateralOffsetCm);
	// Move down from socket to (approx) bottom of foot mesh.
	Origin.Z -= FootTraceSocketToFootBottomOffsetCm;
	return Origin;
}

float ABTZBaseCharacter::GetIKOffsetForFoot(bool bLeft)
{
	float Result = 0.0f;

	const bool bIsDebugFoot = bDebugFootIK && bLeft;
	FVector OriginWorld = GetFootTraceOriginWorld(bLeft);
	float FootSocketZ = OriginWorld.Z;

	// For debug: actual socket Z (before subtracting FootTraceSocketToFootBottomOffsetCm).
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		const FName FootName = bLeft ? LeftFootSocketName : RightFootSocketName;
		if (!FootName.IsNone())
		{
			FootSocketZ = MeshComp->GetSocketLocation(FootName).Z;
		}
	}

    // ИСПРАВЛЕНО: Оптимизированная трассировка для лучшего обнаружения земли
    // Начинаем немного выше сокета и трассируем вниз
	const float StartZ = OriginWorld.Z + FootTraceStartAboveCm;
	const float EndZ = OriginWorld.Z - IKTraceDistance;
	FVector TraceStart(OriginWorld.X, OriginWorld.Y, StartZ);
	FVector TraceEnd(OriginWorld.X, OriginWorld.Y, EndZ);

    // Debug trace visualization
    // debug off
    
	FHitResult HitResult = FHitResult();
	FHitResult HitResultDown = FHitResult();
	FHitResult HitResultUp = FHitResult();
	// ВАЖНО: разные ассеты по-разному отвечают на complex/simple.
	// Поэтому делаем попытки по каналам и по режимам complex/simple.
	const ECollisionChannel ChannelsToTry[] = { ECC_Visibility, ECC_WorldStatic, ECC_WorldDynamic };

	// Исключаем персонажа из трассировки
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);

	UWorld* World = GetWorld();
	if (!World)
	{
		return Result;
	}

	const auto DoTrace = [&](ECollisionChannel Channel, const FVector& Start, const FVector& End, bool bTraceComplex) -> bool
	{
		FCollisionQueryParams Params(SCENE_QUERY_STAT(FootIKTrace), bTraceComplex);
		Params.AddIgnoredActors(ActorsToIgnore);
		return World->LineTraceSingleByChannel(HitResult, Start, End, Channel, Params);
	};

	// Two-direction trace:
	// - Down: Start(high) -> End(low)
	// - Up:   Start(low)  -> End(high)
	// When the foot is clipped into the surface, the first intersection depends on direction,
	// so we pick the hit closer to OriginZ to get the "correct side" of the mesh.
	const FVector TraceStartLow = TraceEnd;
	const FVector TraceEndHigh = TraceStart;

	bool bHitDown = false;
	bool bHitUp = false;
	ECollisionChannel HitChannelDown = ECC_Visibility;
	ECollisionChannel HitChannelUp = ECC_Visibility;
	bool bHitComplexDown = true;
	bool bHitComplexUp = true;

	// Down trace
	for (ECollisionChannel Ch : ChannelsToTry)
	{
		if (DoTrace(Ch, TraceStart, TraceEnd, true))
		{
			bHitDown = true;
			HitResultDown = HitResult;
			HitChannelDown = Ch;
			bHitComplexDown = true;
			break;
		}
		if (DoTrace(Ch, TraceStart, TraceEnd, false))
		{
			bHitDown = true;
			HitResultDown = HitResult;
			HitChannelDown = Ch;
			bHitComplexDown = false;
			break;
		}
	}

	// Up trace (reverse direction)
	for (ECollisionChannel Ch : ChannelsToTry)
	{
		if (DoTrace(Ch, TraceStartLow, TraceEndHigh, true))
		{
			bHitUp = true;
			HitResultUp = HitResult;
			HitChannelUp = Ch;
			bHitComplexUp = true;
			break;
		}
		if (DoTrace(Ch, TraceStartLow, TraceEndHigh, false))
		{
			bHitUp = true;
			HitResultUp = HitResult;
			HitChannelUp = Ch;
			bHitComplexUp = false;
			break;
		}
	}

	if (bHitDown || bHitUp)
	{
		const float RawDown = bHitDown ? (HitResultDown.ImpactPoint.Z - OriginWorld.Z) : FLT_MAX;
		const float RawUp = bHitUp ? (HitResultUp.ImpactPoint.Z - OriginWorld.Z) : FLT_MAX;

		// Choose hit:
		// - If penetrating: prefer UP only if it exists (to help exit geometry).
		//   If only DOWN exists, we will still apply a penetration lift below.
		// - If NOT penetrating and we have BOTH hits: choose the hit with smaller abs(raw)
		//   (closest to OriginZ). This avoids choosing the "wrong side" hit on thin/double-sided meshes.
		bool bChooseUp = false;

		// Penetration test:
		// If the foot trace origin is currently inside world collision (clipping),
		// prefer the UP hit to move the foot out.
		bool bIsPenetrating = false;
		{
			const FCollisionShape Sphere = FCollisionShape::MakeSphere(FootPenetrationTestRadiusCm);
			FCollisionQueryParams Params(SCENE_QUERY_STAT(FootIKOverlap), false);
			Params.AddIgnoredActors(ActorsToIgnore);

			for (ECollisionChannel Ch : ChannelsToTry)
			{
				if (World->OverlapBlockingTestByChannel(OriginWorld, FQuat::Identity, Ch, Sphere, Params))
				{
					bIsPenetrating = true;
					break;
				}
			}
		}

		if (bIsPenetrating)
		{
			bChooseUp = bHitUp; // only true when UP exists
		}
		else
		{
			if (bHitDown && bHitUp)
			{
				bChooseUp = FMath::Abs(RawUp) < FMath::Abs(RawDown);
			}
			else if (bHitUp && !bHitDown)
			{
				bChooseUp = true;
			}
		}

		float ChosenRawOffset = bChooseUp ? RawUp : RawDown;
		const float ChosenHitZ = bChooseUp ? HitResultUp.ImpactPoint.Z : HitResultDown.ImpactPoint.Z;
			const bool bChosenComplex = bChooseUp ? bHitComplexUp : bHitComplexDown;
			const ECollisionChannel ChosenHitChannel = bChooseUp ? HitChannelUp : HitChannelDown;

		// If we are penetrating, bias result upward to resolve "utopanie"
		// even when only DOWN hit is available.
		if (bIsPenetrating)
		{
			ChosenRawOffset += FootPenetrationResolveLiftCm;
		}

		// FLAT: suppress micro jitter when offset is within +/- epsilon.
		{
			const bool bOffsetSmall = FMath::Abs(ChosenRawOffset) <= FootFlatOffsetEpsilonCm;
			const float PrevHitZ = bLeft ? LastLeftFootHitZ : LastRightFootHitZ;
			const bool bPrevHadHit = bLeft ? bLastLeftFootHadHit : bLastRightFootHadHit;

			const float Speed2D = GetVelocity().Size2D();
			const bool bIsNearlyStopped = Speed2D <= FootFlatMaxSpeedCmPerSec;

			// Require hit stability to consider it truly flat.
			const bool bHitStable =
				bPrevHadHit &&
				FMath::Abs(ChosenHitZ - PrevHitZ) <= FootFlatHitZDeltaEpsilonCm;

			if (!bIsPenetrating && bIsNearlyStopped && bOffsetSmall && bHitStable)
			{
				ChosenRawOffset = 0.0f;
			}
		}

		// Visual debug:
		// - Red line: DOWN trace
		// - Blue line: UP trace
		// - Green/Cyan spheres: raw hits
		// - Yellow sphere: chosen contact
		if (bDebugFootIK && World)
		{
			DrawDebugLine(World, TraceStart, TraceEnd, FColor::Red, false, 0.15f, 0, 0.7f);			// DOWN
			DrawDebugLine(World, TraceEnd, TraceStart, FColor::Blue, false, 0.15f, 0, 0.7f);			// UP

			if (bHitDown && HitResultDown.bBlockingHit)
			{
				DrawDebugSphere(World, HitResultDown.ImpactPoint, 2.0f, 10, FColor::Green, false, 0.15f, 0);
			}
			if (bHitUp && HitResultUp.bBlockingHit)
			{
				DrawDebugSphere(World, HitResultUp.ImpactPoint, 2.0f, 10, FColor::Cyan, false, 0.15f, 0);
			}

			const FVector ContactPoint =
				bChooseUp
					? (HitResultUp.bBlockingHit ? HitResultUp.ImpactPoint : HitResultUp.Location)
					: (HitResultDown.bBlockingHit ? HitResultDown.ImpactPoint : HitResultDown.Location);

			DrawDebugSphere(World, ContactPoint, 2.4f, 10, FColor::Yellow, false, 0.15f, 0);
		}

		Result = ChosenRawOffset;

        // Clamp result to reasonable bounds (cm). Larger range makes IK more visible,
        // but extreme values still cause bad knee angles.
        Result = FMath::Clamp(Result, -45.0f, 45.0f);

		if (bDebugFootIK && GEngine)
		{
			const FName FootName = bLeft ? LeftFootSocketName : RightFootSocketName;
			GEngine->AddOnScreenDebugMessage(
				bLeft ? 930 : 931,
				0.0f,
				bLeft ? FColor::Green : FColor::Emerald,
				FString::Printf(
					TEXT("IKTrace %s Foot=%s OriginZ=%.1f FootZ=%.1f StartZ=%.1f DownHitZ=%.2f UpHitZ=%.2f DownRaw=%.2f UpRaw=%.2f Pen=%d Ch=%s Raw=%.2f Clamped=%.2f"),
					bLeft ? TEXT("L") : TEXT("R"),
					*FootName.ToString(),
					OriginWorld.Z,
					FootSocketZ,
					TraceStart.Z,
					bHitDown ? HitResultDown.ImpactPoint.Z : -9999.0f,
					bHitUp ? HitResultUp.ImpactPoint.Z : -9999.0f,
					bHitDown ? RawDown : 0.0f,
					bHitUp ? RawUp : 0.0f,
					bIsPenetrating ? 1 : 0,
					bChooseUp ? TEXT("UP") : TEXT("DOWN"),
					ChosenRawOffset,
					Result
				)
			);
		}

		// store last hit z
		if (bLeft)
		{
			LastLeftFootHitZ = ChosenHitZ;
			bLastLeftFootHadHit = true;
		}
		else
		{
			LastRightFootHitZ = ChosenHitZ;
			bLastRightFootHadHit = true;
		}

		if (bIsDebugFoot && GEngine)
		{
			const TCHAR* ChName =
				(ChosenHitChannel == ECC_Visibility) ? TEXT("Visibility") :
				(ChosenHitChannel == ECC_WorldStatic) ? TEXT("WorldStatic") :
				(ChosenHitChannel == ECC_WorldDynamic) ? TEXT("WorldDynamic") :
				TEXT("Other");
			GEngine->AddOnScreenDebugMessage(910, 10.0f, FColor::Green,
				FString::Printf(TEXT("IK Trace HIT (%s, %s) Ch=%s Raw=%.2f Clamped=%.2f HitZ=%.2f FootZ=%.2f"),
					ChName,
					bChosenComplex ? TEXT("complex") : TEXT("simple"),
					bChooseUp ? TEXT("UP") : TEXT("DOWN"),
					ChosenRawOffset, Result,
					ChosenHitZ, OriginWorld.Z));
			UE_LOG(LogTemp, Warning, TEXT("IK Trace HIT (%s, %s) Ch=%s Raw=%.2f Clamped=%.2f HitZ=%.2f FootZ=%.2f"),
				ChName,
				bChosenComplex ? TEXT("complex") : TEXT("simple"),
				bChooseUp ? TEXT("UP") : TEXT("DOWN"),
				ChosenRawOffset, Result,
				ChosenHitZ, OriginWorld.Z);
		}
	}
    else
    {
		// FLAT: no hits in either direction => suppress IK noise on flat ground.
		if (bLeft) bLastLeftFootHadHit = false; else bLastRightFootHadHit = false;
		Result = 0.0f;

		if (bIsDebugFoot && GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				910,
				10.0f,
				FColor::Red,
				FString::Printf(TEXT("IK Trace FLAT (no UP/DOWN hit) StartZ=%.2f EndZ=%.2f SockZ=%.2f Dist=%.1f"),
					TraceStart.Z, TraceEnd.Z, OriginWorld.Z, IKTraceDistance));
			UE_LOG(LogTemp, Warning, TEXT("IK Trace FLAT (no UP/DOWN hit) StartZ=%.2f EndZ=%.2f SockZ=%.2f Dist=%.1f"),
				TraceStart.Z, TraceEnd.Z, OriginWorld.Z, IKTraceDistance);
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

            // debug off
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
			
            // debug off
		}
		else
		{
            // debug off
		}
	}
}

void ABTZBaseCharacter::TryClimbObstacle()
{
	// Базовая реализация - ничего не делает
	// Переопределяется в PlayerCharacter
}

void ABTZBaseCharacter::Turn(float Value)
{
	// Блокируем поворот если активен FreeLook
	if (bRotationBlocked) return;
	
	// Стандартный поворот персонажа
	AddControllerYawInput(Value);
}

void ABTZBaseCharacter::LookUp(float Value)
{
	// Блокируем поворот если активен FreeLook
	if (bRotationBlocked) return;
	
	// Стандартный поворот камеры
	AddControllerPitchInput(Value);
}

void ABTZBaseCharacter::SetRotationBlocked(bool bBlocked)
{
	bRotationBlocked = bBlocked;
	
    // debug off
}

void ABTZBaseCharacter::SetHeadRotation(float Yaw, float Pitch)
{
	// Сохраняем значения поворота головы в градусах
	HeadYawRotation = Yaw;
	HeadPitchRotation = Pitch;
	
	// Применяем поворот головы через Transform (Modify) Bone
    // debug off
}

void ABTZBaseCharacter::ResetHeadRotation()
{
	HeadYawRotation = 0.0f;
	HeadPitchRotation = 0.0f;
	
    // debug off
}