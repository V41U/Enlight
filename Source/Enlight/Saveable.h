// Enlight Game 2019

#pragma once

//#include "Runtime/Engine/Classes/GameFramework/SaveGame.h"
#include "EnlightSaveGame.h"

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Saveable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class USaveable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ENLIGHT_API ISaveable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Saveable")
		void OnSaving(UEnlightSaveGame* SaveObject);
};
