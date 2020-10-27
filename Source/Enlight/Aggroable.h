// Enlight Game 2019

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Aggroable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UAggroable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ENLIGHT_API IAggroable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Aggroable")
		void OnAggro(AActor* SaveObject);
};
