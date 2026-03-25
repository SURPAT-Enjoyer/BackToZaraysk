// FABRIK (Forward And Backward Reaching IK) for a 2-bone leg chain.
// Used to compute knee (joint) hints for UE Two Bone IK from hip, foot target, and pole.

#pragma once

#include "CoreMinimal.h"

struct BACKTOZARAYSK_API FBTZFabrikLegIK
{
	/**
	 * 3-node chain: Hip (fixed) -- UpperLen -- Knee -- LowerLen -- Foot (pulled to TargetFoot).
	 * Returns a knee position in world space suitable as Two Bone IK "Joint Target" hint
	 * (point in the bend plane defined by Hip, TargetFoot, Pole).
	 */
	static FVector SolveKneeWorld(
		const FVector& HipWorld,
		const FVector& InitialKneeWorld,
		const FVector& TargetFootWorld,
		float UpperBoneLengthCm,
		float LowerBoneLengthCm,
		const FVector& PoleWorld,
		int32 MaxIterations = 8,
		float ToleranceCm = 0.15f);
};
