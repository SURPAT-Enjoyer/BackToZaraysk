#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/TimelineComponent.h"
#include "StrafeComponent.generated.h"

UENUM(BlueprintType)
enum class EStrafeType : uint8
{
    None,           // Стрейф недоступен
    Left,           // Стрейф влево
    Right,          // Стрейф вправо
    Forward,        // Стрейф вперед (для будущего расширения)
    Backward        // Стрейф назад (для будущего расширения)
};

USTRUCT(BlueprintType)
struct FStrafeInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FVector StartLocation;

    UPROPERTY(BlueprintReadOnly)
    FVector TargetLocation;

    UPROPERTY(BlueprintReadOnly)
    EStrafeType StrafeType;

    UPROPERTY(BlueprintReadOnly)
    float StrafeDistance;

    UPROPERTY(BlueprintReadOnly)
    bool bCanStrafe;

    FStrafeInfo()
    {
        StartLocation = FVector::ZeroVector;
        TargetLocation = FVector::ZeroVector;
        StrafeType = EStrafeType::None;
        StrafeDistance = 0.0f;
        bCanStrafe = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BACKTOZARAYSK_API UStrafeComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UStrafeComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Попытка выполнения стрейфа
    UFUNCTION(BlueprintCallable, Category = "Strafe")
    bool TryStrafe(EStrafeType StrafeDirection);

    // Проверка возможности стрейфа
    UFUNCTION(BlueprintCallable, Category = "Strafe")
    bool CanStrafe() const;

    // Настройки стрейфа
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Strafe|Settings")
    float StrafeDistance = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Strafe|Settings")
    float StrafeDuration = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Strafe|Settings")
    float StrafeCooldown = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Strafe|Settings")
    float SpaceDoublePressWindow = 0.5f;

    // Настройки движения
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Strafe|Movement")
    float StrafeSpeed = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Strafe|Movement")
    float SmoothnessFactor = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Strafe|Movement")
    class UCurveFloat* StrafeCurve;

    // Состояние стрейфа
    UPROPERTY(BlueprintReadOnly, Category = "Strafe|State")
    bool bIsStrafing = false;

    UPROPERTY(BlueprintReadOnly, Category = "Strafe|State")
    EStrafeType CurrentStrafeType = EStrafeType::None;

    UPROPERTY(BlueprintReadOnly, Category = "Strafe|State")
    FStrafeInfo CurrentStrafeInfo;

    // ИСПРАВЛЕНО: Добавлена переменная для сохранения режима движения
    UPROPERTY(BlueprintReadOnly, Category = "Strafe|State")
    TEnumAsByte<EMovementMode> PreviousMovementMode = MOVE_Walking;

    // Input tracking
    UPROPERTY(BlueprintReadOnly, Category = "Strafe|Input")
    bool bIsADown = false;

    UPROPERTY(BlueprintReadOnly, Category = "Strafe|Input")
    bool bIsDDown = false;

    UPROPERTY(BlueprintReadOnly, Category = "Strafe|Input")
    bool bIsSpaceDown = false;

    // События
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStrafeStarted, EStrafeType, StrafeType);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStrafeCompleted);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStrafeFailed);

    UPROPERTY(BlueprintAssignable, Category = "Strafe|Events")
    FOnStrafeStarted OnStrafeStarted;

    UPROPERTY(BlueprintAssignable, Category = "Strafe|Events")
    FOnStrafeCompleted OnStrafeCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Strafe|Events")
    FOnStrafeFailed OnStrafeFailed;

    // Input handling
    UFUNCTION(BlueprintCallable, Category = "Strafe|Input")
    void HandleAInput(bool bPressed);

    UFUNCTION(BlueprintCallable, Category = "Strafe|Input")
    void HandleDInput(bool bPressed);

    UFUNCTION(BlueprintCallable, Category = "Strafe|Input")
    void HandleSpaceInput(bool bPressed);

protected:
    // Владелец компонента
    AActor* OwnerActor = nullptr;

    // Текущее состояние стрейфа
    FVector StartLocation;
    FVector TargetLocation;
    float StrafeProgress = 0.0f;
    float LastStrafeTime = 0.0f;

    // Внутренние методы
    void StartStrafeAnimation(const FStrafeInfo& StrafeInfo);
    void UpdateStrafeAnimation(float DeltaTime);
    void CompleteStrafe();
    void CancelStrafe();

    // Проверка комбинации клавиш
    void CheckStrafeCombo();

    // Создание информации о стрейфе
    FStrafeInfo CreateStrafeInfo(EStrafeType StrafeType) const;
};
