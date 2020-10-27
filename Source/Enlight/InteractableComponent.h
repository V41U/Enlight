// Enlight Game 2019

#pragma once

#include "CoreMinimal.h"
#include "Delegates/Delegate.h"
#include "Components/ActorComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"

#include "InteractableComponent.generated.h"

class UInteractorComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRangeSignature, AActor*, actor);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ENLIGHT_API UInteractableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UInteractableComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void Interact();

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FRangeSignature OnEnterPassiveRange;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FRangeSignature OnExitPassiveRange;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FRangeSignature OnEnterActiveRange;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FRangeSignature OnExitActiveRange;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FRangeSignature OnInteract;

	// Variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
	float PassiveRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
	float ActiveRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
	float InteractRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
	bool bHasPassiveEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
	bool bHasActiveEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
	bool bIsInteractable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact", Meta = (MultiLine = true))
	FText InteractMessage;


	UPROPERTY(BlueprintReadOnly, Category = "Interact")
		USphereComponent* PassiveRangeSphere;
	UPROPERTY(BlueprintReadOnly, Category = "Interact")
		USphereComponent* ActiveRangeSphere;
	UPROPERTY(BlueprintReadOnly, Category = "Interact")
		USphereComponent* InteractableRangeSphere;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	bool bInteractionPossible = false;
	AActor* CurrentInteractor;

	UFUNCTION()
	void PassiveEnter(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void PassiveExit(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void ActiveEnter(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void ActiveExit(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void InteractableEnter(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void InteractableExit(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
