// Fill out your copyright notice in the Description page of Project Settings.

#include "ObstacleClimbingComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UObstacleClimbingComponent::UObstacleClimbingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UObstacleClimbingComponent::BeginPlay()
{
    Super::BeginPlay();
    OwnerActor = GetOwner();
}

void UObstacleClimbingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsClimbing)
    {
		UpdateClimbing(DeltaTime);
	}
	else
	{
		// АВТОМАТИЧЕСКОЕ ОБНАРУЖЕНИЕ препятствий каждый кадр
		// Это позволит показывать UI-подсказку игроку ДО нажатия клавиши
		FObstacleData DetectedObstacle = DetectObstacle();
		
		if (DetectedObstacle.IsValid())
		{
			bObstacleDetected = true;
			DetectedObstacleType = DetectedObstacle.Type;
			
			// Сохраняем обнаруженное препятствие для быстрого доступа при нажатии клавиши
			CurrentObstacle = DetectedObstacle;
    }
    else
    {
			bObstacleDetected = false;
			DetectedObstacleType = EObstacleType::None;
		}
	}
}

// ============================================================================
// ПУБЛИЧНЫЕ ФУНКЦИИ
// ============================================================================

bool UObstacleClimbingComponent::TryClimb()
{
	// Проверка: уже лезем?
    if (bIsClimbing)
    {
        return false;
    }

	// Используем уже обнаруженное препятствие из Tick
	if (!bObstacleDetected || !CurrentObstacle.IsValid())
    {
        if (GEngine)
        {
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, 
				TEXT("❌ Препятствие не обнаружено"));
        }
        return false;
    }

	// Начинаем лазание с уже обнаруженным препятствием
	StartClimbing(CurrentObstacle);
    return true;
}

// ============================================================================
// ОБНАРУЖЕНИЕ ПРЕПЯТСТВИЙ
// ============================================================================

