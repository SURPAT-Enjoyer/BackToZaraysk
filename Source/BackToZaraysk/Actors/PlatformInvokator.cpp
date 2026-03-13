// Fill out your copyright notice in the Description page of Project Settings.


#include "PlatformInvokator.h"

void APlatformInvokator::Invoke()
{
	bIsActorInside = !bIsActorInside;
	if (OnInvoked.IsBound())
	{
		OnInvoked.Broadcast(bIsActorInside);
	}
	
}
