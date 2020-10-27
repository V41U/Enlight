// Enlight Game 2019

#pragma once

#include "CoreMinimal.h"
#include "Sound/SoundCue.h"
#include "Engine/UserDefinedStruct.h"
#include "MonologStruct.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FMonologStruct
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monolog", Meta = (MultiLine = true))
		FText MonologText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monolog")
		USoundCue* SpokenMonologCue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monolog")
		float ShowingDuration;

	// Default Constructor
	FMonologStruct()
	{
		ShowingDuration = 0;
	}
};