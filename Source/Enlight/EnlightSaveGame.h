// Enlight Game 2019

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "EnlightSaveGame.generated.h"


USTRUCT(BlueprintType)
struct FBuffPlantBaseSaveData
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BuffPlantBase")
		int32 PlantStage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BuffPlantBase")
		bool bIsGrowing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BuffPlantBase")
		float MissingGrowingTime;

	// Default Constructor
	FBuffPlantBaseSaveData()
	{
		PlantStage = 0;
		bIsGrowing = false;
		MissingGrowingTime = 0;
	}

	/*
	// Make Constructor
	FBuffPlantBaseSaveData(int32 PlantStage, bool bIsGrowing, float MissingGrowingTime)
	{
		this->PlantStage = PlantStage;
		this->bIsGrowing = bIsGrowing;
		this->MissingGrowingTime = MissingGrowingTime;
	}
	*/
};


/**
 * 
 */
UCLASS()
class ENLIGHT_API UEnlightSaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:

	/** Name of the Save Game. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meta")
		FName SaveName;

	/** Name of the Level when the game was saved. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meta")
		FName LevelName;

	/** Date and Time the game was saved. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meta")
		FDateTime SaveTime;


	// Player
	// #############################################################

	/** Location of the Player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
		FTransform PlayerTransform;

	/** Rotation of the Camera */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
		FRotator CameraRotation;


	// BuffPlantBase
	// #############################################################

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BuffPlantBase")
		TMap<FString, FBuffPlantBaseSaveData> BuffPlantBaseSaveMap;
};
