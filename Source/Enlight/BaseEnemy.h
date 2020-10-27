// Enlight Game 2019

#pragma once

#include "CoreMinimal.h"
#include "Delegates/Delegate.h"
#include "BuffableCharacter.h"
#include "BaseEnemy.generated.h"

/**
 * 
 */
UCLASS()
class ENLIGHT_API ABaseEnemy : public ABuffableCharacter
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky Map")
		float ChanceToReactOnSonarNavigation = 0.1f;

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
		void OnBaseAttacked(AActor* attacker);

	UFUNCTION(BlueprintCallable)
		void OnSonarNavigation(AActor* sonarSource);
	UFUNCTION(BlueprintImplementableEvent)
		void OnSonarNavigationTriggered(AActor* sonarSource);
};
