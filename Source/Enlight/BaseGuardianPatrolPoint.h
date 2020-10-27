// Enlight Game 2019

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseGuardianPatrolPoint.generated.h"

UCLASS()
class ENLIGHT_API ABaseGuardianPatrolPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseGuardianPatrolPoint();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
