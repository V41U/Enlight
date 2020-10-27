// Enlight Game 2019

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DangerComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ENLIGHT_API UDangerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDangerComponent();

	//determines whether the owner of this component is a hostile unit
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bIsDangerous = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float DangerRadius = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector Turbulence = FVector(0,0,2);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bIsDissolving = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bIsLightShown = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FColor StartColor = FColor::Red;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FColor EndColor = FColor::Blue;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
