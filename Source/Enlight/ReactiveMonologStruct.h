// Enlight Game 2019

#pragma once

#include "CoreMinimal.h"
#include "Containers/Set.h"
#include "Containers/Array.h"
#include "MonologStruct.h"
#include "ReactiveMonologStruct.generated.h"

/**
 *
 */
USTRUCT(BlueprintType)
struct FReactiveMonologStruct
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monolog")
		TSet<TSubclassOf<class AActor>> ReactionActors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monolog")
		TArray<FMonologStruct> MonologOrder;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monolog")
		int32 FirstMonologCounter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monolog")
		int32 MonologCounterInterval;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monolog")
		int32 MaxMonologReactions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monolog")
		bool bInfiniteMonologReactions;

	// Default Constructor
	FReactiveMonologStruct()
	{
		FirstMonologCounter = 1;
		MonologCounterInterval = 1;
		MaxMonologReactions = 1;
		bInfiniteMonologReactions = false;
	}
};