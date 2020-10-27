// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Runtime/Engine/Classes/Engine/EngineTypes.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Buff.generated.h"

UENUM(BlueprintType)
enum class ApplyMode : uint8
{
	UNIQUE UMETA(DisplayName = "Unique"),
	UNIQUEPERSOURCE UMETA(DisplayName = "Unique Per Source"),
	STACKABLE UMETA(DisplayName = "Stackable"),
};

UCLASS()
class ENLIGHT_API ABuff : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABuff();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (ExposeOnSpawn = true))
		AActor* BuffSource;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (ExposeOnSpawn = true))
		ApplyMode BuffApplyMode = ApplyMode::UNIQUE;

	//used for BuffTimerHandle
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (ExposeOnSpawn = true))
		float Uptime = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (ExposeOnSpawn = true))
		float TickPerSecond = 4.0f;

	UFUNCTION()
		void StartBuff();

	UFUNCTION()
		void TickBuff();

	UFUNCTION(BlueprintCallable)
		void EndBuff();

	UFUNCTION()
		void ResetBuff();

	UFUNCTION(BlueprintImplementableEvent)
		void OnBuffStarted();

	UFUNCTION(BlueprintImplementableEvent)
		void OnBuffTick();

	UFUNCTION(BlueprintImplementableEvent)
		void OnBuffEnded();

private:
	FTimerHandle BuffTickTimerHandle;
	FTimerHandle BuffEndTimerHandle;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};