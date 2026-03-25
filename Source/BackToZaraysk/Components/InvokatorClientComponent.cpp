// Fill out your copyright notice in the Description page of Project Settings.


#include "InvokatorClientComponent.h"

void UInvokatorClientComponent::OnInvoked(bool bIsActorInside)
{
	if (OnInvoctorActivated.IsBound())
	{
		OnInvoctorActivated.Broadcast(bIsActorInside);
	}
}

// Called when the game starts
void UInvokatorClientComponent::BeginPlay()
{
	Super::BeginPlay();
	if (IsValid(Invokator))
	{
		Invokator->OnInvoked.AddUObject(this, &UInvokatorClientComponent::OnInvoked);
	}
	
}


