// Enlight Game 2019

#pragma once

#include "SkyMapComponent.h"
#include "Components/StaticMeshComponent.h"
#include "ActorSpawner.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseSavingPoint.generated.h"

UCLASS()
class ENLIGHT_API ABaseSavingPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseSavingPoint();

	UFUNCTION(BlueprintCallable)
		void DestroyDedicatedSpawner();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		USkyMapComponent* SkyMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		AActorSpawner* DedicatedSpawner;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
