// Enlight Game 2019

#include "BaseEnemy.h"

void ABaseEnemy::OnSonarNavigation(AActor* sonarSource) {
	if (FMath::RandRange(0.0f, 1.0f) <= ChanceToReactOnSonarNavigation)
	{
		OnSonarNavigationTriggered(sonarSource);
	}
}