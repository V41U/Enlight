// Enlight Game 2019

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SkyMapComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ENLIGHT_API USkyMapComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USkyMapComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UTexture* RevealedTexture = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bRevealed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector2D DrawSize = FVector2D(20.f, 20.f);

	// if the owner of this component has been revealed, resize it's icon in the sky for better visibility
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float RevealedSizeMultiplier = 2.0f;

	//defines whether this component is visible at all (does not work for player)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bHidden = false;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
