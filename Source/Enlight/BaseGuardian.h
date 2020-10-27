// Enlight Game 2019

#pragma once

#include "BaseGuardianPatrolPoint.h"
#include "PlayerCharacter.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseGuardian.generated.h"

UCLASS()
class ENLIGHT_API ABaseGuardian : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseGuardian();
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		FVector Move = FVector(0.0f, 0.0f, 0.0f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		float MaxMoveSpeed = 300.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		bool bTriggerActivated = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		float TriggerForceMultiplier = 100.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		bool bPatrolActivated = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		float PatrolForce = 10.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		float PatrolToleratedRadius = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guardian")
		int GuardianUpdatesPerSecond = 4;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guardian")
		float MaximumDistanceToPlayerForTrigger = 4000.0f;


	//guardian hint triggers
	UPROPERTY(BlueprintReadWrite, Category = "Guardian Hint")
		float CurrentHintTriggerValue = 0.0f;
	//Max capacity of triggers
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guardian Hint")
		float MaxHintTriggerCapacity = 10.0f;
	//how much each trigger can add at most to MaxTriggerHintCapacity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guardian Hint")
		float SingleHintTriggerPower = 1.0f;
	// decay per second of the current value (so the greater the hint value, the more it decays)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guardian Hint", meta = (ClampMin = "0.0", ClampMax = "100.0"))
		float HintDecayPerSecond = 10.0f;

	UFUNCTION()
		void GuardianUpdate();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	TArray<AActor*> PatrolPoints;
	AActor* ActivePatrolPoint;

	APlayerCharacter* Player;

	FTimerHandle GuardianUpdateTimerHandle;
};
