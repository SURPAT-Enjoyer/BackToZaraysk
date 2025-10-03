// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ObstacleClimbingComponent.generated.h"

// Типы препятствий для преодоления
UENUM(BlueprintType)
enum class EObstacleType : uint8
{
	None UMETA(DisplayName = "None"),
	Vault UMETA(DisplayName = "Vault"),      // Перепрыгивание (низкие препятствия 50-120 см)
	Mantle UMETA(DisplayName = "Mantle"),    // Подтягивание (средние препятствия 120-200 см)
	Climb UMETA(DisplayName = "Climb")       // Лазание (высокие препятствия 200+ см)
};

// Информация об обнаруженном препятствии
USTRUCT(BlueprintType)
struct FObstacleData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	EObstacleType Type = EObstacleType::None;

	UPROPERTY(BlueprintReadOnly)
	FVector ObstacleTop;        // Верхняя точка препятствия

	UPROPERTY(BlueprintReadOnly)
	FVector ObstacleForward;    // Направление "через" препятствие

	UPROPERTY(BlueprintReadOnly)
	float Height;               // Высота препятствия

	UPROPERTY(BlueprintReadOnly)
	float Thickness;            // Толщина препятствия

	bool IsValid() const { return Type != EObstacleType::None; }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnClimbStarted, EObstacleType, ObstacleType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnClimbCompleted);

/**
 * Компонент для преодоления препятствий с анимациями
 * Простая и понятная архитектура с минимумом кастомной логики
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BACKTOZARAYSK_API UObstacleClimbingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UObstacleClimbingComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Попытка преодолеть препятствие
	UFUNCTION(BlueprintCallable, Category = "Obstacle Climbing")
	bool TryClimb();

	// Текущее состояние лазания
	UPROPERTY(BlueprintReadOnly, Category = "Obstacle Climbing")
	bool bIsClimbing = false;

	// Текущий тип препятствия
	UPROPERTY(BlueprintReadOnly, Category = "Obstacle Climbing")
	EObstacleType CurrentObstacleType = EObstacleType::None;

	// Обнаружено ли препятствие впереди (обновляется каждый кадр)
	UPROPERTY(BlueprintReadOnly, Category = "Obstacle Climbing")
	bool bObstacleDetected = false;

	// Тип обнаруженного препятствия
	UPROPERTY(BlueprintReadOnly, Category = "Obstacle Climbing")
	EObstacleType DetectedObstacleType = EObstacleType::None;

	// События
	UPROPERTY(BlueprintAssignable, Category = "Obstacle Climbing")
	FOnClimbStarted OnClimbStarted;

	UPROPERTY(BlueprintAssignable, Category = "Obstacle Climbing")
	FOnClimbCompleted OnClimbCompleted;

protected:
	virtual void BeginPlay() override;

	// === НАСТРОЙКИ ОБНАРУЖЕНИЯ ===
	
	UPROPERTY(EditAnywhere, Category = "Detection")
	float DetectionDistance = 150.0f;  // Дистанция обнаружения препятствий (оптимально 150 см)

	UPROPERTY(EditAnywhere, Category = "Detection")
	float MinObstacleHeight = 30.0f;   // Минимальная высота препятствия (30 см от пола)

	UPROPERTY(EditAnywhere, Category = "Detection")
	float VaultMaxHeight = 110.0f;     // Максимальная высота для Vault (30-110 см)

	UPROPERTY(EditAnywhere, Category = "Detection")
	float MantleMaxHeight = 200.0f;    // Максимальная высота для Mantle (100-200 см)

	UPROPERTY(EditAnywhere, Category = "Detection")
	float MaxClimbHeight = 220.0f;     // Максимальная высота для лазания (выше - невозможно)

	UPROPERTY(EditAnywhere, Category = "Detection")
	float MaxObstacleThickness = 100.0f; // Максимальная толщина препятствия

	// === НАСТРОЙКИ АНИМАЦИИ ===
	
	UPROPERTY(EditAnywhere, Category = "Animation")
	float VaultDuration = 0.8f;        // Длительность анимации Vault

	UPROPERTY(EditAnywhere, Category = "Animation")
	float MantleDuration = 1.2f;       // Длительность анимации Mantle

	UPROPERTY(EditAnywhere, Category = "Animation")
	float ClimbDuration = 1.5f;        // Длительность анимации Climb

private:
	// Владелец компонента
	AActor* OwnerActor = nullptr;

	// Текущие данные о препятствии
	FObstacleData CurrentObstacle;

	// Прогресс анимации (0.0 - 1.0)
	float AnimationProgress = 0.0f;

	// Стартовая позиция персонажа
	FVector StartLocation;
	FRotator StartRotation;

	// Целевая позиция после преодоления
	FVector TargetLocation;
	FRotator TargetRotation;

	// === ФУНКЦИИ ОБНАРУЖЕНИЯ ===
	
	// Обнаружение препятствия перед персонажем
	FObstacleData DetectObstacle();

	// Определение типа препятствия по его параметрам
	EObstacleType DetermineObstacleType(float Height, float Thickness);

	// === ФУНКЦИИ ВЫПОЛНЕНИЯ ЛАЗАНИЯ ===
	
	// Начало лазания
	void StartClimbing(const FObstacleData& Obstacle);

	// Обновление анимации лазания
	void UpdateClimbing(float DeltaTime);

	// Завершение лазания
	void CompleteClimbing();

	// Отмена лазания
	void CancelClimbing();

	// === ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ===
	
	// Вычисление целевой позиции для типа препятствия
	void CalculateTargetTransform(const FObstacleData& Obstacle);

	// Блокировка управления персонажем
	void DisablePlayerControl();

	// Восстановление управления персонажем
	void EnablePlayerControl();

	// Плавная интерполяция позиции персонажа
	FVector GetInterpolatedLocation(float Progress) const;
	FRotator GetInterpolatedRotation(float Progress) const;
};
