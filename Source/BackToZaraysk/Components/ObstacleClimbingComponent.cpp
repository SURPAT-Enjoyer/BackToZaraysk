#include "ObstacleClimbingComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UObstacleClimbingComponent::UObstacleClimbingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UObstacleClimbingComponent::BeginPlay()
{
    Super::BeginPlay();
    OwnerActor = GetOwner();
}

void UObstacleClimbingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    ACharacter* Character = Cast<ACharacter>(OwnerActor);
    if (!Character) return;

    if (bIsClimbing)
    {
        UpdateClimbAnimation(DeltaTime);
        
        // Убеждаемся, что движение отключено ТОЛЬКО во время активного лазания
        if (Character->GetCharacterMovement()->MovementMode != MOVE_None)
        {
            Character->GetCharacterMovement()->SetMovementMode(MOVE_None);
        }
    }
    else
    {
        // Обрабатываем отложенное включение коллизий
        if (bWaitingForCollisionRestore)
        {
            CollisionRestoreTimer += DeltaTime;
            
            if (CollisionRestoreTimer >= 0.5f)
            {
                // Включаем коллизии ТЕПЕРЬ!
                UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
                Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                Capsule->SetCollisionResponseToAllChannels(ECR_Block);
                Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
                
                bWaitingForCollisionRestore = false;
                CollisionRestoreTimer = 0.0f;
                
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
                        FString::Printf(TEXT("✅ COLLISION ENABLED! Enabled: %d"), 
                            (int32)Capsule->GetCollisionEnabled()));
                }
            }
        }
        
        // DEBUG: Показываем состояние ПОСЛЕ лазания каждые 0.5 секунды
        static float DebugTimer = 0.0f;
        DebugTimer += DeltaTime;
        
        if (DebugTimer >= 0.5f)
        {
            DebugTimer = 0.0f;
            
            UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
            if (GEngine && MovementComp)
            {
                GEngine->AddOnScreenDebugMessage(100, 0.5f, FColor::White, 
                    FString::Printf(TEXT("🔍 Status: Mode=%d, Ground=%s, Vel=%.1f, Collision=%d, Enabled=%s"), 
                        (int32)MovementComp->MovementMode,
                        MovementComp->IsMovingOnGround() ? TEXT("YES") : TEXT("NO"),
                        MovementComp->Velocity.Size(),
                        (int32)Character->GetCapsuleComponent()->GetCollisionEnabled(),
                        MovementComp->IsComponentTickEnabled() ? TEXT("YES") : TEXT("NO")));
            }
        }
    }
}

bool UObstacleClimbingComponent::TryClimbObstacle()
{
    if (bIsClimbing)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Obstacle Climbing: Already climbing!"));
        }
        return false;
    }

    FObstacleInfo Obstacle = CheckObstacleAhead();
    if (!Obstacle.bCanClimb)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("Obstacle Climbing: No valid obstacle found or obstacle too high!"));
        }
        OnClimbFailed.Broadcast();
        return false;
    }

    // Reset from previous climb
    CurrentClimbType = EObstacleClimbType::None;
    
    CurrentObstacle = Obstacle;
    CurrentClimbType = Obstacle.ClimbType;
    bIsClimbing = true;
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
            FString::Printf(TEXT("TryStartClimb: Set CurrentClimbType=%d (was reset first)"), (int32)CurrentClimbType));
    }

    if (GEngine)
    {
        FString ClimbTypeString;
        switch (CurrentClimbType)
        {
        case EObstacleClimbType::Vault:
            ClimbTypeString = TEXT("Vault (перепрыгивание)");
            break;
        case EObstacleClimbType::Climb:
            ClimbTypeString = TEXT("Climb (взбирание)");
            break;
        case EObstacleClimbType::ClimbOver:
            ClimbTypeString = TEXT("ClimbOver (взбирание и спуск)");
            break;
        default:
            ClimbTypeString = TEXT("Unknown");
            break;
        }
        
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
            FString::Printf(TEXT("Obstacle Climbing: Starting %s! Height: %.1f, Thickness: %.1f"), 
                *ClimbTypeString, Obstacle.ObstacleHeight, Obstacle.ObstacleThickness));
    }

    StartClimbAnimation(Obstacle);
    OnClimbStarted.Broadcast(CurrentClimbType);

    return true;
}

