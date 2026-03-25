#include "BTZFabrikLegIK.h"

namespace BTZFabrikLegIK_Internal
{
	static FVector ConstrainKneeToPolePlane(
		const FVector& Hip,
		const FVector& FootTarget,
		const FVector& Pole,
		float UpperLen,
		FVector Knee)
	{
		if (UpperLen < KINDA_SMALL_NUMBER)
		{
			return Knee;
		}

		FVector N = FVector::CrossProduct(FootTarget - Hip, Pole - Hip).GetSafeNormal();
		if (N.SizeSquared() < 0.01f)
		{
			N = FVector::CrossProduct(FootTarget - Hip, FVector::UpVector).GetSafeNormal();
		}
		if (N.SizeSquared() < 0.01f)
		{
			return Knee;
		}

		Knee -= N * FVector::DotProduct(Knee - Hip, N);
		const FVector Dir = (Knee - Hip).GetSafeNormal();
		if (Dir.SizeSquared() < 0.01f)
		{
			return Hip + FVector(0.f, UpperLen, 0.f);
		}
		return Hip + Dir * UpperLen;
	}
}

FVector FBTZFabrikLegIK::SolveKneeWorld(
	const FVector& HipWorld,
	const FVector& InitialKneeWorld,
	const FVector& TargetFootWorld,
	float UpperBoneLengthCm,
	float LowerBoneLengthCm,
	const FVector& PoleWorld,
	int32 MaxIterations,
	float ToleranceCm)
{
	using namespace BTZFabrikLegIK_Internal;

	if (UpperBoneLengthCm < KINDA_SMALL_NUMBER || LowerBoneLengthCm < KINDA_SMALL_NUMBER)
	{
		return InitialKneeWorld;
	}

	const float TotalLen = UpperBoneLengthCm + LowerBoneLengthCm;
	const FVector ToTarget = TargetFootWorld - HipWorld;
	const float Dist = ToTarget.Size();
	if (Dist < KINDA_SMALL_NUMBER)
	{
		return InitialKneeWorld;
	}

	// Unreachable: fully extend toward target (article: straight chain to t).
	if (Dist >= TotalLen - 0.05f)
	{
		const FVector Dir = ToTarget / Dist;
		FVector Knee = HipWorld + UpperBoneLengthCm * Dir;
		return ConstrainKneeToPolePlane(HipWorld, TargetFootWorld, PoleWorld, UpperBoneLengthCm, Knee);
	}

	// p0 = hip, p1 = knee, p2 = foot. d1 = upper, d2 = lower (article indexing).
	const float d1 = UpperBoneLengthCm;
	const float d2 = LowerBoneLengthCm;
	const float TolSq = FMath::Square(FMath::Max(ToleranceCm, 0.01f));

	FVector p0 = HipWorld;
	FVector p1 = InitialKneeWorld;
	FVector p2 = TargetFootWorld;

	MaxIterations = FMath::Clamp(MaxIterations, 1, 32);

	for (int32 Iter = 0; Iter < MaxIterations; ++Iter)
	{
		// --- Forward reaching (end toward root) ---
		p2 = TargetFootWorld;

		// i = 2: update knee from foot
		{
			float R = FVector::Distance(p2, p1);
			if (R < SMALL_NUMBER)
			{
				p1 = p2 + FVector(0.f, 0.f, d2);
				R = d2;
			}
			const float Lambda = d2 / R;
			p1 = p2 * (1.f - Lambda) + p1 * Lambda;
		}
		// i = 1: update hip (temporary) from knee
		{
			float R = FVector::Distance(p1, p0);
			if (R < SMALL_NUMBER)
			{
				p0 = p1 - FVector(d1, 0.f, 0.f);
				R = d1;
			}
			const float Lambda = d1 / R;
			p0 = p1 * (1.f - Lambda) + p0 * Lambda;
		}

		// --- Backward reaching (fix root) ---
		p0 = HipWorld;

		// i = 1: knee from fixed hip
		{
			float R = FVector::Distance(p1, p0);
			if (R < SMALL_NUMBER)
			{
				p1 = p0 + FVector(0.f, 0.f, d1);
				R = d1;
			}
			const float Lambda = d1 / R;
			p1 = p0 * (1.f - Lambda) + p1 * Lambda;
		}
		// i = 2: foot from knee
		{
			float R = FVector::Distance(p2, p1);
			if (R < SMALL_NUMBER)
			{
				p2 = p1 + FVector(0.f, 0.f, d2);
				R = d2;
			}
			const float Lambda = d2 / R;
			p2 = p1 * (1.f - Lambda) + p2 * Lambda;
		}

		// Constraint: keep knee in bend plane (pole), preserve upper bone length (article §4 style).
		p1 = ConstrainKneeToPolePlane(p0, TargetFootWorld, PoleWorld, d1, p1);

		if (FVector::DistSquared(p2, TargetFootWorld) < TolSq)
		{
			break;
		}
	}

	return p1;
}
