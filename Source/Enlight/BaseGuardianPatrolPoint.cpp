// Enlight Game 2019


#include "BaseGuardianPatrolPoint.h"

// Sets default values
ABaseGuardianPatrolPoint::ABaseGuardianPatrolPoint()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABaseGuardianPatrolPoint::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABaseGuardianPatrolPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

