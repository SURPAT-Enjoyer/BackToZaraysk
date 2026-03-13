#include "PlatformBase.h"


APlatformBase::APlatformBase()
{
	PrimaryActorTick.bCanEverTick = true;
	USceneComponent* DefaultPlatformRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Platform root"));
	RootComponent = DefaultPlatformRoot;

	PlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Platform"));
	PlatformMesh->SetupAttachment(DefaultPlatformRoot);
}


// Called when the game starts or when spawned
void APlatformBase::BeginPlay()
{
	Super::BeginPlay();
	//GEngine->AddOnScreenDebugMessage(0, 5, FColor::Red, "Hello world228");
	StartLocation = PlatformMesh->GetRelativeLocation();
	

	if (IsValid(TimelineCurve))
	{
		FOnTimelineFloatStatic PlatformMovementTimelineUpdate;
		PlatformMovementTimelineUpdate.BindUObject(this, &APlatformBase::PlatformTimelineUpdate);
		PlatformTimeline.AddInterpFloat(TimelineCurve, PlatformMovementTimelineUpdate);
	}

}

// Called every frame
void APlatformBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	PlatformTimeline.TickTimeline(DeltaTime);

}

void APlatformBase::PlatformTimelineUpdate(const float Alpha)
{
	const FVector PlatformTargetLocation = FMath::Lerp(StartLocation, EndLocation, Alpha);
	PlatformMesh->SetRelativeLocation(PlatformTargetLocation);
}

/*
void APlatformBase::PlatformForvardReverseTrigger()
{
	CurrentLocation = PlatformMesh->GetRelativeLocation();
	if (CurrentLocation == EndLocation)
	{
		ForvardReverseFlag = true;
		CurrentLocation = StartLocation;
	}
}
*/


