// Fill out your copyright notice in the Description page of Project Settings.

#include "Buff.h"

//timer
#include "Runtime/Engine/Classes/Engine/World.h"
#include "Runtime/Engine/Public/TimerManager.h"

#include "BuffableCharacter.h"

#include "Engine/Engine.h"

// Sets default values
ABuff::ABuff()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ABuff::BeginPlay()
{
	Super::BeginPlay();
}

void ABuff::StartBuff()
{
	OnBuffStarted();

	TickBuff();

	//automatically start end timer
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(BuffEndTimerHandle, this, &ABuff::EndBuff, Uptime, false);
	}
}

void ABuff::TickBuff()
{
	OnBuffTick();

	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(BuffTickTimerHandle, this, &ABuff::TickBuff, 1 / TickPerSecond, false);
	}
}

void ABuff::EndBuff()
{
	OnBuffEnded();

	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(BuffEndTimerHandle);
		World->GetTimerManager().ClearTimer(BuffTickTimerHandle);
	}

	AActor* ActorOwner = GetOwner();
	if (ActorOwner)
	{
		ABuffableCharacter* BuffOwner = Cast<ABuffableCharacter>(ActorOwner);
		if (BuffOwner)
		{
			BuffOwner->RemoveBuff(this);
		}
	}

	Destroy();
}

void ABuff::ResetBuff()
{
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(BuffEndTimerHandle, this, &ABuff::EndBuff, Uptime, false);
	}
}

// Called every frame
void ABuff::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

