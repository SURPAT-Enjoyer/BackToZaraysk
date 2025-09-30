#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/TimelineComponent.h"
#include "ObstacleClimbingComponent.generated.h"

UENUM(BlueprintType)
enum class EObstacleClimbType : uint8
{
    None,           // Нельзя преодолеть
    Vault,          // Перепрыгивание через препятствие
    Climb,          // Взбирание на препятствие
    ClimbOver       // Взбирание и спуск за препятствие
};

USTRUCT(BlueprintType)
struct FObstacleInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FVector ObstacleLocation;

    UPROPERTY(BlueprintReadOnly)
    FVector ObstacleSize;

    UPROPERTY(BlueprintReadOnly)
    float ObstacleHeight;

    UPROPERTY(BlueprintReadOnly)
    float ObstacleThickness;

    UPROPERTY(BlueprintReadOnly)
    EObstacleClimbType ClimbType;

    UPROPERTY(BlueprintReadOnly)
    bool bCanClimb;

    FObstacleInfo()
    {
        ObstacleLocation = FVector::ZeroVector;
        ObstacleSize = FVector::ZeroVector;
        ObstacleHeight = 0.0f;
        ObstacleThickness = 0.0f;
        ClimbType = EObstacleClimbType::None;
        bCanClimb = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BACKTOZARAYSK_API UObstacleClimbingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UObstacleClimbingComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Попытка преодоления препятствия
    UFUNCTION(BlueprintCallable, Category = "Obstacle Climbing")
    bool TryClimbObstacle();

    // Проверка препятствия перед персонажем
    UFUNCTION(BlueprintCallable, Category = "Obstacle Climbing")
    FObstacleInfo CheckObstacleAhead();

    // Настройки
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle Climbing|Settings")
    float DetectionDistance = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle Climbing|Settings")
    float DetectionHeight = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle Climbing|Settings")
    float MaxClimbHeight = 200.0f; // Максимальная высота препятствия для преодоления

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle Climbing|Settings")
    float ThicknessThreshold = 30.0f; // Порог толщины для определения типа преодоления

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle Climbing|Settings")
    float HeightThreshold = 0.6f; // 60% роста игрока

    // Скорости анимации
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle Climbing|Animation")
    float ApproachSpeed = 200.0f; // Еще медленнее

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle Climbing|Animation")
    float ClimbSpeed = 80.0f; // Очень плавно

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle Climbing|Animation")
    float VaultSpeed = 100.0f; // Очень плавно

    // Настройки плавности
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle Climbing|Animation")
    float SmoothnessFactor = 2.0f; // Коэффициент плавности (больше = плавнее)

    // Состояние
    UPROPERTY(BlueprintReadOnly, Category = "Obstacle Climbing|State")
    bool bIsClimbing = false;

    UPROPERTY(BlueprintReadOnly, Category = "Obstacle Climbing|State")
    EObstacleClimbType CurrentClimbType = EObstacleClimbType::None;

    UPROPERTY(BlueprintReadOnly, Category = "Obstacle Climbing|State")
    FObstacleInfo CurrentObstacle;

    // События
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnClimbStarted, EObstacleClimbType, ClimbType);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnClimbCompleted);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnClimbFailed);

    UPROPERTY(BlueprintAssignable, Category = "Obstacle Climbing|Events")
    FOnClimbStarted OnClimbStarted;

    UPROPERTY(BlueprintAssignable, Category = "Obstacle Climbing|Events")
    FOnClimbCompleted OnClimbCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Obstacle Climbing|Events")
    FOnClimbFailed OnClimbFailed;

protected:
    // Владелец компонента
    AActor* OwnerActor = nullptr;

    // Текущая цель преодоления
    FVector TargetLocation;
    FVector StartLocation;
    float ClimbProgress = 0.0f;

    // Внутренние методы
    void StartClimbAnimation(const FObstacleInfo& Obstacle);
    void UpdateClimbAnimation(float DeltaTime);
    void CompleteClimb();
    void CancelClimb();

    // Проверка препятствия
    bool IsValidObstacle(const FHitResult& HitResult) const;
    FObstacleInfo AnalyzeObstacle(const FHitResult& HitResult) const;
    EObstacleClimbType DetermineClimbType(const FObstacleInfo& Obstacle) const;

    // Анимация преодоления
    void PerformVaultAnimation(float DeltaTime);
    void PerformClimbAnimation(float DeltaTime);
    void PerformClimbOverAnimation(float DeltaTime);
};
