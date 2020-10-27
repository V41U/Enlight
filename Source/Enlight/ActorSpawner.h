// Enlight Game 2019

#pragma once

#include "Runtime/Engine/Classes/Components/BoxComponent.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Delegates/Delegate.h"
#include "Components/ActorComponent.h"
#include "ActorSpawner.generated.h"

UCLASS()
class ENLIGHT_API AActorSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AActorSpawner();

	//directly callable
	UFUNCTION(BlueprintCallable)
		AActor* SpawnActor();

	//no return value so that timer can call it without any extra work
	UFUNCTION()
		void Spawn();

	// Called, when the spawner is destroyed, usually when the linked base is enlighted
	UFUNCTION(BlueprintCallable)
		void DestroyAllSpawnedActors(AActor* Destroyer);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bOnlySpawnWhenPlayerInArea = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float MinDistanceToPlayerToSpawn = 2000.0f;

	UPROPERTY(BlueprintReadOnly)
		bool bPlayerInSpawnArea = false;

	//if only single spawn is wanted
	UPROPERTY(EditAnywhere)
		bool SingleSpawn = false;

	UPROPERTY(EditAnywhere)
		int32 MaxAllowedActors = 5;

	//please leave this at 0 if this is used as a single spawner
	UPROPERTY(EditAnywhere)
		int32 SpawnedActorsOnBeginPlay = 0;

	UPROPERTY(EditAnywhere)
		UBoxComponent* SpawnContainer;

	UPROPERTY()
		TArray<TWeakObjectPtr<AActor>> SpawnedActors;

	//a random actor from this array will be spawned
	UPROPERTY(EditAnywhere)
		TArray<TSubclassOf<AActor>> SpawnableActors;

	UPROPERTY(EditAnywhere)
		float MinTimeBetweenSpawns = 1.0f;
	UPROPERTY(EditAnywhere)
		float MaxTimeBetweenSpawns = 1.0f;
	UPROPERTY(EditAnywhere)
		float RetryAfterFailedSpawnInterval = 5.0f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void StopSpawnTimer();

private:

	FTimerHandle SpawnTimerHandle;
	bool bNoFreeSlots = false;
};