FObstacleData UObstacleClimbingComponent::DetectObstacle()
{
	FObstacleData Result;

    ACharacter* Character = Cast<ACharacter>(OwnerActor);
    if (!Character)
	{
    if (GEngine)
    {
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
				TEXT("❌ DetectObstacle: Character is NULL"));
		}
		return Result;
	}

	// Получаем параметры персонажа
	UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
	float CapsuleRadius = Capsule->GetScaledCapsuleRadius();
	float CapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
	
	FVector CharacterLocation = Character->GetActorLocation();
	FVector ForwardVector = Character->GetActorForwardVector();

	// Debug-сообщения убраны, так как функция вызывается каждый кадр

	// Трассировка вперед на 20 см от низа капсулы
	// CharacterLocation находится в центре капсулы, поэтому вычитаем CapsuleHalfHeight и добавляем 20 см
	float TraceHeight = -CapsuleHalfHeight + 20.0f;
	FVector TraceStart = CharacterLocation + FVector(0, 0, TraceHeight);
	FVector TraceEnd = TraceStart + ForwardVector * DetectionDistance;

	// Трассировка настроена

    FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Character);

	// ВАЖНО: Добавляем debug-рисование трассировки
	DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 3.0f, 0, 3.0f);

	// Первая трассировка: есть ли препятствие впереди?
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult, TraceStart, TraceEnd, 
		ECC_Visibility, QueryParams
	);

	// Результат трассировки обрабатывается автоматически

	if (!bHit)
	{
		return Result; // Нет препятствия
	}

	// Сохраняем точку столкновения
	FVector ObstacleHitPoint = HitResult.Location;
	FVector ObstacleNormal = HitResult.Normal;

	// Препятствие найдено

	// Определяем высоту препятствия: трассируем вверх от точки столкновения
	FVector TopTraceStart = ObstacleHitPoint + FVector(0, 0, 10.0f);
	FVector TopTraceEnd = TopTraceStart + FVector(0, 0, 300.0f);

	FHitResult TopHitResult;
	bool bTopHit = GetWorld()->LineTraceSingleByChannel(
		TopHitResult, TopTraceEnd, TopTraceStart, 
		ECC_Visibility, QueryParams
	);

	if (!bTopHit)
	{
		return Result; // Не нашли верх препятствия
	}

	FVector ObstacleTopPoint = TopHitResult.Location;
	
	// ИСПРАВЛЕНО: Высота = расстояние от низа капсулы до верха препятствия
	// Это более точно, чем от центра персонажа
	float CapsuleBottomZ = CharacterLocation.Z - CapsuleHalfHeight;
	float ObstacleHeight = ObstacleTopPoint.Z - CapsuleBottomZ;

	// Высота определена точно от низа капсулы

	// Проверяем минимальную высоту
	if (ObstacleHeight < MinObstacleHeight)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, 
				TEXT("⚠️ Препятствие слишком низкое"));
		}
		return Result;
	}

	// Проверяем максимальную высоту
	if (ObstacleHeight > MaxClimbHeight)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, 
				FString::Printf(TEXT("❌ Препятствие слишком высокое: %.1f см (макс. %.1f см)"), 
					ObstacleHeight, MaxClimbHeight));
		}
		return Result;
	}

	// Определяем толщину препятствия: трассируем вперед от верхней точки
	FVector ThicknessTraceStart = ObstacleTopPoint + FVector(0, 0, 10.0f);
	FVector ThicknessTraceEnd = ThicknessTraceStart + ForwardVector * (DetectionDistance * 2.0f);

	FHitResult ThicknessHitResult;
	bool bThicknessHit = GetWorld()->LineTraceSingleByChannel(
		ThicknessHitResult, ThicknessTraceStart, ThicknessTraceEnd, 
		ECC_Visibility, QueryParams
	);

	float ObstacleThickness = 50.0f; // По умолчанию
	if (bThicknessHit)
	{
		ObstacleThickness = FVector::Dist(ObstacleHitPoint, ThicknessHitResult.Location);
	}

	// Толщина определена

	// Проверяем максимальную толщину
	if (ObstacleThickness > MaxObstacleThickness)
                {
                    if (GEngine)
                    {
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, 
				TEXT("⚠️ Препятствие слишком толстое"));
		}
            return Result;
	}

	// Заполняем результат
	Result.Type = DetermineObstacleType(ObstacleHeight, ObstacleThickness);
	Result.ObstacleTop = ObstacleTopPoint;
	Result.ObstacleForward = ForwardVector;
	Result.Height = ObstacleHeight;
	Result.Thickness = ObstacleThickness;

	// Визуализация для отладки
	DrawDebugSphere(GetWorld(), ObstacleTopPoint, 20.0f, 12, FColor::Green, false, 3.0f);
	DrawDebugLine(GetWorld(), TraceStart, ObstacleHitPoint, FColor::Yellow, false, 3.0f, 0, 2.0f);
	DrawDebugLine(GetWorld(), TopTraceEnd, ObstacleTopPoint, FColor::Cyan, false, 3.0f, 0, 2.0f);
    
    return Result;
}

EObstacleType UObstacleClimbingComponent::DetermineObstacleType(float Height, float Thickness)
{
	if (Height <= VaultMaxHeight)
    {
        if (GEngine)
        {
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
				TEXT("✅ Тип: VAULT (перепрыгивание)"));
        }
		return EObstacleType::Vault;
    }
	else if (Height <= MantleMaxHeight)
    {
        if (GEngine)
        {
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, 
				TEXT("✅ Тип: MANTLE (подтягивание)"));
		}
		return EObstacleType::Mantle;
	}
	else
        {
            if (GEngine)
            {
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Purple, 
				TEXT("✅ Тип: CLIMB (лазание)"));
		}
		return EObstacleType::Climb;
	}
}

// ============================================================================
// ВЫПОЛНЕНИЕ ЛАЗАНИЯ
// ============================================================================

