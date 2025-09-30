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

    if (bIsClimbing)
    {
        UpdateClimbAnimation(DeltaTime);
        
        // Принудительно обновляем позицию персонажа
        ACharacter* Character = Cast<ACharacter>(OwnerActor);
        if (Character)
        {
            // Убеждаемся, что движение отключено
            if (Character->GetCharacterMovement()->MovementMode != MOVE_None)
            {
                Character->GetCharacterMovement()->SetMovementMode(MOVE_None);
            }
        }
    }
    else
    {
        // Если не преодолеваем, убеждаемся что движение включено
        ACharacter* Character = Cast<ACharacter>(OwnerActor);
        if (Character && Character->GetCharacterMovement()->MovementMode == MOVE_None)
        {
            Character->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, TEXT("Movement force restored"));
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

    CurrentObstacle = Obstacle;
    CurrentClimbType = Obstacle.ClimbType;
    bIsClimbing = true;

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

    // Логика определения типа преодоления согласно требованиям
    if (Obstacle.ObstacleThickness <= ThicknessThreshold)
    {
        // Тонкое препятствие
        if (Obstacle.ObstacleHeight <= CharacterHeightThreshold)
        {
            return EObstacleClimbType::Vault; // Перепрыгивание
        }
        else if (Obstacle.ObstacleHeight <= CharacterMaxClimbHeight)
        {
            return EObstacleClimbType::ClimbOver; // Взбирание и спуск
        }
    }
    else
    {
        // Толстое препятствие
        if (Obstacle.ObstacleHeight <= CharacterHeightThreshold)
        {
            return EObstacleClimbType::Climb; // Взбирание на препятствие
        }
        else if (Obstacle.ObstacleHeight <= CharacterMaxClimbHeight)
        {
            return EObstacleClimbType::Climb; // Взбирание на препятствие
        }
    }

    return EObstacleClimbType::None;
}

void UObstacleClimbingComponent::StartClimbAnimation(const FObstacleInfo& Obstacle)
{
    ACharacter* Character = Cast<ACharacter>(OwnerActor);
    if (!Character)
        return;

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
    Character->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    
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
        // Перепрыгивание - перемещаемся за препятствие
        TargetLocation = Obstacle.ObstacleLocation + Character->GetActorForwardVector() * (Obstacle.ObstacleThickness + 100.0f);
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, 
                FString::Printf(TEXT("Vault: Moving to (%.1f, %.1f, %.1f)"), 
                    TargetLocation.X, TargetLocation.Y, TargetLocation.Z));
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

    Character->SetActorLocation(CurrentLocation, false, nullptr, ETeleportType::TeleportPhysics);
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
    
    // Используем принудительное перемещение с отключением коллизий
    Character->SetActorLocation(CurrentLocation, false, nullptr, ETeleportType::TeleportPhysics);
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
        FVector FinalLocation = CurrentObstacle.ObstacleLocation + Character->GetActorForwardVector() * (CurrentObstacle.ObstacleThickness + 100.0f);
        FinalLocation.Z = StartLocation.Z;
        CurrentLocation = FMath::Lerp(TopLocation, FinalLocation, SmoothPhaseProgress);
    }

    Character->SetActorLocation(CurrentLocation, false, nullptr, ETeleportType::TeleportPhysics);
}

void UObstacleClimbingComponent::CompleteClimb()
{
    ACharacter* Character = Cast<ACharacter>(OwnerActor);
    if (Character)
    {
        // Принудительно устанавливаем финальную позицию
        Character->SetActorLocation(TargetLocation, false, nullptr, ETeleportType::TeleportPhysics);
        
        // Восстанавливаем коллизии капсулы
        Character->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        
        // Восстанавливаем движение
        Character->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
        Character->GetCharacterMovement()->SetMovementMode(MOVE_Walking); // Двойная проверка
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                FString::Printf(TEXT("Movement restored. Mode: %d"), 
                    (int32)Character->GetCharacterMovement()->MovementMode));
        }
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                FString::Printf(TEXT("Obstacle Climbing: Completed! Final position (%.1f, %.1f, %.1f)"), 
                    Character->GetActorLocation().X, Character->GetActorLocation().Y, Character->GetActorLocation().Z));
        }
    }

    bIsClimbing = false;
    CurrentClimbType = EObstacleClimbType::None;
    OnClimbCompleted.Broadcast();
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
