// Enlight Game 2019

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "GlobalEventListener.h"
#include "EnlightGameState.generated.h"

USTRUCT(BlueprintType)
struct FGuardianTrigger
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		AActor* GuardianTrigger;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		float Strength;

	FGuardianTrigger(AActor* Actor, float fStrength)
	{
		GuardianTrigger = Actor;
		Strength = fStrength;
	}

	FGuardianTrigger()
	{
		GuardianTrigger = nullptr;
		Strength = 0.0f;
	}
};

/**
 * 
 */
UCLASS()
class ENLIGHT_API AEnlightGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
		void OnGlobalGuardianTrigger(AActor* GuardianTrigger, float Strength);

	//get all queued guadian trigger events and remove them
	UFUNCTION(BlueprintCallable)
		TArray<FGuardianTrigger> ConsumeGuardianTriggers();

	UFUNCTION(BlueprintCallable)
		void RegisterGlobalGameEventListener(TScriptInterface<IGlobalEventListener> Listener);

	UFUNCTION(BlueprintCallable)
		void DispatchGlobalGameEvent(FName EventName);

private:
	
	TArray<FGuardianTrigger> GuardianTriggers;
	
	UPROPERTY()
		TArray<TScriptInterface<IGlobalEventListener>> GlobalEventListeners;
};