void UObstacleClimbingComponent::StartClimbing(const FObstacleData& Obstacle)
{
	ACharacter* Character = Cast<ACharacter>(OwnerActor);
	if (!Character) return;

	// Сохраняем данные
	CurrentObstacle = Obstacle;
	CurrentObstacleType = Obstacle.Type;
	bIsClimbing = true;
	AnimationProgress = 0.0f;

	// Сохраняем стартовую позицию и ротацию
	StartLocation = Character->GetActorLocation();
	StartRotation = Character->GetActorRotation();

	// Вычисляем целевую позицию
	CalculateTargetTransform(Obstacle);

	// Блокируем управление
	DisablePlayerControl();

	// Уведомляем об начале лазания
	OnClimbStarted.Broadcast(CurrentObstacleType);
    
    if (GEngine)
    {
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
			FString::Printf(TEXT("🎬 СТАРТ ЛАЗАНИЯ: %s"), 
				CurrentObstacleType == EObstacleType::Vault ? TEXT("VAULT") :
				CurrentObstacleType == EObstacleType::Mantle ? TEXT("MANTLE") : TEXT("CLIMB")));
	}
}

void UObstacleClimbingComponent::UpdateClimbing(float DeltaTime)
{
	ACharacter* Character = Cast<ACharacter>(OwnerActor);
	if (!Character) return;

	// Определяем длительность анимации
	float Duration = VaultDuration;
	switch (CurrentObstacleType)
	{
		case EObstacleType::Vault:  Duration = VaultDuration;  break;
		case EObstacleType::Mantle: Duration = MantleDuration; break;
		case EObstacleType::Climb:  Duration = ClimbDuration;  break;
		default: break;
	}

	// Обновляем прогресс
	AnimationProgress += DeltaTime / Duration;

	if (AnimationProgress >= 1.0f)
	{
		// Анимация завершена
		CompleteClimbing();
		return;
	}

	// Интерполяция позиции и ротации
	FVector NewLocation = GetInterpolatedLocation(AnimationProgress);
	FRotator NewRotation = GetInterpolatedRotation(AnimationProgress);

	// Применяем новую позицию
	Character->SetActorLocationAndRotation(NewLocation, NewRotation, false, nullptr, ETeleportType::TeleportPhysics);

	// Debug
	static float DebugTimer = 0.0f;
	DebugTimer += DeltaTime;
	if (DebugTimer >= 0.5f)
	{
		DebugTimer = 0.0f;
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Cyan, 
				FString::Printf(TEXT("🎬 Прогресс: %.0f%% | Yaw: %.1f°"), 
					AnimationProgress * 100.0f, NewRotation.Yaw));
		}
	}
}

void UObstacleClimbingComponent::CompleteClimbing()
{
    ACharacter* Character = Cast<ACharacter>(OwnerActor);
	if (!Character) return;

	// Устанавливаем финальную позицию
	Character->SetActorLocationAndRotation(TargetLocation, TargetRotation, false, nullptr, ETeleportType::TeleportPhysics);

	// Восстанавливаем управление
	EnablePlayerControl();

	// Сбрасываем состояние
	bIsClimbing = false;
	CurrentObstacleType = EObstacleType::None;
	AnimationProgress = 0.0f;

	// Уведомляем о завершении
	OnClimbCompleted.Broadcast();
    
    if (GEngine)
    {
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
			TEXT("✅ ЛАЗАНИЕ ЗАВЕРШЕНО"));
	}
}

void UObstacleClimbingComponent::CancelClimbing()
{
	ACharacter* Character = Cast<ACharacter>(OwnerActor);
	if (!Character) return;

	// Восстанавливаем управление
	EnablePlayerControl();

	// Сбрасываем состояние
	bIsClimbing = false;
	CurrentObstacleType = EObstacleType::None;
	AnimationProgress = 0.0f;

        if (GEngine)
        {
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, 
			TEXT("⚠️ ЛАЗАНИЕ ОТМЕНЕНО"));
	}
}

// ============================================================================
// ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ
// ============================================================================

