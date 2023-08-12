// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TimelineComponent.h"
#include "Math/UnrealMathUtility.h"
#include "PlatformBase.generated.h"

UENUM(BlueprintType)
enum class EPlatformBehavior : uint8
{
	OnDemand = 0, //-платформа перемещается только при запросе
	Loop //- платформа перемещается по циклу.
};

UCLASS()
class BACKTOZARAYSK_API APlatformBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APlatformBase();

	UFUNCTION(BlueprintCallable)
	void PlatformTimelineUpdate(float Alpha);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		EPlatformBehavior PlatformBehavior = EPlatformBehavior::OnDemand;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	
	//UPROPERTY(EditAnywhere, BlueprintReadOnly)
	//	int test = 100;
	FTimeline PlatformTimeline;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UStaticMeshComponent* PlatformMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient)
		FVector StartLocation;
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, meta = (MakeEditWidget))
		FVector EndLocation;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UCurveFloat* TimelineCurve;

	

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};

