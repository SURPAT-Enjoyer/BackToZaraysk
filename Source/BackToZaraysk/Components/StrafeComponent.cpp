#include "StrafeComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Curves/CurveFloat.h"

UStrafeComponent::UStrafeComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UStrafeComponent::BeginPlay()
{
    Super::BeginPlay();
    OwnerActor = GetOwner();
}

void UStrafeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsStrafing)
    {
        UpdateStrafeAnimation(DeltaTime);
        
        // ИСПРАВЛЕНО: Убираем принудительную остановку движения - теперь движение управляется в UpdateStrafeAnimation
        // Система движения теперь полностью контролируется через Velocity в UpdateStrafeAnimation
    }

    // Обработка стрейфа по нажатию C
}

bool UStrafeComponent::TryStrafe(EStrafeType StrafeDirection)
{
    if (bIsStrafing)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Strafe: Already strafing!"));
        }
        return false;
    }

    if (!CanStrafe())
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("Strafe: Cannot strafe now!"));
        }
        OnStrafeFailed.Broadcast();
        return false;
    }

    FStrafeInfo StrafeInfo = CreateStrafeInfo(StrafeDirection);
    if (!StrafeInfo.bCanStrafe)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("Strafe: Invalid strafe direction!"));
        }
        OnStrafeFailed.Broadcast();
        return false;
    }

    CurrentStrafeInfo = StrafeInfo;
    CurrentStrafeType = StrafeDirection;
    bIsStrafing = true;
    LastStrafeTime = GetWorld()->GetTimeSeconds();

    if (GEngine)
    {
        FString StrafeTypeString;
        switch (CurrentStrafeType)
        {
        case EStrafeType::Left:
            StrafeTypeString = TEXT("Left");
            break;
        case EStrafeType::Right:
            StrafeTypeString = TEXT("Right");
            break;
        case EStrafeType::Forward:
            StrafeTypeString = TEXT("Forward");
            break;
        case EStrafeType::Backward:
            StrafeTypeString = TEXT("Backward");
            break;
        default:
            StrafeTypeString = TEXT("Unknown");
            break;
        }
        
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
            FString::Printf(TEXT("Strafe: Starting %s strafe! Distance: %.1f"), 
                *StrafeTypeString, StrafeInfo.StrafeDistance));
    }

    StartStrafeAnimation(StrafeInfo);
    OnStrafeStarted.Broadcast(CurrentStrafeType);

    return true;
}

bool UStrafeComponent::CanStrafe() const
{
    ACharacter* Character = Cast<ACharacter>(OwnerActor);
    if (!Character)
        return false;

    return !bIsStrafing && 
           !Character->GetCharacterMovement()->IsFalling() && 
           !Character->GetCharacterMovement()->IsCrouching() &&
           (GetWorld()->GetTimeSeconds() - LastStrafeTime) >= StrafeCooldown;
}

void UStrafeComponent::HandleAInput(bool bPressed)
{
    bIsADown = bPressed;
    CheckStrafeCombo();
}

void UStrafeComponent::HandleDInput(bool bPressed)
{
    bIsDDown = bPressed;
    CheckStrafeCombo();
}

void UStrafeComponent::HandleSpaceInput(bool bPressed)
{
    bIsSpaceDown = bPressed;
    
    if (bPressed)
    {
        CheckStrafeCombo();
    }
}

void UStrafeComponent::CheckStrafeCombo()
{
    if (!CanStrafe()) return;
    
    // Проверяем комбинацию A + Space для стрейфа влево
    if (bIsADown && bIsSpaceDown)
    {
        TryStrafe(EStrafeType::Left);
        return;
    }
    
    // Проверяем комбинацию D + Space для стрейфа вправо
    if (bIsDDown && bIsSpaceDown)
    {
        TryStrafe(EStrafeType::Right);
        return;
    }
}

FStrafeInfo UStrafeComponent::CreateStrafeInfo(EStrafeType StrafeType) const
{
    FStrafeInfo Result;
    Result.StrafeType = StrafeType;
    Result.StrafeDistance = StrafeDistance;
    
    ACharacter* Character = Cast<ACharacter>(OwnerActor);
    if (!Character)
    {
        Result.bCanStrafe = false;
        return Result;
    }

    Result.StartLocation = Character->GetActorLocation();
    Result.bCanStrafe = true;

    // Определяем направление стрейфа
    FVector StrafeDirection = FVector::ZeroVector;
    FRotator CharacterRotation = Character->GetControlRotation();
    
    switch (StrafeType)
    {
    case EStrafeType::Left:
        StrafeDirection = CharacterRotation.RotateVector(FVector::LeftVector);
        break;
    case EStrafeType::Right:
        StrafeDirection = CharacterRotation.RotateVector(FVector::RightVector);
        break;
    case EStrafeType::Forward:
        StrafeDirection = CharacterRotation.RotateVector(FVector::ForwardVector);
        break;
    case EStrafeType::Backward:
        StrafeDirection = CharacterRotation.RotateVector(FVector::BackwardVector);
        break;
    default:
        Result.bCanStrafe = false;
        return Result;
    }

    Result.TargetLocation = Result.StartLocation + (StrafeDirection * Result.StrafeDistance);
    
    return Result;
}

