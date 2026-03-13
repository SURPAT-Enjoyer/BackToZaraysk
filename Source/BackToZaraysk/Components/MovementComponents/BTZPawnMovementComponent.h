#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "BTZPawnMovementComponent.generated.h"


UCLASS()
class BACKTOZARAYSK_API UBTZPawnMovementComponent : public UPawnMovementComponent
{
	GENERATED_BODY()

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	void JumpStart();
	virtual bool IsFalling() const override;

protected:
	UPROPERTY(EditAnywhere)
	float MaxSpeed = 1200.0f;
	UPROPERTY(EditAnywhere)
	bool bEnableGravity = true;
	bool bIsFalling;
	UPROPERTY(EditAnywhere)
	float InitialJumpVelocity = 500.0f;

public:
	FVector VerticalVelocity = FVector::ZeroVector;

};
