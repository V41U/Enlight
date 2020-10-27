// Enlight Game 2019


#include "EnlightGameState.h"

void AEnlightGameState::OnGlobalGuardianTrigger(AActor* GuardianTrigger, float Strength)
{
	GuardianTriggers.Push(FGuardianTrigger{ GuardianTrigger, Strength });
}

TArray<FGuardianTrigger> AEnlightGameState::ConsumeGuardianTriggers()
{
	TArray<FGuardianTrigger> Temp = GuardianTriggers;
	GuardianTriggers.Empty();
	return Temp;
}

void AEnlightGameState::RegisterGlobalGameEventListener(TScriptInterface<IGlobalEventListener> Listener)
{
	GlobalEventListeners.Add(Listener);
}

void AEnlightGameState::DispatchGlobalGameEvent(FName EventName)
{
	TScriptInterface<IGlobalEventListener> Listener;
	for (int idx = 0; idx < GlobalEventListeners.Num(); idx++)
	{
		Listener = GlobalEventListeners[idx];
		if (IsValid(Listener.GetObject()))
		{
			//Listener->OnGlobalGameEvent(EventName);
			Listener->Execute_OnGlobalGameEvent(Listener.GetObject(), EventName);
		}
		else
		{
			GlobalEventListeners.RemoveAt(idx);
			idx--;
		}
	}
}