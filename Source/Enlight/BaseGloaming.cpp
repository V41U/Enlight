// Enlight Game 2019


#include "BaseGloaming.h"
///////////////////////////////

#include "GameFramework/CharacterMovementComponent.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "Runtime/Engine/Public/TimerManager.h"

#include "BuffableCharacter.h"
#include "PlayerCharacter.h"

ABaseGloaming::ABaseGloaming()
{
	SensingSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sensing Sphere"));
	SensingSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);					//no physics
	SensingSphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	SensingSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);	//overlapp all channels
	SensingSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Ignore); // ignore Navigation
	SensingSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel2, ECollisionResponse::ECR_Overlap); // overlap Interaction
	SensingSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel3, ECollisionResponse::ECR_Ignore); // ignore GloamingAttraction
	SensingSphere->InitSphereRadius(600.0f);
	SensingSphere->SetupAttachment(RootComponent);
	SensingSphere->OnComponentBeginOverlap.AddDynamic(this, &ABaseGloaming::OnSensingSphereBeginOverlap);
	SensingSphere->OnComponentEndOverlap.AddDynamic(this, &ABaseGloaming::OnSensingSphereEndOverlap);

	UE_LOG(LogInit, Warning, TEXT("Init MeshImitator."));
	MeshImitator = CreateDefaultSubobject<UNiagaraComponent>(TEXT("MeshImitator"));
	MeshImitator->SetupAttachment(RootComponent);

}

void ABaseGloaming::BeginPlay()
{
	Super::BeginPlay();
	SetGloamingState(StartingState);

	InitialDefaultMoveSpeed = DefaultMovementSpeed;
	InitialScaredMoveSpeed = ScaredMovementSpeed;

	FOnTimelineFloat onGloamingScalingTimelineCallback;
	if (GloamingScalingCurve) {
		GloamingScalingTimeline = NewObject<UTimelineComponent>(this, TEXT("GloamingScalingTimeline"));
		GloamingScalingTimeline->SetLooping(false); //non looping
		GloamingScalingTimeline->SetTimelineLength(0.8f);
		GloamingScalingTimeline->SetTimelineLengthMode(ETimelineLengthMode::TL_LastKeyFrame);
		GloamingScalingTimeline->SetPlaybackPosition(0.0f, false);

		onGloamingScalingTimelineCallback.BindUFunction(this, FName{ TEXT("GloamingScalingTimelineCallback") });
		GloamingScalingTimeline->AddInterpFloat(GloamingScalingCurve, onGloamingScalingTimelineCallback);

		GloamingScalingTimeline->RegisterComponent();
	}


	// start continuous sensing updates
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(SensingTimerHandle, this, &ABaseGloaming::SenseSphere, 1/SensesPerSecond, true);
	}
}

void ABaseGloaming::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//lerp movement speed during following
	if (!bCorrupted && !bTeleportable && State == EGloamingState::EG_Idle && GloamingOwner && IsValid(GloamingOwner))
	{
		float DistanceFloat = TargetFollowingDistance / 1.8f;
		float Multiplier = FMath::Loge(FVector::Distance(GetActorLocation(),
									   GloamingOwner->GetActorLocation()) / DistanceFloat + 1.0f);
		ABuffableCharacter* BuffableOwner = Cast<ABuffableCharacter>(GloamingOwner);
		if(BuffableOwner && IsValid(BuffableOwner))
			GetCharacterMovement()->MaxWalkSpeed = BuffableOwner->GetCharacterMovement()->MaxWalkSpeed * Multiplier;
	}

	if(bThrowingState)
		SacrificeProgress += SacrificeProgressGainPerSecond * DeltaTime;
	else
		SacrificeProgress -= SacrificeProgressDecayPerSecond * DeltaTime;

	if (SacrificeProgress < 0.0f)
		SacrificeProgress = 0.0f;
	else if (SacrificeProgress > MaxSacrificeProgress)
		Sacrifice();
}

bool ABaseGloaming::SetGloamingOwner(AActor * NewOwner)
{
	bool bOk = false;
	
	if (IsValid(NewOwner))
	{
		bCorrupted = false;
		GloamingOwner = NewOwner;

		bOk = true;
	}

	return bOk;
}

