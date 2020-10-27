// Enlight Game 2019

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "EnlightSaveGame.h"
#include "EnlightGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class ENLIGHT_API UEnlightGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:

	/** Variable to decide, if the current game session is a completely new game or was loaded. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meta")
		bool bNewGame = true;

	/** All data from the loaded game slot. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meta")
		UEnlightSaveGame* SaveGame;

	/** Damage the player gets per second from the fog. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constants")
		float FogDamage = 2.0f;
};