FObstacleInfo UObstacleClimbingComponent::CheckObstacleAhead()
{
    FObstacleInfo Result;

    if (!OwnerActor)
        return Result;

    ACharacter* Character = Cast<ACharacter>(OwnerActor);
    if (!Character)
        return Result;

    // Получаем направление движения
    FVector ForwardVector = Character->GetActorForwardVector();
    FVector CharacterLocation = Character->GetActorLocation();
    FVector TraceStartLocation = CharacterLocation + FVector(0, 0, DetectionHeight);
    FVector TraceEndLocation = TraceStartLocation + ForwardVector * DetectionDistance;

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Cyan, 
            FString::Printf(TEXT("Trace Setup: Start(%.1f, %.1f, %.1f) -> End(%.1f, %.1f, %.1f), Distance=%.1f"), 
                TraceStartLocation.X, TraceStartLocation.Y, TraceStartLocation.Z,
                TraceEndLocation.X, TraceEndLocation.Y, TraceEndLocation.Z,
                DetectionDistance));
    }

    // Трассировка для поиска препятствия
    FHitResult HitResult;
    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(OwnerActor);

    // Используем более широкую трассировку для лучшего обнаружения препятствий
    bool bHit = UKismetSystemLibrary::LineTraceSingle(
        GetWorld(),
        TraceStartLocation,
        TraceEndLocation,
        UEngineTypes::ConvertToTraceType(ECC_WorldStatic), // Используем WorldStatic вместо Visibility
        true,
        ActorsToIgnore,
        EDrawDebugTrace::ForOneFrame,
        HitResult,
        true
    );

    if (GEngine)
    {
        if (bHit)
        {
            GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Cyan, 
                FString::Printf(TEXT("Obstacle Detection: Hit actor %s at distance %.1f"), 
                    HitResult.GetActor() ? *HitResult.GetActor()->GetName() : TEXT("NULL"), 
                    FVector::Dist(TraceStartLocation, HitResult.Location)));
        }
        else
        {
            GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Silver, TEXT("Obstacle Detection: No obstacle found"));
        }
    }

    if (!bHit || !IsValidObstacle(HitResult))
    {
        if (GEngine && bHit)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, 
                FString::Printf(TEXT("Obstacle Detection: Invalid obstacle - %s"), 
                    HitResult.GetActor() ? *HitResult.GetActor()->GetName() : TEXT("NULL")));
        }
        
        // Попробуем несколько вариантов трассировки
        if (!bHit)
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("Trying alternative traces..."));
            }
            
            // 1. Короткая трассировка
            FVector ShortTraceEnd = TraceStartLocation + ForwardVector * (DetectionDistance * 0.5f);
            bool bShortHit = UKismetSystemLibrary::LineTraceSingle(
                GetWorld(),
                TraceStartLocation,
                ShortTraceEnd,
                UEngineTypes::ConvertToTraceType(ECC_WorldStatic),
                true,
                ActorsToIgnore,
                EDrawDebugTrace::ForOneFrame,
                HitResult,
                true
            );
            
            if (bShortHit && IsValidObstacle(HitResult))
            {
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, 
                        FString::Printf(TEXT("Short trace found valid obstacle: %s"), 
                            *HitResult.GetActor()->GetName()));
                }
                bHit = true;
            }
            
            // 2. Трассировка с земли
            if (!bHit)
            {
                FVector GroundTraceStart = CharacterLocation;
                FVector GroundTraceEnd = GroundTraceStart + ForwardVector * DetectionDistance;
                
                bool bGroundHit = UKismetSystemLibrary::LineTraceSingle(
                    GetWorld(),
                    GroundTraceStart,
                    GroundTraceEnd,
                    UEngineTypes::ConvertToTraceType(ECC_WorldStatic),
                    true,
                    ActorsToIgnore,
                    EDrawDebugTrace::ForOneFrame,
                    HitResult,
                    true
                );
                
                if (bGroundHit && IsValidObstacle(HitResult))
                {
                    if (GEngine)
                    {
                        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, 
                            FString::Printf(TEXT("Ground trace found valid obstacle: %s"), 
                                *HitResult.GetActor()->GetName()));
                    }
                    bHit = true;
                }
            }
        }
        
        if (!bHit)
        {
            return Result;
        }
    }

    Result = AnalyzeObstacle(HitResult);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, 
            FString::Printf(TEXT("Obstacle Analysis: Height=%.1f, Thickness=%.1f, CanClimb=%s"), 
                Result.ObstacleHeight, Result.ObstacleThickness, 
                Result.bCanClimb ? TEXT("YES") : TEXT("NO")));
    }
    
    return Result;
}

