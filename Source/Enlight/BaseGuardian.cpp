// Enlight Game 2019

#include "BaseGuardian.h"

#include "EnlightGameState.h"

#include "Runtime/Engine/Classes/Engine/World.h"
#include "Runtime/Engine/Public/TimerManager.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ABaseGuardian::ABaseGuardian()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABaseGuardian::BeginPlay()
{
	Super::BeginPlay();


	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		Player = (PlayerController != nullptr) ? Cast<APlayerCharacter>(PlayerController->GetPawn()) : nullptr;

		World->GetTimerManager().SetTimer(GuardianUpdateTimerHandle, this, &ABaseGuardian::GuardianUpdate, 1 / (float)GuardianUpdatesPerSecond, true);

		UGameplayStatics::GetAllActorsOfClass(World, ABaseGuardianPatrolPoint::StaticClass(), PatrolPoints);
		if (PatrolPoints.Num() > 0)
			ActivePatrolPoint = PatrolPoints[0];
	}
}

// Called every frame
void ABaseGuardian::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector CurrentLocation = GetActorLocation();
	FVector NextLocation = CurrentLocation + Move * DeltaTime;
	if(IsValid(Player))
		NextLocation.Z = Player->GetActorLocation().Z;
	SetActorLocation(NextLocation);


	CurrentHintTriggerValue *= FMath::Pow(1.0f - (HintDecayPerSecond / 100.0f), DeltaTime);
	if (CurrentHintTriggerValue < 0.0f)
		CurrentHintTriggerValue = 0.0f;
}

void ABaseGuardian::GuardianUpdate()
{
	UE_LOG(LogTemp, Display, TEXT("BaseGuardian - GuardianUpdate - Begin"));

	//TODO: if no trigger has been received for a period of time only then the patrol point should have an impact. and if a trigger is send all the patrol movement should be reset to 0
	// this would make a more impactful movement and the retreating to patrol points would feel smoother

	UWorld* World = GetWorld();
	AEnlightGameState* GameState = World ? World->GetGameState<AEnlightGameState>() : nullptr;
	if (GameState)
	{
		//check triggers
		TArray<FGuardianTrigger> Triggers = GameState->ConsumeGuardianTriggers();
		if (bTriggerActivated && Triggers.Num() > 0)
		{
			for (auto SingleTrigger : Triggers)
			{
				AActor* TriggerActor = SingleTrigger.GuardianTrigger;
				if (IsValid(TriggerActor))
				{
					//update guardian hint trigger
					//only update hint if the TRIGGER has been within a certain range of the PLAYER (not guardian!)
					if (IsValid(Player) && FVector::Distance(Player->GetActorLocation(), TriggerActor->GetActorLocation()) < MaximumDistanceToPlayerForTrigger)
					{
						// Add Trigger Force
						FVector Direction = TriggerActor->GetActorLocation() - GetActorLocation();
						Direction.Normalize();
						Move += (Direction * TriggerForceMultiplier) * SingleTrigger.Strength;

						// Add Hint Value
						CurrentHintTriggerValue += SingleHintTriggerPower * SingleTrigger.Strength;
						if (CurrentHintTriggerValue > MaxHintTriggerCapacity)
							CurrentHintTriggerValue = MaxHintTriggerCapacity;
					}
				}
			}
		}
		else
		{
		//check patrol (only if no triggers are happening
			if (bPatrolActivated && PatrolPoints.Num() > 0)
			{
				FVector PatrolDirection = ActivePatrolPoint->GetActorLocation() - GetActorLocation();
				if (FVector::DistXY(ActivePatrolPoint->GetActorLocation(), GetActorLocation()) < PatrolToleratedRadius)
				{//update active patrol point
					ActivePatrolPoint = PatrolPoints[FMath::RandRange(0, PatrolPoints.Num() - 1)];
				}
				else
				{//otherwise push toward patrol point
					PatrolDirection.Normalize();
					Move += PatrolDirection * PatrolForce / (float)GuardianUpdatesPerSecond;
				}

			}
		}

		if (Move.Size() > MaxMoveSpeed)
		{
			Move.Normalize();
			Move *= MaxMoveSpeed;
		}
	}

	UE_LOG(LogTemp, Display, TEXT("BaseGuardian - GuardianUpdate - Finished"));
}
