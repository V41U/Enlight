// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/../../../Engine/Plugins/FX/Niagara/Source/Niagara/Public/NiagaraComponent.h"

#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "Runtime/Engine/Classes/Components/BoxComponent.h"
#include "Runtime/Engine/Classes/Components/TimelineComponent.h"
#include "Containers/Queue.h"
#include "NavigationSystem.h"

#include "Saveable.h"
#include "EnlightGameInstance.h"
#include "InteractorComponent.h"
#include "BaseGloaming.h"
#include "MonologStruct.h"
#include "GlobalMusicHandler.h"

#include "CoreMinimal.h"
#include "BuffableCharacter.h"
#include "Blueprint/UserWidget.h"
#include "PlayerCharacter.generated.h"

//forward declaration 
class ABaseGuardian;

/**
 *
 */
UCLASS()
class ENLIGHT_API APlayerCharacter : public ABuffableCharacter,
	public	ISaveable
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* FollowCamera;

public:
	APlayerCharacter();


	/** ATTENTION If you're doing something in this function:
		BE SURE THAT THIS IS NECESSARY
		Tick is extremely inefficient and fucks shit up
	*/
	virtual void Tick(float DeltaTime) override;

	virtual void OnHeal(float Heal, AActor* Healer = nullptr) override;
	virtual void OnDamage(float Damage, AActor* DamageDealer = nullptr) override;
	virtual void OnDeath(int RemainingLives, AActor* LastDamageDealer = nullptr) override;

	/**********************************************************************/
	/****************************ISAVEABLE*********************************/

	virtual void OnSaving_Implementation(UEnlightSaveGame* SaveObject) override;
	/**********************************************************************/
	// Called when the game starts
	virtual void BeginPlay() override;

	/** Called when BeginPlay is finished. Needed so that additional initializations can be done through blueprint*/
	UFUNCTION(BlueprintImplementableEvent)
		void ReceiveOnInitialize();
	UPROPERTY()
		bool bIsInitialized = false;

	UPROPERTY(BlueprintReadOnly)
		ABaseGuardian* Guardian;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
		float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
		float BaseLookUpRate;

	UPROPERTY(BlueprintReadWrite, Category = "Camera")
		bool bInvertCameraUpDown = false;

	UPROPERTY(BlueprintReadWrite, Category = "Camera")
		bool bInvertCameraLeftRight = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
		float CameraMinTargetArmLength = 400;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
		float CameraMaxTargetArmLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
		float CameraLerpSpeed = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
		float AngleTriggerForSkyMap = 5.0f;

	UFUNCTION()
		void UpdateCameraArm();

	/** Sphere for SkyMapping*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky Map")
		float SkyMapRadius = 25000.0f;

	//Overlap definitions for UKismetSystemLibrary::SphereOverlapActors
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky Map")
		TArray<TEnumAsByte<EObjectTypeQuery>> SkyMapObjectTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky Map")
		float SonarIntervalTime = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky Map")
		float SonarImpulsRadius = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky Map")
		int32 SonarImpulsSpeed = 1;

	//value between 0 and 1 specifying how much of the fog is shown
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky Map")
		float FogShownPercentage = 1.0f;
	//how fast is the change of fog percentages lerping
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky Map")
		float FogLerpSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance Notifier")
		float DistanceNotifierLoopsPerSecond = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance Notifier")
		float DistanceNotifierKillDistance = 1500.0f;

	//should the distancenotifier apply a permanent force or apply alterating between loop ticks?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance Notifier")
		bool bDistanceNotifierImpulseAlwaysActive = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance Notifier")
		float DistanceNotifierLoopsBeforeActiveChange = 10.0f;

	//reference to global music handler that is running in current level 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Music")
		AGlobalMusicHandler* GlobalMusic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
		float GlobalMusicVolumeModifier = 0.5f;

	UFUNCTION(BlueprintCallable)
		void ActivateBreadcrumbs(float Duration, float ParticleLifetime);

	void DeactivateBreadcrumbs();

	UFUNCTION(BlueprintCallable)
		void ShowMonolog(FMonologStruct monolog);

	UFUNCTION(BlueprintCallable)
		void AddQuestText(FName ID, FText QuestText);

	UFUNCTION(BlueprintCallable)
		void UpdateQuestText(FName ID, FText QuestText);

	UFUNCTION(BlueprintCallable)
		void RemoveQuestText(FName ID);

	/** Niagara System responsible for Fog */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Particles")
		UNiagaraComponent * FogSystem;

	/** Niagara System responsible for imitating the mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Particles")
		UNiagaraComponent* MeshImitator;

	/** Niagara System responsible for following character hands*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Particles")
		UNiagaraComponent* HandParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
		float MinFogRadius = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
		float MaxFogRadius = 1000.0f;
	
	/** Actors in this list have higher priority than anything else*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
		TArray<TSubclassOf<AActor>> FogClassPriority;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Particles")
		float CurrentFogRadius;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Particles")
		float FogParticleSpawnrate = 10000.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Particles")
		float CurrentDangerPercentage = 0.0f;
	//determines in how many sections the sphere is parted to check individually for dangers
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
		uint8 DangerPercentageSections = 8;

	/** Niagara System responsible for Wind*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Particles")
		UNiagaraComponent* WindParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
		float HandParticlesDefaultLifetime = 0.3f;

	/** Reference to the IngameMenu Widget Class */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
		TSubclassOf<class UUserWidget> IngameMenuClass;

	/** Reference to the Interaction Widget Class */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
		TSubclassOf<class UUserWidget> InteractionWidgetClass;

	/** Reference to the Monolog Widget Class */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
		TSubclassOf<class UUserWidget> MonologWidgetClass;

	/** Reference to the HUD Widget Class */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
		TSubclassOf<class UUserWidget> HUDWidgetClass;

	/** Variable to hold the HUD widget after creating it */
	UPROPERTY(BlueprintReadOnly, Category = "Widgets")
		UUserWidget* HUDWidget;

	/**Reference for how far a save point can be away from the player until it is no longer valid*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
		float MaxSavePointDistance = 400.0f;

	/** Reference to the AnimationBlueprint Class */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
		TSubclassOf<class UObject> AnimationBlueprintClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gloaming")
		float LaunchForce = 1000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gloaming")
		ABaseGloaming* OwnedGloaming;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gloaming")
		AActor* NotiferActor;


	//COMBAT

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
		bool bPlayerIsDead = false;

	UPROPERTY(BlueprintReadWrite, Category = "Combat")
		AActor* HomeBase;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		float DeadRespawnDistanceToHomeBase = 400.0f;

	UPROPERTY(EditAnywhere, Category = "Combat")
		float RespawnDelay = 5.0f;

	//dodging
	UPROPERTY(VisibleAnywhere, Category = "Combat")
		UTimelineComponent* DodgeTimeline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		UCurveFloat* DodgeLerpCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		float DashDistance = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		float DashDuration = 0.1f;

	//Block
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		bool bIsBlockingActivated = false;

	//Throw
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		bool bIsThrowingActivated = false;

	//is called from blueprint side so that the throwing starts
	UFUNCTION(BlueprintCallable, Category = "Combat")
		void ThrowGloaming();


	FVector DashStartPosition;
	FVector DashEndPosition;
	bool bCurrentlyDashing = false;

	UFUNCTION()
		void TimelineCallback(float Value);

	UFUNCTION()
		void TimelineFinishedCallback();

	UFUNCTION(BlueprintImplementableEvent)
		void ReceiveOnBlink();

	UFUNCTION(BlueprintImplementableEvent)
		void ReceiveOnDash();

	UFUNCTION(BlueprintImplementableEvent)
		void ReceiveOnDashOffCooldown();

	UFUNCTION(BlueprintImplementableEvent)
		void ReceiveOnThrow();

	UFUNCTION(BlueprintImplementableEvent)
		void ReceiveOnJump();

	UFUNCTION(BlueprintImplementableEvent)
		void ReceiveOnActivateNavigation();

	UFUNCTION(BlueprintImplementableEvent)
		void ReceiveOnDeactivateNavigation();
	
	UFUNCTION(BlueprintImplementableEvent)
		void ReceiveOnPlayerDeath();

	UFUNCTION(BlueprintImplementableEvent)
		void ReceiveOnPlayerRespawn();

	UFUNCTION(BlueprintImplementableEvent)
		void ReceiveOnOpenMenu();

	UFUNCTION(BlueprintImplementableEvent)
		void ReceiveOnSonarTriggered();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
		bool bMayDodge=true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		float DodgeCooldown = 1.0f;