bool UObstacleClimbingComponent::IsValidObstacle(const FHitResult& HitResult) const
{
    if (!HitResult.GetActor())
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("IsValidObstacle: No actor in hit result"));
        }
        return false;
    }

    // Проверяем, что это не персонаж
    if (Cast<ACharacter>(HitResult.GetActor()))
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, 
                FString::Printf(TEXT("IsValidObstacle: Hit character %s"), *HitResult.GetActor()->GetName()));
        }
        return false;
    }

    // Проверяем мобильность компонента (разрешаем статичные, стационарные и кинематические объекты)
    USceneComponent* RootComp = HitResult.GetActor()->GetRootComponent();
    if (RootComp)
    {
        EComponentMobility::Type Mobility = RootComp->Mobility;
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, 
                FString::Printf(TEXT("IsValidObstacle: Actor %s has mobility %d"), 
                    *HitResult.GetActor()->GetName(), (int32)Mobility));
        }
        
        // Принимаем все типы мобильности кроме Movable (которые могут двигаться)
        if (Mobility == EComponentMobility::Movable)
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, 
                    FString::Printf(TEXT("IsValidObstacle: Actor %s is movable (rejected)"), 
                        *HitResult.GetActor()->GetName()));
            }
            return false;
        }
        
        // Подтверждаем, что объект принят
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, 
                FString::Printf(TEXT("IsValidObstacle: Actor %s mobility %d is acceptable"), 
                    *HitResult.GetActor()->GetName(), (int32)Mobility));
        }
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, 
            FString::Printf(TEXT("IsValidObstacle: Valid obstacle %s found!"), *HitResult.GetActor()->GetName()));
    }

    return true;
}

FObstacleInfo UObstacleClimbingComponent::AnalyzeObstacle(const FHitResult& HitResult) const
{
    FObstacleInfo Result;
    Result.ObstacleLocation = HitResult.Location;

    ACharacter* Character = Cast<ACharacter>(OwnerActor);
    if (!Character)
        return Result;

    // Получаем размеры препятствия
    FBox ObstacleBox = HitResult.GetActor()->GetComponentsBoundingBox();
    FVector ObstacleBounds = ObstacleBox.GetSize();
    FVector ObstacleCenter = ObstacleBox.GetCenter();
    
    Result.ObstacleSize = ObstacleBounds;
    Result.ObstacleHeight = ObstacleBounds.Z;
    Result.ObstacleThickness = FMath::Min(ObstacleBounds.X, ObstacleBounds.Y);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, 
            FString::Printf(TEXT("Obstacle Bounds: Size(%.1f, %.1f, %.1f), Center(%.1f, %.1f, %.1f)"), 
                ObstacleBounds.X, ObstacleBounds.Y, ObstacleBounds.Z,
                ObstacleCenter.X, ObstacleCenter.Y, ObstacleCenter.Z));
    }

    // Получаем рост персонажа
    float CharacterHeight = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2.0f;
    float CharacterMaxClimbHeight = CharacterHeight + 50.0f; // Рост + 50см

    // Проверяем, можно ли преодолеть препятствие
    if (Result.ObstacleHeight > CharacterMaxClimbHeight)
    {
        Result.bCanClimb = false;
        return Result;
    }

    Result.bCanClimb = true;
    Result.ClimbType = DetermineClimbType(Result);
    return Result;
}

