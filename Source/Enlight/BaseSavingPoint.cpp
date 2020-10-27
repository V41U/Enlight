// Enlight Game 2019


#include "BaseSavingPoint.h"

// Sets default values
ABaseSavingPoint::ABaseSavingPoint()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	//RootComponent = Mesh;

	SkyMap = CreateDefaultSubobject<USkyMapComponent>(TEXT("SkyMap"));
}

void ABaseSavingPoint::DestroyDedicatedSpawner()
{
	if (IsValid(DedicatedSpawner))
	{
		DedicatedSpawner->StopSpawnTimer();
		DedicatedSpawner->DestroyAllSpawnedActors(this);
		DedicatedSpawner->Destroy();
	}
}

// Called when the game starts or when spawned
void ABaseSavingPoint::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ABaseSavingPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