protected:
	/** Called when Player wants to open the Ingame Menu */
	void OpenMenu();

	/** Called when Player wants to interact with something */
	void Interact();

	/** called when dash button is clicked. */
	void Dash();

	/** called when the dash cooldown is ready again. */
	void DashResetCooldown();

	/** called when the player wants to blink to the owned gloaming*/
	void Blink();

	/** The owned gloaming moves to the throwing position */
	void HoldGloaming();

	/** The owned gloaming is released from holding. */
	void DropHoldingGloaming();

	/** The owned gloaming will be launched away */
	void Throw();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** Called when jump is clicked and released*/
	//internally calls ACharacter::Jump but we can activate particles here
	void Jump();
	void StopJumping();

	void NavigationSpell();
	void NavigationSpellOver();

	bool bNotifierTick = true; //used for the alternating ticks
	int NotifierTicks;
	void DistanceNotifier();

	void ShowNextMonolog();

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	void TurnAtRateMouse(float Rate);

	void LookUpAtRateMouse(float Rate);

	/** Fog Kill Area Interaction Handling*/
	UFUNCTION()
		void OnCapsuleBeginOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** Fog Kill Area Interaction Handling*/
	UFUNCTION()
		void OnCapsuleEndOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UMaterialParameterCollectionInstance* GlobalParameterCollection;

private:
	/** Variable to hold the IngameMenu widget after creating it */
	UPROPERTY()
		UUserWidget* IngameMenu;
	/** Variable to hold the Interaction widget after creating it */
	UPROPERTY()
		UUserWidget* InteractionWidget;
	/** Variable to hold the Monolog widget after creating it */
	UPROPERTY()
		UUserWidget* MonologWidget;

	TQueue<FMonologStruct> MonologQueue;

	/** Variable that holds the InteractorComponent */
	UInteractorComponent* Interactor;

	float CameraTargetLength;

	UFUNCTION()
		void InteractionPossible(FText message);

	UFUNCTION()
		void InteractionNotPossibleAnymore(FText message);

	//TIMERS
	FTimerHandle DodgeCooldownHandle;
	FTimerHandle InitializationTimerHandle;
	FTimerHandle DistanceNotificatierLoopTimerHandle;
	FTimerHandle BreadcrumbTimerHandle;
	FTimerHandle MonologTimerHandle;
	FTimerHandle RespawnTimerHandle;
};