EObstacleClimbType UObstacleClimbingComponent::DetermineClimbType(const FObstacleInfo& Obstacle) const
{
    ACharacter* Character = Cast<ACharacter>(OwnerActor);
    if (!Character)
        return EObstacleClimbType::None;

    float CharacterHeight = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2.0f;
    float CharacterHeightThreshold = CharacterHeight * HeightThreshold; // 60% роста
    float CharacterMaxClimbHeight = CharacterHeight + 50.0f; // Рост + 50см

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::White, 
            FString::Printf(TEXT("DetermineClimbType: Height=%.1f, Thickness=%.1f, CharHeight=%.1f, Threshold=%.1f, MaxHeight=%.1f"),
                Obstacle.ObstacleHeight, Obstacle.ObstacleThickness, CharacterHeight, CharacterHeightThreshold, CharacterMaxClimbHeight));
    }

    // Логика определения типа преодоления согласно требованиям
    if (Obstacle.ObstacleThickness <= ThicknessThreshold)
    {
        // Тонкое препятствие
        if (Obstacle.ObstacleHeight <= CharacterHeightThreshold)
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, TEXT("DetermineClimbType: Returning VAULT"));
            }
            return EObstacleClimbType::Vault; // Перепрыгивание
        }
        else if (Obstacle.ObstacleHeight <= CharacterMaxClimbHeight)
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, TEXT("DetermineClimbType: Returning CLIMBOVER"));
            }
            return EObstacleClimbType::ClimbOver; // Взбирание и спуск
        }
        else
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("DetermineClimbType: Returning NONE (too high)"));
            }
            return EObstacleClimbType::None; // Слишком высокое
        }
    }
    else
    {
        // Толстое препятствие
        if (Obstacle.ObstacleHeight <= CharacterMaxClimbHeight)
        {
            return EObstacleClimbType::Climb; // Взбирание на препятствие
        }
        else
        {
            return EObstacleClimbType::None; // Слишком высокое
        }
    }

    return EObstacleClimbType::None;
}

void UObstacleClimbingComponent::StartClimbAnimation(const FObstacleInfo& Obstacle)
{
    ACharacter* Character = Cast<ACharacter>(OwnerActor);
    if (!Character)
        return;

    // ЗАЩИТА: Если уже лезем - НЕ запускать повторно!
    if (bIsClimbing)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, 
                TEXT("WARNING: Already climbing! Ignoring duplicate StartClimbAnimation call"));
        }
        return;
    }

    StartLocation = Character->GetActorLocation();
    ClimbProgress = 0.0f;

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, 
            FString::Printf(TEXT("Climb Animation: Starting from (%.1f, %.1f, %.1f)"), 
                StartLocation.X, StartLocation.Y, StartLocation.Z));
    }

    // Отключаем движение персонажа
    Character->GetCharacterMovement()->SetMovementMode(MOVE_None);
    Character->GetCharacterMovement()->StopMovementImmediately();
    
    // Отключаем коллизии капсулы для свободного перемещения
    UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
    Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, 
            FString::Printf(TEXT("COLLISION DISABLED! Enabled: %d"), 
                (int32)Capsule->GetCollisionEnabled()));
    }
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, 
            FString::Printf(TEXT("Movement disabled. Current mode: %d, Collision disabled"), 
                (int32)Character->GetCharacterMovement()->MovementMode));
    }

    // Определяем целевую позицию в зависимости от типа преодоления
    switch (CurrentClimbType)
    {
    case EObstacleClimbType::Vault:
        // Перепрыгивание - перемещаемся ДАЛЕКО за препятствие на уровне земли
        // УВЕЛИЧЕНО с 100 до 200 см для гарантированного избежания коллизий
        TargetLocation = Obstacle.ObstacleLocation + Character->GetActorForwardVector() * (Obstacle.ObstacleThickness + 200.0f);
        TargetLocation.Z = StartLocation.Z; // Оставляем на той же высоте, что и начали
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, 
                FString::Printf(TEXT("Vault: Moving to (%.1f, %.1f, %.1f), Distance: %.1f cm"), 
                    TargetLocation.X, TargetLocation.Y, TargetLocation.Z,
                    Obstacle.ObstacleThickness + 200.0f));
        }
        break;

    case EObstacleClimbType::Climb:
        // Взбирание - остаемся на препятствии
        TargetLocation = Obstacle.ObstacleLocation + FVector(0, 0, Obstacle.ObstacleHeight + 50.0f);
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, 
                FString::Printf(TEXT("Climb: Moving to top (%.1f, %.1f, %.1f)"), 
                    TargetLocation.X, TargetLocation.Y, TargetLocation.Z));
        }
        break;

    case EObstacleClimbType::ClimbOver:
        // Взбирание и спуск - сначала вверх, потом вниз
        TargetLocation = Obstacle.ObstacleLocation + FVector(0, 0, Obstacle.ObstacleHeight + 50.0f);
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, 
                FString::Printf(TEXT("ClimbOver: Moving to top (%.1f, %.1f, %.1f)"), 
                    TargetLocation.X, TargetLocation.Y, TargetLocation.Z));
        }
        break;

    default:
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Climb Animation: Unknown climb type!"));
        }
        CancelClimb();
        return;
    }
}

