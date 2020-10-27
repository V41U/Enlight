// Enlight Game 2019

#include "InteractableComponent.h"
#include "InteractorComponent.h"

// Sets default values for this component's properties
UInteractableComponent::UInteractableComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PassiveRangeSphere = CreateDefaultSubobject<USphereComponent>(MakeUniqueObjectName(GetOwner(), GetClass(), FName("PassiveRangeComponent")));
	ActiveRangeSphere = CreateDefaultSubobject<USphereComponent>(MakeUniqueObjectName(GetOwner(), GetClass(), FName("ActiveRangeComponent")));
	InteractableRangeSphere = CreateDefaultSubobject<USphereComponent>(MakeUniqueObjectName(GetOwner(), GetClass(), FName("InteractableRangeComponent")));
}

// Called when the game starts
void UInteractableComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* owner = GetOwner();

	PassiveRangeSphere->RegisterComponent();
	PassiveRangeSphere->SetCollisionProfileName(TEXT("Trigger"));
	PassiveRangeSphere->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel2);//Interaction object channel
	PassiveRangeSphere->AttachToComponent(owner->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	PassiveRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &UInteractableComponent::PassiveEnter);
	PassiveRangeSphere->OnComponentEndOverlap.AddDynamic(this, &UInteractableComponent::PassiveExit);
	PassiveRangeSphere->SetSphereRadius(PassiveRange);

	ActiveRangeSphere->RegisterComponent();
	ActiveRangeSphere->SetCollisionProfileName(TEXT("Trigger"));
	ActiveRangeSphere->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel2);//Interaction object channel
	ActiveRangeSphere->AttachToComponent(owner->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	ActiveRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &UInteractableComponent::ActiveEnter);
	ActiveRangeSphere->OnComponentEndOverlap.AddDynamic(this, &UInteractableComponent::ActiveExit);
	ActiveRangeSphere->SetSphereRadius(ActiveRange);

	InteractableRangeSphere->RegisterComponent();
	InteractableRangeSphere->SetCollisionProfileName(TEXT("Trigger"));
	InteractableRangeSphere->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel2);//Interaction object channel
	InteractableRangeSphere->AttachToComponent(owner->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	InteractableRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &UInteractableComponent::InteractableEnter);
	InteractableRangeSphere->OnComponentEndOverlap.AddDynamic(this, &UInteractableComponent::InteractableExit);
	InteractableRangeSphere->SetSphereRadius(InteractRange);
}

void UInteractableComponent::PassiveEnter(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bHasPassiveEffect && OnEnterPassiveRange.IsBound())
	{
		OnEnterPassiveRange.Broadcast(OtherActor);
	}
}

void UInteractableComponent::PassiveExit(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (bHasPassiveEffect && OnExitPassiveRange.IsBound())
	{
		OnExitPassiveRange.Broadcast(OtherActor);
	}
}

void UInteractableComponent::ActiveEnter(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bHasActiveEffect && OnEnterActiveRange.IsBound())
	{
		OnEnterActiveRange.Broadcast(OtherActor);
	}
}

void UInteractableComponent::ActiveExit(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (bHasActiveEffect && OnExitActiveRange.IsBound())
	{
		OnExitActiveRange.Broadcast(OtherActor);
	}
}

void UInteractableComponent::InteractableEnter(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bIsInteractable) {
		UInteractorComponent* Interactor = OtherActor->FindComponentByClass<UInteractorComponent>();
		if (Interactor != nullptr) {
			CurrentInteractor = OtherActor;
			bInteractionPossible = true;
			Interactor->InRange(this);
		}
	}
}

void UInteractableComponent::InteractableExit(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UInteractorComponent* Interactor = OtherActor->FindComponentByClass<UInteractorComponent>();
	if (Interactor != nullptr) {
		bInteractionPossible = false;
		Interactor->OutOfRange(this);
		CurrentInteractor = nullptr;
	}
}

void UInteractableComponent::Interact() {
	if (bInteractionPossible && CurrentInteractor != nullptr && OnInteract.IsBound()) {
		OnInteract.Broadcast(CurrentInteractor);
	}
}

// Called every frame
void UInteractableComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}