void UObstacleClimbingComponent::CalculateTargetTransform(const FObstacleData& Obstacle)
{
    ACharacter* Character = Cast<ACharacter>(OwnerActor);
	if (!Character) return;

	// Получаем параметры капсулы
	UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
	float CapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();

	// Целевая ротация - в направлении препятствия
	FVector ForwardDir = Obstacle.ObstacleForward;
	TargetRotation = ForwardDir.Rotation();

	// Целевая позиция зависит от типа препятствия
	FVector BaseTargetLocation = Obstacle.ObstacleTop;

	switch (Obstacle.Type)
	{
		case EObstacleType::Vault:
			// Vault: приземляемся за препятствием, на уровне земли
			TargetLocation = BaseTargetLocation + ForwardDir * (Obstacle.Thickness + 80.0f);
			TargetLocation.Z = StartLocation.Z;
        break;

		case EObstacleType::Mantle:
			// Mantle: встаем на верх препятствия
			TargetLocation = BaseTargetLocation + ForwardDir * 50.0f;
			TargetLocation.Z = BaseTargetLocation.Z + CapsuleHalfHeight;
        break;

		case EObstacleType::Climb:
			// Climb: встаем на верх высокого препятствия
			TargetLocation = BaseTargetLocation + ForwardDir * 60.0f;
			TargetLocation.Z = BaseTargetLocation.Z + CapsuleHalfHeight;
        break;

    default:
			TargetLocation = StartLocation;
        break;
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Magenta, 
			FString::Printf(TEXT("🎯 Цель: (%.1f, %.1f, %.1f)"), 
				TargetLocation.X, TargetLocation.Y, TargetLocation.Z));
	}

	// Визуализация целевой позиции
	DrawDebugSphere(GetWorld(), TargetLocation, 30.0f, 12, FColor::Red, false, 5.0f);
	DrawDebugLine(GetWorld(), StartLocation, TargetLocation, FColor::Green, false, 5.0f, 0, 3.0f);
}

void UObstacleClimbingComponent::DisablePlayerControl()
{
    ACharacter* Character = Cast<ACharacter>(OwnerActor);
	if (!Character) return;

	// Останавливаем движение
	UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
	if (MovementComp)
	{
		MovementComp->DisableMovement();
		MovementComp->StopMovementImmediately();
	}

	// Блокируем ввод
	APlayerController* PC = Cast<APlayerController>(Character->GetController());
	if (PC)
	{
		PC->SetIgnoreMoveInput(true);
		PC->SetIgnoreLookInput(true);
	}

	// Отключаем коллизию капсулы временно
	UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
	if (Capsule)
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
    
    if (GEngine)
    {
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
			TEXT("🔒 Управление заблокировано"));
    }
}

void UObstacleClimbingComponent::EnablePlayerControl()
{
    ACharacter* Character = Cast<ACharacter>(OwnerActor);
	if (!Character) return;

	// Восстанавливаем движение
	UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
	if (MovementComp)
	{
		MovementComp->SetMovementMode(MOVE_Walking);
	}

	// Разблокируем ввод
	APlayerController* PC = Cast<APlayerController>(Character->GetController());
	if (PC)
	{
		PC->ResetIgnoreMoveInput();
		PC->ResetIgnoreLookInput();
	}

	// Включаем коллизию обратно
	UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
	if (Capsule)
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        }
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
			TEXT("🔓 Управление восстановлено"));
	}
}

FVector UObstacleClimbingComponent::GetInterpolatedLocation(float Progress) const
{
	// Используем сглаженную интерполяцию для более естественного движения
	float SmoothProgress = FMath::SmoothStep(0.0f, 1.0f, Progress);
	
	// Для Vault добавляем параболическую дугу по высоте
	FVector Result = FMath::Lerp(StartLocation, TargetLocation, SmoothProgress);
	
	if (CurrentObstacleType == EObstacleType::Vault)
	{
		// Добавляем дугу высоты для прыжка
		float ArcHeight = CurrentObstacle.Height * 0.5f + 50.0f;
		float ArcProgress = FMath::Sin(Progress * PI);
		Result.Z += ArcHeight * ArcProgress;
	}
	
	return Result;
}

FRotator UObstacleClimbingComponent::GetInterpolatedRotation(float Progress) const
{
	// Плавная интерполяция ротации
	float SmoothProgress = FMath::SmoothStep(0.0f, 1.0f, Progress);
	return FMath::Lerp(StartRotation, TargetRotation, SmoothProgress);
}
