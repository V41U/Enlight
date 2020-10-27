// Enlight Game 2019

#pragma once

#include "Runtime/Engine/Classes/Components/BoxComponent.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FogKillArea.generated.h"

UCLASS()
class ENLIGHT_API AFogKillArea : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFogKillArea();

	UPROPERTY(EditAnywhere)
		UBoxComponent* SpawnContainer;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