void UObstacleClimbingComponent::UpdateClimbAnimation(float DeltaTime)
{
    ACharacter* Character = Cast<ACharacter>(OwnerActor);
    if (!Character)
    {
        CancelClimb();
        return;
    }

    switch (CurrentClimbType)
    {
    case EObstacleClimbType::Vault:
        PerformVaultAnimation(DeltaTime);
        break;

    case EObstacleClimbType::Climb:
        PerformClimbAnimation(DeltaTime);
        break;

    case EObstacleClimbType::ClimbOver:
        PerformClimbOverAnimation(DeltaTime);
        break;

    default:
        CancelClimb();
        break;
    }
}

void UObstacleClimbingComponent::PerformVaultAnimation(float DeltaTime)
{
    ACharacter* Character = Cast<ACharacter>(OwnerActor);
    if (!Character)
        return;

    float Speed = VaultSpeed;
    ClimbProgress += Speed * DeltaTime;

    if (ClimbProgress >= 1.0f)
    {
        ClimbProgress = 1.0f;
        CompleteClimb();
        return;
    }

    // Очень плавное перемещение с красивой дугой
    float SmoothProgress = FMath::SmoothStep(0.0f, 1.0f, ClimbProgress);
    SmoothProgress = FMath::SmoothStep(0.0f, 1.0f, SmoothProgress); // Двойное сглаживание
    FVector CurrentLocation = FMath::Lerp(StartLocation, TargetLocation, SmoothProgress);
    
    // Более плавная дуга с использованием синуса
    float ArcProgress = FMath::Sin(SmoothProgress * PI);
    float ArcHeight = 60.0f * ArcProgress; // Увеличена высота дуги для красоты
    CurrentLocation.Z += ArcHeight;

    // Во время анимации НЕ используем TeleportPhysics!
    Character->SetActorLocation(CurrentLocation, false, nullptr, ETeleportType::None);
}

void UObstacleClimbingComponent::PerformClimbAnimation(float DeltaTime)
{
    ACharacter* Character = Cast<ACharacter>(OwnerActor);
    if (!Character)
        return;

    // Плавное замедление в конце анимации
    float Speed = ClimbSpeed;
    if (ClimbProgress > 0.8f)
    {
        // Замедляем в последних 20% анимации
        float SlowdownFactor = 1.0f - ((ClimbProgress - 0.8f) / 0.2f) * 0.7f; // Замедляем до 30% скорости
        Speed *= SlowdownFactor;
    }
    ClimbProgress += Speed * DeltaTime;

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, 
            FString::Printf(TEXT("Climb Progress: %.2f/1.0"), ClimbProgress));
    }

    if (ClimbProgress >= 1.0f)
    {
        ClimbProgress = 1.0f;
        CompleteClimb();
        return;
    }

    // Очень плавное перемещение с улучшенной интерполяцией
    float SmoothProgress = FMath::SmoothStep(0.0f, 1.0f, ClimbProgress);
    // Дополнительное сглаживание для еще большей плавности
    SmoothProgress = FMath::SmoothStep(0.0f, 1.0f, SmoothProgress);
    FVector CurrentLocation = FMath::Lerp(StartLocation, TargetLocation, SmoothProgress);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Cyan, 
            FString::Printf(TEXT("Moving to: (%.1f, %.1f, %.1f) Progress: %.2f"), 
                CurrentLocation.X, CurrentLocation.Y, CurrentLocation.Z, SmoothProgress));
    }
    
    // Во время анимации НЕ используем TeleportPhysics!
    Character->SetActorLocation(CurrentLocation, false, nullptr, ETeleportType::None);
}

