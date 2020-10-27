// Enlight Game 2019

#pragma once

#include "Engine/../../../Engine/Plugins/FX/Niagara/Source/Niagara/Public/NiagaraComponent.h"

#include "Components/SphereComponent.h"
#include "Runtime/Engine/Classes/Components/TimelineComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Saveable.h"

#include "CoreMinimal.h"
#include "BuffableCharacter.h"
#include "BaseGloaming.generated.h"

// depending on the attack type we can implement different behavior
UENUM(BlueprintType)
enum class EGloamingState :uint8
{
	EG_Idle			UMETA(DisplayName = "Idle"),
	EG_Curious		UMETA(DisplayName = "Curious"),
	EG_Scared		UMETA(DisplayName = "Scared"),
	EG_EnlightBase	UMETA(DisplayName = "EnlightBase")
};

/**
 * 
 */
UCLASS()
class ENLIGHT_API ABaseGloaming : public ABuffableCharacter,
								  public	ISaveable
{
	GENERATED_BODY()

public:
	ABaseGloaming();

	// PROPERTIES //////////////////////////////////////////

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gloaming")
		float DefaultMovementSpeed = 100.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gloaming")
		float ScaredMovementSpeed = 400.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gloaming")
		float CuriousMovementSpeed = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gloaming")
		float TargetFollowingDistance = 150.f;

	//by definition a gloaming is spawned as corrupted. the player must catch and attack it to defeat it's corruption
	// in order to set this to false use SetGloamingOwner with a valid actor
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gloaming")
		bool bCorrupted = true;

	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gloaming")
		USphereComponent* SensingSphere;

	/** Niagara System responsible for imitating the mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Particles")
		UNiagaraComponent* MeshImitator;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gloaming")
		float SensesPerSecond = 4.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gloaming")
		AActor* GloamingOwner = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gloaming")
		TArray<TSubclassOf<AActor>> StateActorPriority;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gloaming")
		float SacrificeHP = 250.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Throw")
		bool bThrowingState = false;

	UPROPERTY(VisibleAnywhere, Category = "Throw")
		UTimelineComponent* GloamingScalingTimeline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throw")
		UCurveFloat* GloamingScalingCurve;

	/** How long should the glowing be stunned after being thrown */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throw")
		float ThrowStunTime = 3.0f;

	/** How long a gloaming may be in the throwing state before being thrown. Afterwards should be moved to default state*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throw")
		float MaxAllowedTimeInThrowState = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throw")
		float ThrowCooldownTime = 5.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Throw")
		bool bTeleportable = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Throw")
		bool bMayBeThrown = true;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throw")
		float SacrificeProgress = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throw")
		float MaxSacrificeProgress = 100.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throw")
		float SacrificeProgressDecayPerSecond = 10.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throw")
		float SacrificeProgressGainPerSecond = 20.0f;


	UPROPERTY(BlueprintReadOnly, Category = "State")
		EGloamingState State;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
		EGloamingState StartingState = EGloamingState::EG_Idle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
		float MaxTimeAwayFromOwnerBeforeReset = 10.0f;

	// FUNCTIONS ///////////////////////////////////////
	
	//sets bCorrupted to false and sets the gloaming owner
	UFUNCTION(BlueprintCallable)
		bool SetGloamingOwner(AActor* NewOwner);

	/** Gloaming is resetting: 'delete' owner and be corrupted again after specified time */
	UFUNCTION(BlueprintCallable)
		void StartResetGloaming();

	/** Gloaming is resetted: 'delete' owner and be corrupted again */
	UFUNCTION(BlueprintCallable)
		void CancelResetGloaming();

	UFUNCTION(BlueprintImplementableEvent)
		void ReceiveOnGloamingReset();

	//allow gloamings to be preparing themselves before being thrown. call these before launching
	//TODO?: do launching within these functions?
	UFUNCTION(BlueprintCallable)
		void HoldGloaming();
	UFUNCTION(BlueprintCallable)
		void DropGloaming();
	UFUNCTION(BlueprintImplementableEvent)
		void ReceiveHoldGloaming();
	UFUNCTION(BlueprintCallable)
		void ThrowGloaming();
	UFUNCTION(BlueprintImplementableEvent)
		void ReceiveThrowGloaming();
	UFUNCTION()
		void OnThrowFinished();
	UFUNCTION(BlueprintImplementableEvent)
		void ReceiveThrowFinished();
	UFUNCTION()
		void OnThrowCanceled();
	UFUNCTION(BlueprintImplementableEvent)
		void ReceiveOnThrowCanceled();
	UFUNCTION()
		void StartThrowCooldown();
	UFUNCTION()
		void OnThrowCooldownOver();
	UFUNCTION(BlueprintImplementableEvent)
		void ReceiveOnThrowCooldownOver();

	UFUNCTION()
		void GloamingScalingTimelineCallback(float Value);
	UFUNCTION(BlueprintImplementableEvent)
		void ReceiveGloamingScalingTimelineCallback(float Value);

	UFUNCTION(BlueprintCallable)
		void Sacrifice();
	UFUNCTION(BlueprintImplementableEvent)
		void OnSacrifice(float SacrificeHealth);

	UFUNCTION(BlueprintCallable)
		void SetGloamingState(EGloamingState NewState = EGloamingState::EG_Idle);
	UFUNCTION(BlueprintImplementableEvent)
		void ReceiveStateChange(EGloamingState NewState);

	UFUNCTION(BlueprintCallable)
		void SenseSphere();
	//each gloaming child is allowed to handle their state on their own. they need to handle this event in order to handle the closest actor that is of interest
	UFUNCTION(BlueprintImplementableEvent)
		void ReceiveSenseSphere(AActor* NewClosestActor, float Distance);

	UFUNCTION(Category = "Gloaming")
		void OnSensingSphereBeginOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION(BlueprintImplementableEvent, Category = "Gloaming")
		void ReceiveOnSensingSphereBeginOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION(Category = "Gloaming")
		void OnSensingSphereEndOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	UFUNCTION(BlueprintImplementableEvent, Category = "Gloaming")
		void ReceiveOnSensingSphereEndOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gloaming")
		AActor* ClosestActor = nullptr;

	UFUNCTION(BlueprintCallable)
		bool EnlightBase(AActor* CorruptedBase);

	UFUNCTION(BlueprintImplementableEvent)
		void ReceiveOnEnlightBase(AActor* CorruptedBase);

	/** ATTENTION If you're doing something in this function:
		BE SURE THAT THIS IS NECESSARY
		Tick is extremely inefficient and fucks shit up
	*/
	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;

	/*********************************************************************/
	/***********************ABuffableCharacter****************************/
	virtual void OnHeal(float Heal, AActor* Healer = nullptr) override;
	virtual void OnDamage(float Damage, AActor* DamageDealer = nullptr) override;
	virtual void OnDeath(int RemainingLives, AActor* LastDamageDealer = nullptr) override;
	virtual void OnSuicide() override;
	/*********************************************************************/

	/**********************************************************************/
	/****************************ISAVEABLE*********************************/

	virtual void OnSaving_Implementation(UEnlightSaveGame* SaveObject) override;
	/**********************************************************************/
protected:
	
private:
	float InitialDefaultMoveSpeed;
	float InitialScaredMoveSpeed;

	FTimerHandle SensingTimerHandle;
	FTimerHandle ThrowStunTimerHandle;
	FTimerHandle ThrowCooldownTimerHandle;
	FTimerHandle ResetGloamingTimerHandle;
};
