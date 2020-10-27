// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Runtime/Engine/Classes/Engine/CanvasRenderTarget2D.h"

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EnlightPlayerController.generated.h"


/**
 * 
 */
UCLASS()
class ENLIGHT_API AEnlightPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AEnlightPlayerController();
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
		void ActivateNavigation();

	UFUNCTION()
		void DeactivateNavigation();


	bool bNavigationActive = false;

private:
	virtual void GetAudioListenerPosition(FVector & OutLocation, FVector & OutFrontDir, FVector & OutRightDir) override;
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//updates the positions of all POI and updates the fog system accordingly
	UFUNCTION()
		void FogUpdate();

	UFUNCTION()
		void Navigation();

	UFUNCTION()
		void SkyMapOverlappingUpdate();

	UFUNCTION()
		void Sonar();
	
	UFUNCTION()
		void AnyDamage();

	UFUNCTION()
		void ClearSkyBox();
	
	const TArray<AActor*> ActorsToIgnore;

	UPROPERTY()
	TArray<AActor*> OverlappingActors;
	
	// Fog radius depends on the starting max health of player
	float FogRadiusMultiplier;

	// Fog Updates
	float FogUpdatesPerSecond = 11.0f;
	FTimerHandle FogTimerHandle;

	// Sky Map Updates
	UCanvasRenderTarget2D* CanvasRenderTarget;
	UTexture* DefaultStarTexture;
	UTexture* PlayerViewRadiusTexture;;
	UTexture* PlayerStarTexture;
	float NavigationUpdatesPerSecond = 11.0f;
	FTimerHandle NavigationTimerHandle;
	FTimerHandle NavigationUpdateTimerHandle;
	FTimerHandle NavigationSonarTimerHandle;
	FTimerHandle NavigationSonarCooldownTimerHandle;
	float TargetPercentage;
	int SonarCalls = 0;
	bool bSonarImpuls = false;
};
