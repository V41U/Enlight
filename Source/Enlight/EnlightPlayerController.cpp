// Fill out your copyright notice in the Description page of Project Settings.

#include "EnlightPlayerController.h"


#include "PlayerCharacter.h"
#include "DangerComponent.h"
#include "SkyMapComponent.h"
#include "BaseEnemy.h"
#include "BaseGuardian.h"

#include "BaseSavingPoint.h"

#include "Runtime/Engine/Classes/Engine/World.h"
#include "Runtime/Engine/Public/TimerManager.h"
#include "Runtime/Engine/Classes/Engine/Canvas.h"
#include "UObject/ConstructorHelpers.h"
#include "Runtime/Engine/Classes/Kismet/KismetRenderingLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"

AEnlightPlayerController::AEnlightPlayerController()
{
	UE_LOG(LogTemp, Display, TEXT("EnlightPlayerController - Constructor - Begin"));

	static ConstructorHelpers::FObjectFinder<UCanvasRenderTarget2D> RenderTargetAsset(TEXT("CanvasRenderTarget2D'/Game/Enlight/SkyBox/Canvas_SkyMap.Canvas_SkyMap'"));
	if (RenderTargetAsset.Succeeded())
	{
		CanvasRenderTarget = RenderTargetAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UTexture> RenderTextureAsset(TEXT("Texture2D'/Game/Enlight/SkyBox/Icons/T_Star.T_Star'"));
	if (RenderTargetAsset.Succeeded())
	{
		DefaultStarTexture = RenderTextureAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UTexture> PlayerRenderTextureAsset(TEXT("Texture2D'/Game/Enlight/SkyBox/Icons/T_PlayerStar.T_PlayerStar'"));
	if (PlayerRenderTextureAsset.Succeeded())
	{
		PlayerStarTexture = PlayerRenderTextureAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UTexture> PlayerViewTextureAsset(TEXT("Texture2D'/Game/Enlight/SkyBox/Icons/T_PlayerRadius.T_PlayerRadius'"));
	if (PlayerViewTextureAsset.Succeeded())
	{
		PlayerViewRadiusTexture = PlayerViewTextureAsset.Object;
	}

	UE_LOG(LogTemp, Display, TEXT("EnlightPlayerController - Constructor - Finished"));
}

// Source: https://forums.unrealengine.com/development-discussion/audio/121233-audio-ears-on-the-character-not-the-camera
// Plays sound with attunation based on the Character Position and not on the Camera Position
void AEnlightPlayerController::GetAudioListenerPosition(FVector& OutLocation, FVector& OutFrontDir, FVector& OutRightDir)
{
	UE_LOG(LogTemp, Display, TEXT("EnlightPlayerController - GetAudioListenerPosition - Begin"));

	FVector ViewLocation;
	FRotator ViewRotation;

	if (bOverrideAudioListener)
	{
		USceneComponent* ListenerComponent = AudioListenerComponent.Get();
		if (ListenerComponent != nullptr)
		{
			ViewRotation = ListenerComponent->GetComponentRotation() + AudioListenerRotationOverride;
			ViewLocation = ListenerComponent->GetComponentLocation() + ViewRotation.RotateVector(AudioListenerLocationOverride);
		}
		else
		{
			ViewLocation = AudioListenerLocationOverride;
			ViewRotation = AudioListenerRotationOverride;
		}
	}
	else
	{
		GetPlayerViewPoint(ViewLocation, ViewRotation);
		if (GetPawn())
		{
			ViewLocation = GetPawn()->GetActorLocation();
		}
	}

	const FRotationTranslationMatrix ViewRotationMatrix(ViewRotation, ViewLocation);

	OutLocation = ViewLocation;
	OutFrontDir = ViewRotationMatrix.GetUnitAxis(EAxis::X);
	OutRightDir = ViewRotationMatrix.GetUnitAxis(EAxis::Y);

	UE_LOG(LogTemp, Display, TEXT("EnlightPlayerController - GetAudioListenerPosition - Finished"));
}

void AEnlightPlayerController::BeginPlay()
{
	UE_LOG(LogTemp, Display, TEXT("EnlightPlayerController - BeginPlay - Begin"));

	UWorld* World = GetWorld();
	APlayerCharacter* EnlightPlayer = Cast<APlayerCharacter>(GetPawn());

	if (World && EnlightPlayer)
	{
		TargetPercentage = EnlightPlayer->FogShownPercentage;

		// If the timer has expired or does not exist, start it  
		if (FogTimerHandle.IsValid() == false)
			GetWorld()->GetTimerManager().SetTimer(FogTimerHandle, this, &AEnlightPlayerController::FogUpdate, 1/ FogUpdatesPerSecond, true);
		else
		{
			if (World->GetTimerManager().IsTimerPaused(FogTimerHandle) == true)
				World->GetTimerManager().UnPauseTimer(FogTimerHandle);
			else
				World->GetTimerManager().PauseTimer(FogTimerHandle);
		}

		ClearSkyBox();
		
		SkyMapOverlappingUpdate(); //Call overlap on begin play so that overlapping actors exist in the first 5 seconds
		World->GetTimerManager().SetTimer(NavigationTimerHandle, this, &AEnlightPlayerController::SkyMapOverlappingUpdate, 5.0f, true);
	}

	UE_LOG(LogTemp, Display, TEXT("EnlightPlayerController - BeginPlay - Finished"));
}

void AEnlightPlayerController::Tick(float DeltaTime)
{
	UE_LOG(LogTemp, Display, TEXT("EnlightPlayerController - Tick - Begin"));

	Super::Tick(DeltaTime);
	UWorld* World = GetWorld();
	APlayerCharacter* EnlightPlayer = Cast<APlayerCharacter>(GetPawn());

	if (EnlightPlayer && IsValid(EnlightPlayer->FogSystem)
		&& World && World->GetTimerManager().IsTimerActive(NavigationUpdateTimerHandle))
	{
		if (EnlightPlayer->FogShownPercentage < TargetPercentage)
		{
			EnlightPlayer->FogShownPercentage = EnlightPlayer->FogShownPercentage
				+ FMath::Min(EnlightPlayer->FogLerpSpeed * DeltaTime,
					TargetPercentage - EnlightPlayer->FogShownPercentage);
		}
		else
		{
			EnlightPlayer->FogShownPercentage = EnlightPlayer->FogShownPercentage
				- FMath::Min(EnlightPlayer->FogLerpSpeed * DeltaTime,
				EnlightPlayer->FogShownPercentage - TargetPercentage);
		}
	}
	UE_LOG(LogTemp, Display, TEXT("EnlightPlayerController - Tick - Finished"));
	//UE_LOG(LogTemp, Warning, TEXT("FogPercentage: %f"), EnlightPlayer->FogShownPercentage);
}

void AEnlightPlayerController::SkyMapOverlappingUpdate()
{
	UE_LOG(LogTemp, Display, TEXT("EnlightPlayerController - SkyMapOverlappingUpdate - Begin"));

	UWorld* World = GetWorld();
	if (World)
	{
		APawn* PawnActor = GetPawn();
		APlayerCharacter* EnlightPlayer = nullptr;
		if (PawnActor)
		{
			EnlightPlayer = Cast<APlayerCharacter>(PawnActor);
		}
		UKismetSystemLibrary::SphereOverlapActors(World, EnlightPlayer->GetActorLocation(), EnlightPlayer->SkyMapRadius, EnlightPlayer->SkyMapObjectTypes, AActor::StaticClass(), ActorsToIgnore, OverlappingActors);
	}

	UE_LOG(LogTemp, Display, TEXT("EnlightPlayerController - SkyMapOverlappingUpdate - Finished"));
}

void AEnlightPlayerController::FogUpdate()
{
	UE_LOG(LogTemp, Display, TEXT("EnlightPlayerController - FogUpdate - Begin"));
	UWorld* World = GetWorld();
	APawn* PawnActor = GetPawn();
	APlayerCharacter* EnlightPlayer = nullptr;
	UNiagaraComponent* Fog = nullptr;
	if (PawnActor)
	{
		EnlightPlayer = Cast<APlayerCharacter>(PawnActor);
	}

	if (EnlightPlayer && World)
	{
		UE_LOG(LogTemp, Display, TEXT("EnlightPlayerController - FogUpdate - Update Radius"));
		//update radius
		Fog = EnlightPlayer->FogSystem;
		float Radius = FMath::Lerp(EnlightPlayer->MinFogRadius,
									EnlightPlayer->MaxFogRadius,
									(EnlightPlayer->HealthPoints / EnlightPlayer->MaxHealthPoints));
		
		if (bSonarImpuls)
		{
			float TimePassed = World->GetTimerManager().GetTimerRate(NavigationSonarTimerHandle)*SonarCalls
				+ World->GetTimerManager().GetTimerElapsed(NavigationSonarTimerHandle);

			float SonarRadius = FMath::Sin(TimePassed*EnlightPlayer->SonarImpulsSpeed * 2 * PI / EnlightPlayer->SonarIntervalTime) * EnlightPlayer->SonarImpulsRadius;
			if (SonarRadius < 0)
			{
				bSonarImpuls = false;
			}
			if (bSonarImpuls)
			{
				Radius += SonarRadius;
			}
		}

		// if the player is dead, the radius is handled by the PlayerCharacter
		if (EnlightPlayer->bPlayerIsDead)
		{
			Radius = EnlightPlayer->CurrentFogRadius;
		}

		float Volume = (4 * PI * pow(Radius, 3)) / 3;
		
		//UE_LOG(LogTemp, Warning, TEXT("Fog Radius: %f"), Radius);
		EnlightPlayer->CurrentFogRadius = Radius;
		Fog->SetNiagaraVariableFloat("User.Radius", Radius);
		Fog->SetNiagaraVariableFloat("User.Spawnrate", EnlightPlayer->FogParticleSpawnrate * (Radius / EnlightPlayer->MaxFogRadius));

		Fog->SetNiagaraVariableFloat("User.FogPercentage", EnlightPlayer->FogShownPercentage);
		//forcefully set the FogShowPercentage to TargetPercentage
		if (World->GetTimerManager().IsTimerActive(NavigationUpdateTimerHandle) && 
			!bNavigationActive &&
			TargetPercentage == 1.0f &&
			FMath::IsNearlyEqual(EnlightPlayer->FogShownPercentage, TargetPercentage, 0.001f))
		{
			ClearSkyBox();
			EnlightPlayer->FogShownPercentage = TargetPercentage;
		}

		UE_LOG(LogTemp, Display, TEXT("EnlightPlayerController - FogUpdate - Dangersorting"));

		/*multipile steps needed:
		1. Sort anything with dangercomponent for the closest actors
		2. determine the five closest actors
		3. hand those 5 actors to the fog system
		*/

		/// Step 1) Sorting and determining the closest actors
		TMultiMap<float, AActor*> DistanceMap;
		for (AActor* Actor : OverlappingActors)
		{
			if (IsValid(Actor))
			{
				//check whether actor is owner of skymap component and if so then reveal if in range
				USkyMapComponent* SkyMapComp = Cast<USkyMapComponent>(Actor->GetComponentByClass(USkyMapComponent::StaticClass()));
				UE_LOG(LogTemp, Display, TEXT("EnlightPlayerController - FogUpdate - Take 1"));
				if (IsValid(SkyMapComp))
				{
					//first check whether the actor should be revealed
					if (!SkyMapComp->bRevealed)
					{
						SkyMapComp->bRevealed = FVector::Distance(EnlightPlayer->GetActorLocation(), Actor->GetActorLocation()) < EnlightPlayer->CurrentFogRadius;
					}
				}
				//store actors that have UDangerComponent in a map with their distance
				UDangerComponent* DangerComponent = Cast<UDangerComponent>(Actor->GetComponentByClass(UDangerComponent::StaticClass()));
				UE_LOG(LogTemp, Display, TEXT("EnlightPlayerController - FogUpdate - Take 2"));
				if (IsValid(DangerComponent))
				{
					//only store actors that are within reach of player
					float Distance = FVector::Dist(Actor->GetActorLocation(), EnlightPlayer->GetActorLocation());
					if (Distance < Radius + DangerComponent->DangerRadius)
						DistanceMap.Add(Distance, Actor);
				}
			}
		}

		//sort actors using the keys in ascending order (1 < 2 < 5 < 9)
		DistanceMap.KeySort([](float A, float B) {
			return A < B;
		});


		/// Step 2) Closest 5 actors
		TArray<float> ClosestDangerActorsKeys;
		TArray<AActor*> ClosestDangerActorsValues;
		UE_LOG(LogTemp, Display, TEXT("EnlightPlayerController - FogUpdate - Take 3"));
		DistanceMap.GenerateKeyArray(ClosestDangerActorsKeys);
		DistanceMap.GenerateValueArray(ClosestDangerActorsValues);
		TArray<TTuple<float, AActor*>> ClosestDangerActors;

		for (int32 NumActor = 0; NumActor < 5; ++NumActor)
		{
			if (ClosestDangerActorsKeys.Num() - 1 < NumActor || ClosestDangerActorsValues.Num() - 1 < NumActor)
				ClosestDangerActors.Add(TTuple<float, AActor*>(0, nullptr));
			else
			{
				
				ClosestDangerActors.Add(TTuple<float, AActor*>(ClosestDangerActorsKeys[NumActor], ClosestDangerActorsValues[NumActor]));
			}
		}

		//sort the 5 danger actors by bIsDangerous priority
		// dangerous actors should be first in queue to have higher priority in fog system
		UE_LOG(LogTemp, Display, TEXT("EnlightPlayerController - FogUpdate - Take 5"));
		ClosestDangerActors.Sort([](TTuple<float, AActor*> At, TTuple<float, AActor*> Bt) {
			AActor* A = At.Value;
			AActor* B = Bt.Value;
			UDangerComponent* ADangerComponent = nullptr;
			UDangerComponent* BDangerComponent = nullptr;

			if(IsValid(A))
				ADangerComponent = A ? Cast<UDangerComponent>(A->GetComponentByClass(UDangerComponent::StaticClass())) : nullptr;
			
			if (IsValid(B))
				BDangerComponent = B ? Cast<UDangerComponent>(B->GetComponentByClass(UDangerComponent::StaticClass())) : nullptr;

			bool DangerA = false;
			bool DangerB = false;
			if (IsValid(ADangerComponent))
				DangerA = ADangerComponent->bIsDangerous;
			if (IsValid(BDangerComponent))
				DangerB = BDangerComponent->bIsDangerous;
			return DangerA || (!DangerA && !DangerB);
		});

		
		//now apply a class prioritized sorting to the closest 5 actors (i.e. bases could always be first)
		UE_LOG(LogTemp, Display, TEXT("EnlightPlayerController - FogUpdate - Take 8"));
		if (EnlightPlayer->FogClassPriority.Num() > 0)
		{
			ClosestDangerActors.Sort([this, EnlightPlayer](TTuple<float, AActor*> At, TTuple<float, AActor*> Bt) {
				for (int32 nActorPrio = 0; nActorPrio < EnlightPlayer->FogClassPriority.Num(); ++nActorPrio)
				{
					if (IsValid(At.Value) && At.Value->IsA(EnlightPlayer->FogClassPriority[nActorPrio]))
						return true;
					else if (IsValid(Bt.Value) && Bt.Value->IsA(EnlightPlayer->FogClassPriority[nActorPrio]))
						return false;
				}
				return true;
			});
		}

		UE_LOG(LogTemp, Display, TEXT("EnlightPlayerController - FogUpdate - Take 11"));
		bool bGuardianIsClose = ClosestDangerActors.ContainsByPredicate([this](TTuple<float, AActor*> At) {
			return IsValid(At.Value) && At.Value->IsA(ABaseGuardian::StaticClass());
		});

		//if the guardian is closeby force dangerpercentage to be 100%
		if (bGuardianIsClose)
		{
			UE_LOG(LogTemp, Display, TEXT("EnlightPlayerController - FogUpdate - Take 12"));
			EnlightPlayer->CurrentDangerPercentage = 1.0f;
			Fog->SetNiagaraVariableBool("User.ActivateVortex", true);
		}
		else
		{
			UE_LOG(LogTemp, Display, TEXT("EnlightPlayerController - FogUpdate - Take 13"));
			Fog->SetNiagaraVariableBool("User.ActivateVortex", false);
			//Danger Percentage
			float Part = (360.0f / EnlightPlayer->DangerPercentageSections);
			float Total = 0.0f;
			for (unsigned int nCurrentStep = 0; nCurrentStep < 8; ++nCurrentStep)
			{
				FVector CheckPoint = EnlightPlayer->GetActorForwardVector().RotateAngleAxis(Part*nCurrentStep, FVector(0.0f, 0.0f, 1.0f));
				CheckPoint.Normalize();
				CheckPoint = CheckPoint * Radius;
				for (int32 NumActor = 0; NumActor < 5; ++NumActor)
				{
					AActor* DangerActor = ClosestDangerActors[NumActor].Value;
					if (IsValid(DangerActor))
					{
						UDangerComponent* DangerComponent = Cast<UDangerComponent>(ClosestDangerActors[NumActor].Value->GetComponentByClass(UDangerComponent::StaticClass()));
						if (DangerComponent && DangerComponent->bIsDangerous)
						{
							//if this checkpoint is within the dangeractors radius
							if (FVector::DistSquared(EnlightPlayer->GetActorLocation() + CheckPoint, DangerActor->GetActorLocation()) < DangerComponent->DangerRadius * DangerComponent->DangerRadius)
							{
								Total += Part;
								break; //only one trigger per checkpoint is allowed
							}
						}
					}
				}
			}
			EnlightPlayer->CurrentDangerPercentage = Total / 360.0f;
			UE_LOG(LogTemp, Display, TEXT("EnlightPlayerController - FogUpdate - Take 16"));
		}

		/// Step 3) Update Niagara
		FString DangerPosition;
		FString DangerRadius;
		FString StartColor;
		FString EndColor;
		FString Turbulence;

		UE_LOG(LogTemp, Display, TEXT("EnlightPlayerController - FogUpdate - Take 17"));
		for (int32 NumActor = 0; NumActor < ClosestDangerActors.Num(); ++NumActor)
		{
			
			DangerPosition = "User.DangerPosition";
			DangerRadius = "User.DangerRadius";
			StartColor = "User.StartColor";
			EndColor = "User.EndColor";
			Turbulence = "User.Turbulence";

			if (ClosestDangerActors[NumActor].Value)
			{
				Fog->SetNiagaraVariableVec3(DangerPosition.Append(FString::FromInt(NumActor + 1)), ClosestDangerActors[NumActor].Value->GetActorLocation());
				
				UDangerComponent* DangerComponent = Cast<UDangerComponent>(ClosestDangerActors[NumActor].Value->GetComponentByClass(UDangerComponent::StaticClass()));
				if (DangerComponent)
				{
					Fog->SetNiagaraVariableFloat(DangerRadius.Append(FString::FromInt(NumActor + 1)), DangerComponent->DangerRadius);
					Fog->SetNiagaraVariableLinearColor(StartColor.Append(FString::FromInt(NumActor + 1)), DangerComponent->StartColor);
					Fog->SetNiagaraVariableLinearColor(EndColor.Append(FString::FromInt(NumActor + 1)), DangerComponent->EndColor);
					Fog->SetNiagaraVariableVec3(Turbulence.Append(FString::FromInt(NumActor + 1)), DangerComponent->Turbulence);
				}
			}
			else
			{
				//set position to playerlocation & radius to 0 so no particles are affected
				Fog->SetNiagaraVariableVec3(DangerPosition.Append(FString::FromInt(NumActor + 1)), EnlightPlayer->GetActorLocation());
				Fog->SetNiagaraVariableFloat(DangerRadius.Append(FString::FromInt(NumActor + 1)), 0.0f);
			}
		}
		
	}

	UE_LOG(LogTemp, Display, TEXT("EnlightPlayerController - FogUpdate - Finished"));
}


void AEnlightPlayerController::ActivateNavigation()
{
	UE_LOG(LogTemp, Display, TEXT("EnlightPlayerController - ActivateNavigation - Begin"));

	SonarCalls = 0;
	//allow navigation for X seconds 
	// X = defined in playercharacter
	APlayerCharacter* EnlightPlayer = Cast<APlayerCharacter>(GetPawn());

	TargetPercentage = 0.7f;
	bNavigationActive = true;

	UWorld* World = GetWorld();
	if (EnlightPlayer && World)
	{
		GetWorld()->GetTimerManager().SetTimer(NavigationUpdateTimerHandle, this, &AEnlightPlayerController::Navigation, 1 / NavigationUpdatesPerSecond, true);
		Sonar();
	}
}

void AEnlightPlayerController::DeactivateNavigation()
{	
	UE_LOG(LogTemp, Display, TEXT("EnlightPlayerController - DeactivateNavigation - Begin"));

	TargetPercentage = 1.0f;
	bNavigationActive = false;
}

void AEnlightPlayerController::Sonar()
{
	UWorld* World = GetWorld();
	if (bNavigationActive && World)
	{
		UE_LOG(LogTemp, Display, TEXT("EnlightPlayerController - Sonar - Begin"));

		SonarCalls++;

		APlayerCharacter* EnlightPlayer = Cast<APlayerCharacter>(GetPawn());
		if (IsValid(EnlightPlayer) && !EnlightPlayer->bPlayerIsDead)
		{
			// Send out an Sonar-Impuls, but it can't trigger faster than EnlightPlayer->SonarIntervalTime, if player spams it
			if (World->GetTimerManager().GetTimerRemaining(NavigationSonarCooldownTimerHandle) <= 0.0f)
			{
				for (AActor* Actor : OverlappingActors)
				{
					if (IsValid(Actor))
					{
						ABaseEnemy* Enemy = Cast<ABaseEnemy>(Actor);
						if (IsValid(Enemy))
							Enemy->OnSonarNavigation(EnlightPlayer);
					}
				}
				bSonarImpuls = true;
				EnlightPlayer->ReceiveOnSonarTriggered();
				World->GetTimerManager().SetTimer(NavigationSonarCooldownTimerHandle, EnlightPlayer->SonarIntervalTime, false);
			}

			World->GetTimerManager().SetTimer(NavigationSonarTimerHandle, this, &AEnlightPlayerController::Sonar, EnlightPlayer->SonarIntervalTime, false);
		}

		UE_LOG(LogTemp, Display, TEXT("EnlightPlayerController - Sonar - Finished"));
	}
}

void AEnlightPlayerController::Navigation()
{
	UE_LOG(LogTemp, Display, TEXT("EnlightPlayerController - Navigation - Begin"));

	APawn* PawnActor = GetPawn();
	APlayerCharacter* EnlightPlayer = nullptr;
	UWorld* World = GetWorld();
	if (PawnActor)
	{
		EnlightPlayer = Cast<APlayerCharacter>(PawnActor);
	}

	if (EnlightPlayer && CanvasRenderTarget && World && DefaultStarTexture)
	{
		UE_LOG(LogTemp, Display, TEXT("EnlightPlayerController - Navigation - Draw on Texture"));
		//Clear
		UKismetRenderingLibrary::ClearRenderTarget2D(World, CanvasRenderTarget);

		//Setup
		UCanvas* Canvas = nullptr;
		FDrawToRenderTargetContext RenderContext;
		FVector2D CanvasSize;
		UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(World, CanvasRenderTarget, Canvas, CanvasSize, RenderContext);


		//Player marker
		if (PlayerStarTexture)
		{
			FVector GlobalNorthVector(1, 0, 0);
			FVector PlayerForwardVector = EnlightPlayer->GetActorForwardVector();
			
			PlayerForwardVector.Normalize();
			GlobalNorthVector.Normalize();
			
			
			FVector EulerRotationOfPlayer = EnlightPlayer->GetActorRotation().Euler();
			
			FVector2D UVVector(0.5, 0.5);
			UVVector *= CanvasSize;
			Canvas->K2_DrawTexture(PlayerStarTexture,
				UVVector - FVector2D(10.0f, 10.0f),
				FVector2D(20.0f, 20.0f),
				FVector2D(0.0f, 0.0f),
				FVector2D::UnitVector,
				FLinearColor::Green ,
				BLEND_Translucent,
				-EulerRotationOfPlayer.Z);
			//Player view radius
			if (PlayerViewRadiusTexture)
			{	//viewradius texture is 512x512
				//rendertarget is 1024x1024 (CanvasSize)
				FVector2D TextureSize = FVector2D(512.0f, 512.0f) / 8;
				//currently packaging with UTexture.Source is not possible...
				//FVector2D TextureSize = FVector2D(PlayerViewRadiusTexture->Source.GetSizeX(), PlayerViewRadiusTexture->Source.GetSizeY()) / 4;
				TextureSize *= (EnlightPlayer->CurrentFogRadius/EnlightPlayer->MaxFogRadius);
				Canvas->K2_DrawTexture(PlayerViewRadiusTexture,
					UVVector - TextureSize /2,
					TextureSize,
					FVector2D(0.0f, 0.0f));
			}
				
		}

		//stars
		for (AActor* Actor : OverlappingActors)
		{
			if (IsValid(Actor))
			{
				USkyMapComponent* SkyMapComp = Cast<USkyMapComponent>(Actor->GetComponentByClass(USkyMapComponent::StaticClass()));
				if (SkyMapComp && SkyMapComp->bHidden == false)
				{
					UTexture* RenderTexture = DefaultStarTexture;
					if (SkyMapComp->bRevealed && SkyMapComp->RevealedTexture)
						RenderTexture = SkyMapComp->RevealedTexture;

					FVector TmpVector = EnlightPlayer->GetActorLocation() - Actor->GetActorLocation();
					FVector2D UVVector(TmpVector.Y, TmpVector.X);
					if (IsValid(EnlightPlayer->GlobalParameterCollection))
					{
						float SkyMapCameraWidth = 0.0f;
						EnlightPlayer->GlobalParameterCollection->GetScalarParameterValue(FName("CaptureWidth"), SkyMapCameraWidth);
						UVVector /= SkyMapCameraWidth;
					}
					else
					{
						UVVector /= EnlightPlayer->SkyMapRadius;
					}
					UVVector += FVector2D(0.5f, 0.5f);
					UVVector *= CanvasSize;

					FVector2D ActualDrawSize = (SkyMapComp->bRevealed && SkyMapComp->RevealedTexture) ? SkyMapComp->DrawSize * SkyMapComp->RevealedSizeMultiplier : SkyMapComp->DrawSize;
					Canvas->K2_DrawTexture(RenderTexture, UVVector - (ActualDrawSize / 2), ActualDrawSize, FVector2D(0.0f, 0.0f));

				}
			}
		}

		UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(World, RenderContext);
	}

	UE_LOG(LogTemp, Display, TEXT("EnlightPlayerController - Navigation - Finished"));
}

void AEnlightPlayerController::AnyDamage()
{
	//APlayerCharacter* Player = Cast<APlayerCharacter>(GetPawn());
	//if (Player)
	//{
	//	if (GEngine)
	//	{
	//		GEngine->AddOnScreenDebugMessage(
	//			-1,        // don't over wrire previous message, add a new one
	//			0.35f,   // Duration of message - limits distance messages scroll onto screen
	//			FColor::Cyan.WithAlpha(64),   // Color and transparancy!
	//			FString::Printf(TEXT("%s"), Player->HealthPoints)  // Our usual text message format
	//		);
	//	}
	//}
	
}

void AEnlightPlayerController::ClearSkyBox()
{
	UE_LOG(LogTemp, Display, TEXT("EnlightPlayerController - ClearSkyBox - Begin"));
	//Clear Skymap image
	UWorld* World = GetWorld();
	if (World)
	{
		UKismetRenderingLibrary::ClearRenderTarget2D(World, CanvasRenderTarget);
		//pause update timer
		if (NavigationUpdateTimerHandle.IsValid())
			World->GetTimerManager().PauseTimer(NavigationUpdateTimerHandle);

	}

	UE_LOG(LogTemp, Display, TEXT("EnlightPlayerController - ClearSkyBox - Finished"));
}
