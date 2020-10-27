// Enlight Game 2019

#include "InteractorComponent.h"

// Sets default values for this component's properties
UInteractorComponent::UInteractorComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UInteractorComponent::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void UInteractorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	bool bLookingAtInteractable = false;
	if (InteractablesInRange.Num() > 0) {
		// Iterate over all Interactables in Range
		// Test if we are looking at them (Raycast) - if so, show that the interaction with them is possible
		FHitResult HitResult;
		FVector CameraLoc;
		FRotator CameraRot;

		GetOwner()->GetActorEyesViewPoint(CameraLoc, CameraRot);
		FVector Start = CameraLoc;
		FVector End = CameraLoc + (CameraRot.Vector() * MaxInteractionRange);

		FCollisionQueryParams CollisionQuery = FCollisionQueryParams(FName(TEXT("InteractableComponent Trace")), true, NULL);
		CollisionQuery.AddIgnoredActor(GetOwner());
		CollisionQuery.bIgnoreTouches = false;

		if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_WorldStatic, CollisionQuery)) {
			// UE_LOG(LogTemp, Warning, TEXT("Hit something. %s"), *(hitResult.Actor->GetName()));
			for (UInteractableComponent* comp : InteractablesInRange) {
				if (HitResult.Actor == comp->GetOwner()) {
					bLookingAtInteractable = true;
					//if (CurrentInteractable != comp) {
						CurrentInteractable = comp;
						if (OnInteractionPossible.IsBound()) {
							OnInteractionPossible.Broadcast(CurrentInteractable->InteractMessage);
						}
					//}
					break;
				}
			}
		}
	}
	if (!bLookingAtInteractable && CurrentInteractable != nullptr) {
		if (OnInteractionNotPossibleAnymore.IsBound()) {
			OnInteractionNotPossibleAnymore.Broadcast(CurrentInteractable->InteractMessage);
		}
		CurrentInteractable = nullptr;
	}
}

void UInteractorComponent::InRange(UInteractableComponent* interactable) {
	InteractablesInRange.Add(interactable);
}

void UInteractorComponent::OutOfRange(UInteractableComponent* interactable) {
	InteractablesInRange.Remove(interactable);
}

void UInteractorComponent::Interact() {
	if (CurrentInteractable != nullptr && CurrentInteractable->bIsInteractable) {
		CurrentInteractable->Interact();
	}
}