void UObstacleClimbingComponent::PerformClimbOverAnimation(float DeltaTime)
{
    ACharacter* Character = Cast<ACharacter>(OwnerActor);
    if (!Character)
        return;

    float Speed = ClimbSpeed;
    ClimbProgress += Speed * DeltaTime;

    if (ClimbProgress >= 1.0f)
    {
        ClimbProgress = 1.0f;
        CompleteClimb();
        return;
    }

    // Очень плавная двухэтапная анимация: вверх, потом вниз
    FVector CurrentLocation;
    float SmoothProgress = FMath::SmoothStep(0.0f, 1.0f, ClimbProgress);
    SmoothProgress = FMath::SmoothStep(0.0f, 1.0f, SmoothProgress); // Двойное сглаживание
    
    if (ClimbProgress <= 0.5f)
    {
        // Фаза 1: Плавное взбирание вверх
        float PhaseProgress = ClimbProgress * 2.0f;
        float SmoothPhaseProgress = FMath::SmoothStep(0.0f, 1.0f, PhaseProgress);
        CurrentLocation = FMath::Lerp(StartLocation, TargetLocation, SmoothPhaseProgress);
    }
    else
    {
        // Фаза 2: Плавный спуск вниз
        float PhaseProgress = (ClimbProgress - 0.5f) * 2.0f;
        float SmoothPhaseProgress = FMath::SmoothStep(0.0f, 1.0f, PhaseProgress);
        FVector TopLocation = TargetLocation;
        // УВЕЛИЧЕНО с 100 до 200 см для избежания застревания
        FVector FinalLocation = CurrentObstacle.ObstacleLocation + Character->GetActorForwardVector() * (CurrentObstacle.ObstacleThickness + 200.0f);
        FinalLocation.Z = StartLocation.Z;
        CurrentLocation = FMath::Lerp(TopLocation, FinalLocation, SmoothPhaseProgress);
    }

    // Во время анимации НЕ используем TeleportPhysics!
    Character->SetActorLocation(CurrentLocation, false, nullptr, ETeleportType::None);
}

