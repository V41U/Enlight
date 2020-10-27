// Enlight Game 2019

#pragma once

#include "Buff.h"

#include "Components/SphereComponent.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Spell.generated.h"

UCLASS()
class ENLIGHT_API ASpell : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpell();

	// the sphere that is responsible for triggers
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision")
		USphereComponent* CollisionSphere;

	// buffs that will be applied
	UPROPERTY(EditAnywhere)
		TArray<TSubclassOf<ABuff>> AppliedBuffs;

	// how much damage the spell deals on affected actors
	UPROPERTY(EditAnywhere)
		float Damage;

	//How long the spell is allowed to be active.
	// if negative: stay 'forever'
	UPROPERTY(EditAnywhere)
		float UpTime;

	// actors that will be ignored when interacting
	UPROPERTY(EditAnywhere)
		TArray<AActor*> IgnoredActors;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