void ABaseGloaming::StartResetGloaming()
{
	UWorld* World = GetWorld();
	if (World && !World->GetTimerManager().IsTimerActive(ResetGloamingTimerHandle))
	{
		FTimerDelegate ResetCallback;
		ResetCallback.BindLambda([this]()
		{
			UE_LOG(LogTemp, Display, TEXT("BaseGloaming - ResetCallback - Begin"));
			if (IsValid(this))
			{
				if (IsValid(GloamingOwner))
				{
					APlayerCharacter* Player = Cast<APlayerCharacter>(GloamingOwner);
					Player->OwnedGloaming = nullptr;
				}

				GloamingOwner = nullptr;
				bCorrupted = true;
				DefaultMovementSpeed = InitialDefaultMoveSpeed;
				ScaredMovementSpeed = InitialScaredMoveSpeed;

				ReceiveOnGloamingReset();
			}
			UE_LOG(LogTemp, Display, TEXT("BaseGloaming - ResetCallback - Finished"));
		});


		World->GetTimerManager().SetTimer(ResetGloamingTimerHandle, ResetCallback, MaxTimeAwayFromOwnerBeforeReset, false);
	}
}

void ABaseGloaming::CancelResetGloaming()
{
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(ResetGloamingTimerHandle);
	}
}

void ABaseGloaming::HoldGloaming()
{
	if (!bTeleportable && bMayBeThrown && State != EGloamingState::EG_EnlightBase)
	{
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Ignore);
		bThrowingState = true;
		if (GloamingScalingCurve) {
			GloamingScalingTimeline->PlayFromStart();
		}

		ReceiveHoldGloaming();
	}
}

void ABaseGloaming::DropGloaming()
{
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Block);
	
	StartThrowCooldown();
	
	bThrowingState = false;
	if (GloamingScalingCurve) {
		GloamingScalingTimeline->Reverse();
	}
	OnThrowCanceled();
}

void ABaseGloaming::ThrowGloaming()
{
	if (bThrowingState)
	{
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Block);
		if (GloamingScalingCurve) {
			GloamingScalingTimeline->Reverse();
		}

		ReceiveThrowGloaming();

		UWorld* World = GetWorld();
		if (World)
		{
			bTeleportable = true;
			//we call a custom callback after ThrowStunTime seconds and set the bThrowingState to false;
			World->GetTimerManager().SetTimer(ThrowStunTimerHandle, this, &ABaseGloaming::OnThrowFinished, ThrowStunTime, false);

			StartThrowCooldown();
		}

		bThrowingState = false;
	}
}

void ABaseGloaming::StartThrowCooldown()
{
	bMayBeThrown = false;
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(ThrowCooldownTimerHandle, this, &ABaseGloaming::OnThrowCooldownOver, ThrowCooldownTime, false);
	}
}

void ABaseGloaming::OnThrowFinished()
{
	bTeleportable = false;

	ReceiveThrowFinished();
}

void ABaseGloaming::OnThrowCanceled()
{
	ReceiveOnThrowCanceled();
}

void ABaseGloaming::OnThrowCooldownOver()
{
	bMayBeThrown = true;

	ReceiveOnThrowCooldownOver();
}

void ABaseGloaming::GloamingScalingTimelineCallback(float Value)
{//called every tick of the timeline
	this->SetActorScale3D(FVector(Value, Value, Value));
	ReceiveGloamingScalingTimelineCallback(Value);
}

void ABaseGloaming::Sacrifice()
{
	ABuffableCharacter* BuffableOwner = Cast<ABuffableCharacter>(GloamingOwner);

	if (IsValid(BuffableOwner))
	{
		BuffableOwner->Heal(SacrificeHP);

		//notify
		OnSacrifice(SacrificeHP);
	}
}

void ABaseGloaming::SetGloamingState(EGloamingState NewState /*= EGloamingState::EG_Idle*/)
{
	//update class defaults
	switch (NewState)
	{
	case EGloamingState::EG_Idle:
		GetCharacterMovement()->MaxWalkSpeed = DefaultMovementSpeed;
		break;
	case EGloamingState::EG_Curious:
		GetCharacterMovement()->MaxWalkSpeed = CuriousMovementSpeed;
		break;
	case EGloamingState::EG_Scared:
		GetCharacterMovement()->MaxWalkSpeed = ScaredMovementSpeed;
		break;
	default:
		//something went wrong
		break;
	}

	State = NewState;

	ReceiveStateChange(State);
}