void UObstacleClimbingComponent::CompleteClimb()
{
    ACharacter* Character = Cast<ACharacter>(OwnerActor);
    if (Character)
    {
        // КРИТИЧЕСКИ ВАЖНО: Сначала перемещаем персонажа ДАЛЕКО от препятствия
        // Используем БЕЗОПАСНУЮ позицию - еще дальше вперед!
        FVector SafeLocation = Character->GetActorLocation();
        
        // Дополнительно сдвигаем вперед на 50 см для гарантии
        SafeLocation += Character->GetActorForwardVector() * 50.0f;
        
        // ТАКЖЕ поднимаем на 10 см вверх, чтобы точно не застрять
        SafeLocation.Z += 10.0f;
        
        // Перемещаем в безопасную зону (коллизия отключена)
        Character->SetActorLocation(SafeLocation, false, nullptr, ETeleportType::None);
        
        // КРИТИЧЕСКИ ВАЖНО: Принудительно синхронизируем Mesh с капсулой!
        if (USkeletalMeshComponent* Mesh = Character->GetMesh())
        {
            Mesh->SetRelativeLocation(FVector::ZeroVector);
            Mesh->SetRelativeRotation(FRotator::ZeroRotator);
            Mesh->UpdateComponentToWorld();
            
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
                    TEXT("🔧 Mesh synchronized with capsule!"));
            }
        }
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, 
                FString::Printf(TEXT("Safe position: (%.1f, %.1f, %.1f)"), 
                    SafeLocation.X, SafeLocation.Y, SafeLocation.Z));
        }
        
        // ТЕПЕРЬ проверяем землю из БЕЗОПАСНОЙ позиции
        FHitResult HitResult;
        FVector StartTrace = SafeLocation;
        FVector EndTrace = StartTrace - FVector(0, 0, 300.0f); // Увеличено до 300 см
        
        FVector FinalLocation = SafeLocation;
        if (GetWorld()->LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, ECC_Visibility))
        {
            // Вычисляем позицию на земле
            FinalLocation = HitResult.Location + FVector(0, 0, Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
        }
        
        // Перемещаем персонажа на землю
        Character->SetActorLocation(FinalLocation, false, nullptr, ETeleportType::None);
        
        // Снова синхронизируем Mesh после финального перемещения
        if (USkeletalMeshComponent* Mesh = Character->GetMesh())
        {
            Mesh->SetRelativeLocation(FVector::ZeroVector);
            Mesh->SetRelativeRotation(FRotator::ZeroRotator);
            Mesh->UpdateComponentToWorld();
        }
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Magenta, 
                FString::Printf(TEXT("Moved to ground: (%.1f, %.1f, %.1f)"), 
                    FinalLocation.X, FinalLocation.Y, FinalLocation.Z));
        }
        
        // НЕ включаем коллизии сразу! Запускаем таймер на 0.5 секунды
        bWaitingForCollisionRestore = true;
        CollisionRestoreTimer = 0.0f;
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
                TEXT("⏱️ Collision restore delayed for 0.5 seconds..."));
        }
        
        // Плавное восстановление движения
        Character->GetCharacterMovement()->StopMovementImmediately();
        Character->GetCharacterMovement()->Velocity = FVector::ZeroVector;
        
        // Восстанавливаем режим ходьбы - АГРЕССИВНЫЙ подход
        UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
        
        // КРИТИЧЕСКИ ВАЖНО: Сначала принудительно обнуляем состояние
        MovementComp->StopMovementImmediately();
        MovementComp->Velocity = FVector::ZeroVector;
        MovementComp->bForceMaxAccel = false;
        
        // Включаем физику и обновление
        MovementComp->SetComponentTickEnabled(true);
        MovementComp->SetUpdatedComponent(Character->GetRootComponent());
        
        // ТЕПЕРЬ устанавливаем режим ходьбы - используем ВСЕ методы подряд!
        MovementComp->SetMovementMode(MOVE_Walking);
        MovementComp->SetDefaultMovementMode();
        
        // Принудительно через свойство
        MovementComp->MovementMode = MOVE_Walking;
        
        // Принудительно обновляем компонент
        MovementComp->UpdateComponentVelocity();
        
        // КРИТИЧЕСКИ ВАЖНО: Принудительно ставим персонажа на землю
        FFindFloorResult FloorResult;
        MovementComp->FindFloor(Character->GetActorLocation(), FloorResult, false);
        
        if (FloorResult.IsWalkableFloor())
        {
            // Устанавливаем базу (пол) для персонажа
            MovementComp->SetBase(FloorResult.HitResult.GetComponent(), FloorResult.HitResult.BoneName);
            
            // Принудительно обновляем состояние пола
            MovementComp->CurrentFloor = FloorResult;
            MovementComp->bForceNextFloorCheck = false;
            
            // ТЕПЕРЬ устанавливаем режим ходьбы
            MovementComp->SetMovementMode(MOVE_Walking);
            
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Cyan, 
                    TEXT("🎯 FLOOR SET! Character grounded on walkable surface!"));
            }
        }
        else
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, 
                    TEXT("⚠️ WARNING: No walkable floor found!"));
            }
        }
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, 
                FString::Printf(TEXT("✅ MOVEMENT RESTORED! Mode: %d (3=Walking), Ground: %s, Velocity: %.1f"), 
                    (int32)Character->GetCharacterMovement()->MovementMode,
                    Character->GetCharacterMovement()->IsMovingOnGround() ? TEXT("YES") : TEXT("NO"),
                    Character->GetCharacterMovement()->Velocity.Size()));
        }
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, 
                FString::Printf(TEXT("🎯 CLIMB COMPLETE! Position: (%.1f, %.1f, %.1f)"), 
                    Character->GetActorLocation().X, Character->GetActorLocation().Y, Character->GetActorLocation().Z));
        }
    }

    bIsClimbing = false;
    // Don't reset CurrentClimbType immediately - let animation finish
    // It will be reset when starting next climb
    OnClimbCompleted.Broadcast();
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
            FString::Printf(TEXT("CompleteClimb: bIsClimbing=false, CurrentClimbType still=%d"), (int32)CurrentClimbType));
    }
}

void UObstacleClimbingComponent::CancelClimb()
{
    ACharacter* Character = Cast<ACharacter>(OwnerActor);
    if (Character)
    {
        // Восстанавливаем коллизии капсулы
        Character->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        
        // Восстанавливаем движение
        Character->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Obstacle Climbing: Cancelled!"));
        }
    }

    bIsClimbing = false;
    CurrentClimbType = EObstacleClimbType::None;
    OnClimbFailed.Broadcast();
}
