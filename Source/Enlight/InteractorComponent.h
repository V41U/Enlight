// Enlight Game 2019

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractableComponent.h"

#include "InteractorComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPossibleInteractSignature, FText, message);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ENLIGHT_API UInteractorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UInteractorComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void Interact();

	void InRange(UInteractableComponent* interactable);
	void OutOfRange(UInteractableComponent* interactable);

	// Variables
	float MaxInteractionRange = 1000.0f;

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FPossibleInteractSignature OnInteractionPossible;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FPossibleInteractSignature OnInteractionNotPossibleAnymore;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	UInteractableComponent* CurrentInteractable;
	TArray<UInteractableComponent*> InteractablesInRange;
};
