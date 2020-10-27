// Enlight Game 2019

#include "ActorSpawner.h"
#include "PlayerCharacter.h"

#include "GameFramework/PlayerController.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "Runtime/Engine/Public/TimerManager.h"
#include "NavigationSystem.h"


// Sets default values
AActorSpawner::AActorSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SpawnContainer = CreateDefaultSubobject<UBoxComponent>(TEXT("Container"));
	SpawnContainer->SetCollisionEnabled(ECollisionEnabled::QueryOnly);					// no physics
	SpawnContainer->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel2);	// interaction
	SpawnContainer->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);	// ignore all channels
	SpawnContainer->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap); // only overlap with Pawns
	SpawnContainer->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AActorSpawner::BeginPlay()
{
	Super::BeginPlay();

	//OnDestroyed.AddDynamic(this, &AActorSpawner::DestroyAllSpawnedActors);
	
	if (SpawnableActors.Num() > 0)
	{
		//spawn start actors
		for (int32 i = 0; i < SpawnedActorsOnBeginPlay; ++i)
		{
			SpawnActor();
		}

		// If the timer has expired or does not exist, start it  
		UWorld* World = GetWorld();
		if (World)
		{
			World->GetTimerManager().SetTimer(SpawnTimerHandle, this, &AActorSpawner::Spawn, FMath::RandRange(MinTimeBetweenSpawns, MaxTimeBetweenSpawns), false);
		}
	}
}

void AActorSpawner::StopSpawnTimer()
{
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(SpawnTimerHandle);
	}
}

void AActorSpawner::DestroyAllSpawnedActors(AActor* Destroyer)
{
	for (int32 NumActor = 0; NumActor < SpawnedActors.Num(); ++NumActor)
	{
		if (SpawnedActors[NumActor].IsValid())
		{
			ABuffableCharacter* BuffableCharacter = Cast<ABuffableCharacter>(SpawnedActors[NumActor]);
			if (IsValid(BuffableCharacter))
				BuffableCharacter->Suicide();
		}
	}
}

AActor* AActorSpawner::SpawnActor()
{
	UE_LOG(LogTemp, Display, TEXT("ActorSpawner - SpawnActor - Begin"));

	bool ValidSpawn;
	AActor* Spawned = nullptr;

	if (bOnlySpawnWhenPlayerInArea)
	{
		TArray<AActor*> OverlappingActors;
		SpawnContainer->GetOverlappingActors(OverlappingActors, APlayerCharacter::StaticClass());
		bPlayerInSpawnArea = OverlappingActors.Num() > 0;
	}
	
	if (!bOnlySpawnWhenPlayerInArea || bPlayerInSpawnArea)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			APlayerController* PlayerController = (World != nullptr) ? World->GetFirstPlayerController() : nullptr;
			APlayerCharacter * Player = (PlayerController != nullptr) ? Cast<APlayerCharacter>(PlayerController->GetPawn()) : nullptr;

			//SPAWN ACTOR IN BOX BOUNDS
			FActorSpawnParameters SpawnParameters;

			FVector Location = FMath::RandPointInBox(SpawnContainer->CalcBounds(GetActorTransform()).GetBox());

			// Try to get the nearest valid navigatable point, so the Enemy has a valid HomeLocation
			UNavigationSystemV1* NavigationSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
			FNavLocation NewNavLocation;
			FVector QueryingExtent = FVector(500.0f, 500.0f, 2500.0f);
			//NavigationSystem->GetRandomReachablePointInRadius(Location, 500, NewNavLocation);
			ValidSpawn = NavigationSystem->ProjectPointToNavigation(Location, NewNavLocation, QueryingExtent);
			if (ValidSpawn)
			{
				Location = NewNavLocation.Location;
				if (Player)
				{
					if (FVector::Dist(Location, Player->GetActorLocation()) < MinDistanceToPlayerToSpawn)
						return nullptr; // within the double player radius no spawn is allowed
				}

				Location += FVector(0.0f, 0.0f, 35.0f); // Slightly spawn above, so that actors don't fall through the ground sometimes

				Spawned = World->SpawnActor<AActor>(SpawnableActors[FMath::RandRange(0, (SpawnableActors.Num() - 1))], Location, GetActorRotation());
				if (Spawned)
				{
					UE_LOG(LogTemp, Display, TEXT("Spawned: %s"), *(Spawned->GetClass()->GetDefaultObjectName().GetPlainNameString()));
					SpawnedActors.Add(Spawned);
				}
			}
		}
	}

	UE_LOG(LogTemp, Display, TEXT("ActorSpawner - SpawnActor - Finished"));
	return Spawned;
}

void AActorSpawner::Spawn()
{
	// First Clean up
	for (int32 NumActor = 0; NumActor < SpawnedActors.Num(); ++NumActor)
	{
		if (!SpawnedActors[NumActor].IsValid())
		{
			SpawnedActors.RemoveAt(NumActor);
			--NumActor; // SpawnedActors is automatically shrinked
		}
	}

	// Check, if there are free slots
	if (SpawnedActors.Num() < MaxAllowedActors)
	{
		// Check, if all slots have been full previously
		if (bNoFreeSlots)
		{
			// Some spawned actor just died. Now we can start the Respawn-Timer for this free slot.
			bNoFreeSlots = false;
			GetWorld()->GetTimerManager().SetTimer(SpawnTimerHandle, this, &AActorSpawner::Spawn, FMath::RandRange(MinTimeBetweenSpawns, MaxTimeBetweenSpawns), false);
		}
		else
		{
			// Everything as usual - Spawn a new Actor.
			AActor* Spawned = SpawnActor();
			if (Spawned)
			{
				if (!SingleSpawn)
				{
					GetWorld()->GetTimerManager().SetTimer(SpawnTimerHandle, this, &AActorSpawner::Spawn, FMath::RandRange(MinTimeBetweenSpawns, MaxTimeBetweenSpawns), false);
				}
			}
			else
			{
				GetWorld()->GetTimerManager().SetTimer(SpawnTimerHandle, this, &AActorSpawner::Spawn, RetryAfterFailedSpawnInterval, false);
			}
		}
	}
	else
	{
		// No free Slots - Cleanup regurarly and Wait for a free slot.
		bNoFreeSlots = true;
		GetWorld()->GetTimerManager().SetTimer(SpawnTimerHandle, this, &AActorSpawner::Spawn, 1.0f, false);
	}
}

// Called every frame
void AActorSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