void ABaseGloaming::SenseSphere()
{
	UE_LOG(LogTemp, Display, TEXT("BaseGloaming - SenseSphere - Begin"));
	TArray<AActor*> OverlappingActors;

	//ATTENTION: currently only buffable characters are noticed during a sense update!! (maybe this is not a good idea?)
	SensingSphere->GetOverlappingActors(OverlappingActors);

	//ignore self
	if(OverlappingActors.Contains(this))
		OverlappingActors.Remove(this);

	//if gloaming has an owner then ignore this as well
	if (GloamingOwner)
	{
		if (OverlappingActors.Contains(GloamingOwner))
			OverlappingActors.Remove(GloamingOwner);
	}

	float Distance = 0.0f;

	if (OverlappingActors.Num() > 0)
	{
		//sort ascending by distance
		FVector OwnLocation = GetActorLocation();
		OverlappingActors.Sort([OwnLocation](const AActor& A, const AActor& B) {
			float DistanceToA = FVector::DistSquared(OwnLocation, A.GetActorLocation());
			float DistanceToB = FVector::DistSquared(OwnLocation, B.GetActorLocation());
			return DistanceToA < DistanceToB;
		});

		if (StateActorPriority.Num() > 0)
		{
			//sort by dynamic given type (like first buffable characters, then plants, then ...)
			OverlappingActors.Sort([this](const AActor& A, const AActor& B) {
				for (int32 nActorPrio = 0; nActorPrio < StateActorPriority.Num(); ++nActorPrio)
				{
					if (A.IsA(StateActorPriority[nActorPrio]))
						return true;
					else if (B.IsA(StateActorPriority[nActorPrio]))
						return false;
				}
				return true;
			});
		}

		ClosestActor = OverlappingActors[0];
		Distance = ClosestActor->GetDistanceTo(this);

	}
	else
	{
		//if nothing is found in this overlap then there is no valid closest actor
		ClosestActor = nullptr;
	}


	ReceiveSenseSphere(ClosestActor, Distance);
	UE_LOG(LogTemp, Display, TEXT("BaseGloaming - SenseSphere - Finished"));
}

void ABaseGloaming::OnSensingSphereBeginOverlap(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	//currently no interaction when new actors are entering/exiting sensing sphere
	

	//notify blueprint event
	ReceiveOnSensingSphereBeginOverlap(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
}

void ABaseGloaming::OnSensingSphereEndOverlap(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex)
{
	//we have to update the closest actor if said actor leaves the sensing sphere
	if (OtherActor == ClosestActor)
		ClosestActor = nullptr;

	//notify blueprint event
	ReceiveOnSensingSphereEndOverlap(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex);
}

void ABaseGloaming::OnHeal(float Heal, AActor* Healer /*= nullptr*/)
{
}

void ABaseGloaming::OnDamage(float Damage, AActor* DamageDealer /*= nullptr*/)
{
}

void ABaseGloaming::OnSuicide()
{
	bInvulnerable = false;
}

void ABaseGloaming::OnDeath(int RemainingLives, AActor* LastDamageDealer/* = nullptr*/)
{
	UE_LOG(LogTemp, Display, TEXT("BaseGloaming - OnDeath - Begin"));
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearAllTimersForObject(this);
		World->GetTimerManager().ClearTimer(SensingTimerHandle);
		World->GetTimerManager().ClearTimer(ThrowCooldownTimerHandle);
		World->GetTimerManager().ClearTimer(ThrowStunTimerHandle);
		World->GetTimerManager().ClearTimer(ResetGloamingTimerHandle);
	}
	GloamingScalingTimeline->Stop();
	UE_LOG(LogTemp, Display, TEXT("BaseGloaming - OnDeath - Finished"));
}

void ABaseGloaming::OnSaving_Implementation(UEnlightSaveGame * SaveObject)
{
	//save gloaming
}

bool ABaseGloaming::EnlightBase(AActor* CorruptedBase)
{
	if (!bThrowingState && !bTeleportable)
	{	// Stop Sensing
		UWorld* World = GetWorld();
		if (World)
		{
			World->GetTimerManager().ClearTimer(SensingTimerHandle);
		}
		// Set the Base as important Actor
		ClosestActor = CorruptedBase;
		State = EGloamingState::EG_EnlightBase;
		GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Overlap);
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);

		// Make invulnerable
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
		bInvulnerable = true;
		ReceiveOnEnlightBase(CorruptedBase);
		return true;
	}
	return false;
}