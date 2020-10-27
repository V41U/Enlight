// Enlight Game 2019


#include "FogKillArea.h"

// Sets default values
AFogKillArea::AFogKillArea()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SpawnContainer = CreateDefaultSubobject<UBoxComponent>(TEXT("Container"));
	SpawnContainer->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AFogKillArea::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AFogKillArea::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