void UStrafeComponent::StartStrafeAnimation(const FStrafeInfo& StrafeInfo)
{
    ACharacter* Character = Cast<ACharacter>(OwnerActor);
    if (!Character)
        return;

    StartLocation = StrafeInfo.StartLocation;
    TargetLocation = StrafeInfo.TargetLocation;
    StrafeProgress = 0.0f;

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, 
            FString::Printf(TEXT("Strafe: Starting smooth movement from (%.1f, %.1f, %.1f) to (%.1f, %.1f, %.1f)"), 
                StartLocation.X, StartLocation.Y, StartLocation.Z,
                TargetLocation.X, TargetLocation.Y, TargetLocation.Z));
    }

    // ИСПРАВЛЕНО: Используем естественное движение персонажа
    UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
    if (MovementComp)
    {
        // Сохраняем текущий режим движения
        PreviousMovementMode = MovementComp->MovementMode;
        // Останавливаем текущее движение
        MovementComp->StopMovementImmediately();
        MovementComp->Velocity = FVector::ZeroVector;
    }
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, 
            FString::Printf(TEXT("Strafe: Using natural character movement for smooth sliding")));
    }
}

void UStrafeComponent::UpdateStrafeAnimation(float DeltaTime)
{
    ACharacter* Character = Cast<ACharacter>(OwnerActor);
    if (!Character)
    {
        CancelStrafe();
        return;
    }

    UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
    if (!MovementComp)
    {
        CancelStrafe();
        return;
    }

    // ИСПРАВЛЕНО: Используем естественное движение персонажа для плавного скольжения
    StrafeProgress += DeltaTime;
    
    // Вычисляем общую длительность стрейфа
    float TotalDistance = FVector::Dist(StartLocation, TargetLocation);
    float CurrentStrafeDuration = TotalDistance / StrafeSpeed;
    
    // Нормализуем прогресс (0.0 - 1.0)
    float NormalizedProgress = FMath::Clamp(StrafeProgress / CurrentStrafeDuration, 0.0f, 1.0f);
    
    // Если стрейф завершен
    if (NormalizedProgress >= 1.0f)
    {
        CompleteStrafe();
        return;
    }
    
    // ИСПРАВЛЕНО: Вычисляем направление движения
    FVector StrafeDirection = (TargetLocation - StartLocation).GetSafeNormal();
    
    // ИСПРАВЛЕНО: Используем прямое изменение скорости для плавного скольжения
    FVector TargetVelocity = StrafeDirection * StrafeSpeed;
    MovementComp->Velocity = TargetVelocity;
    
    // Debug информация
    static float DebugTimer = 0.0f;
    DebugTimer += DeltaTime;
    if (DebugTimer >= 0.2f)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Cyan, 
                FString::Printf(TEXT("Strafe: Progress=%.2f, Direction=(%.2f, %.2f, %.2f), Duration=%.2f"), 
                    NormalizedProgress, StrafeDirection.X, StrafeDirection.Y, StrafeDirection.Z, CurrentStrafeDuration));
        }
        DebugTimer = 0.0f;
    }
}

void UStrafeComponent::CompleteStrafe()
{
    ACharacter* Character = Cast<ACharacter>(OwnerActor);
    if (Character)
    {
        UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
        if (MovementComp)
        {
            // ИСПРАВЛЕНО: Останавливаем движение после естественного скольжения
            MovementComp->StopMovementImmediately();
            MovementComp->Velocity = FVector::ZeroVector;
        }
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                FString::Printf(TEXT("Strafe completed! Final position (%.1f, %.1f, %.1f) - Natural sliding finished"), 
                    Character->GetActorLocation().X, Character->GetActorLocation().Y, Character->GetActorLocation().Z));
        }
    }

    bIsStrafing = false;
    CurrentStrafeType = EStrafeType::None;
    OnStrafeCompleted.Broadcast();
}

void UStrafeComponent::CancelStrafe()
{
    ACharacter* Character = Cast<ACharacter>(OwnerActor);
    if (Character)
    {
        UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
        if (MovementComp)
        {
            // ИСПРАВЛЕНО: Останавливаем движение при отмене
            MovementComp->StopMovementImmediately();
            MovementComp->Velocity = FVector::ZeroVector;
        }
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Strafe: Cancelled! Natural movement stopped"));
        }
    }

    bIsStrafing = false;
    CurrentStrafeType = EStrafeType::None;
    OnStrafeFailed.Broadcast();
}
