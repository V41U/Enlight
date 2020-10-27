// Fill out your copyright notice in the Description page of Project Settings.

#include "BuffableCharacter.h"

#include "Runtime/Engine/Public/EngineGlobals.h"
#include "Engine/World.h"

// Sets default values
ABuffableCharacter::ABuffableCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABuffableCharacter::BeginPlay()
{
	HealthPoints = MaxHealthPoints;
	UpdateMovementSpeed();

	Super::BeginPlay();
}


void ABuffableCharacter::Damage(float Damage, AActor* DamageDealer /*= nullptr*/)
{
	ReceiveBeforeDamageApplied(Damage, Damage, DamageDealer);

	if(!bInvulnerable)
		HealthPoints -= Damage;

	OnDamage(Damage, DamageDealer);
	ReceiveOnDamage(Damage, DamageDealer);
	
	if (HealthPoints <= 0 && !bDead)
	{
		Lives--;

		//remaining lives
		OnDeath(Lives, DamageDealer);
		ReceiveOnDeath(Lives, DamageDealer);

		if (Lives <= 0)
		{
			OnFinalDeath(DamageDealer);
			ReceiveOnFinalDeath(DamageDealer);

			bDead = true;

			if(bAutoDestroyOnDeath)
				Destroy();
		}
	}

}

void ABuffableCharacter::Heal(float Heal, AActor* Healer /*= nullptr*/)
{
	HealthPoints += Heal;

	if (HealthPoints > MaxHealthPoints)
	{
		Heal -= HealthPoints - MaxHealthPoints;
		HealthPoints = MaxHealthPoints;
	}

	//call event with actually healed amount
	OnHeal(Heal, Healer);
	ReceiveOnHeal(Heal, Healer);
}

void ABuffableCharacter::Suicide()
{
	OnSuicide();
	ReceiveOnSuicide();
	Damage(MaxHealthPoints, this);
}


void ABuffableCharacter::OnHeal(float Heal, AActor* Healer /*= nullptr*/)
{
	//empty
}

void ABuffableCharacter::OnDamage(float Damage, AActor* DamageDealer /*= nullptr*/)
{
	//empty
}

void ABuffableCharacter::OnSuicide()
{
	//empty
}

void ABuffableCharacter::OnDeath(int RemainingLives, AActor* LastDamageDealer /*= nullptr*/)
{
	//empty
}

void ABuffableCharacter::OnFinalDeath(AActor* LastDamageDealer /*= nullptr*/)
{
}

void ABuffableCharacter::UpdateMovementSpeed()
{
	GetCharacterMovement()->MaxWalkSpeed = this->MovementSpeed * this->MovementSpeedModifier;
}

void ABuffableCharacter::SetBaseMovementSpeed(float newMovementSpeed)
{
	this->MovementSpeed = newMovementSpeed;
	UpdateMovementSpeed();
}

void ABuffableCharacter::SetMaxAcceleration(float newAcceleration) {
	GetCharacterMovement()->MaxAcceleration = newAcceleration;
}

ABuff* ABuffableCharacter::AddBuff(ABuff* Buff)
{
	if (Buff)
	{
		Buff->SetOwner(this);

		switch (Buff->BuffApplyMode) {
		case ApplyMode::UNIQUE:
		case ApplyMode::UNIQUEPERSOURCE:
			for (auto ArrayBuff : Buffs)
			{
				if (ArrayBuff->GetClass() == Buff->GetClass())
				{
					if (Buff->BuffApplyMode == ApplyMode::UNIQUE || ArrayBuff->BuffSource == Buff->BuffSource)
					{
						ArrayBuff->EndBuff();
						break;
					}
				}
			}
			break;
		case ApplyMode::STACKABLE:
			// Just Add the Buff
			break;
		}
		//UE_LOG(LogTemp, Warning, TEXT("Adding Buff."));
		Buffs.Add(Buff);
		Buff->StartBuff();
	}

	return Buff;
}

int32 ABuffableCharacter::RemoveBuff(ABuff * Buff)
{
	return Buffs.Remove(Buff);
}

// Called every frame
void ABuffableCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ABuffableCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

