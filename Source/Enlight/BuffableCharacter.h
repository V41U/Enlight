// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Buff.h"
#include "Delegates/Delegate.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BuffableCharacter.generated.h"

UCLASS()
class ENLIGHT_API ABuffableCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABuffableCharacter();

	//Current Health Points
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Buffable)
		float HealthPoints;
	//Maximum Health Points
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Buffable)
		float MaxHealthPoints = 1000;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Buffable)
		bool bDead = false;
	//Can the actor currently receive damage?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Buffable)
		bool bInvulnerable = false;
	//how often can this actor "die" in order to be destroyed
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Buffable)
		int Lives = 1;
	//should the buffable character destroy itself if no lives are remaining?
	//ATTENTION: if this is false the child must destroy itself somehow
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Buffable)
		bool bAutoDestroyOnDeath = true;

	// BaseMovementspeed of the Character. Use SetBaseMovementSpeed to modify at runtime.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Buffable)
		float MovementSpeed = 600.0f;
	// Modifier that is multiplied with the BaseMovementSpeed to get the actual Speed. Changes due to Buffs during runtime.
	UPROPERTY(BlueprintReadWrite, Category = Buffable)
		float MovementSpeedModifier = 1.0f;

	UFUNCTION(BlueprintCallable)
		void Damage(float Damage, AActor* DamageDealer = nullptr);
	UFUNCTION(BlueprintCallable)
		void Heal(float Heal, AActor* Healer = nullptr);

	UFUNCTION(BlueprintImplementableEvent)
		void ReceiveOnHeal(float Heal, AActor* Healer = nullptr);
	virtual void OnHeal(float Heal, AActor* Healer = nullptr);

	UFUNCTION(BlueprintImplementableEvent)
		void ReceiveOnDamage(float Damage, AActor* DamageDealer = nullptr);
	virtual void OnDamage(float Damage, AActor* DamageDealer = nullptr);

	UFUNCTION(BlueprintCallable)
		void Suicide();
	UFUNCTION(BlueprintImplementableEvent)
		void ReceiveOnSuicide();
	virtual void OnSuicide();
	
	//ATTENTION: this is called BEFORE damage is applied to this actor. the reference will adapted and then the damage is applied 
	// like answer for https://answers.unrealengine.com/questions/57955/return-a-value-from-a-blueprintimplementableevent.html
	UFUNCTION(BlueprintImplementableEvent)
		void ReceiveBeforeDamageApplied(float &OutDamage, float InDamage = 0.0f, AActor* DamageDealer = nullptr);

	UFUNCTION(BlueprintImplementableEvent)
		void ReceiveOnDeath(int RemainingLives, AActor* LastDamageDealer = nullptr);
	//if all Lives have been consumed FinalDeath is called
	UFUNCTION(BlueprintImplementableEvent)
		void ReceiveOnFinalDeath(AActor* LastDamageDealer = nullptr);
	virtual void OnDeath(int RemainingLives, AActor* LastDamageDealer = nullptr);
	//last death 
	virtual void OnFinalDeath(AActor* LastDamageDealer = nullptr);

	// Sets the MovementSpeed * MovementSpeedModifier as the actual Movementspeed for the Movement Component.
	UFUNCTION(BlueprintCallable)
		void UpdateMovementSpeed();

	// Sets the MovementSpeed and calls UpdateMovementSpeed();
	UFUNCTION(BlueprintCallable)
		void SetBaseMovementSpeed(float newMovementSpeed);

	// Sets the MaxAcceleration of the Movement Component.
	UFUNCTION(BlueprintCallable)
		void SetMaxAcceleration(float newAcceleration);

	//BUFFS
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Buffable)
		TArray<ABuff*> Buffs;

	UFUNCTION(BlueprintCallable)
		ABuff* AddBuff(ABuff* Buff);

	int32 RemoveBuff(ABuff* Buff);

	//ENGINE SPECIFIC
		/////////////////////////////////////////////////////////////////////////////////////////////
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
