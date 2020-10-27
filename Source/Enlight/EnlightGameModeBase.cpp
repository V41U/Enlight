// Fill out your copyright notice in the Description page of Project Settings.


#include "EnlightGameModeBase.h"

#include "UObject/ConstructorHelpers.h"

AEnlightGameModeBase::AEnlightGameModeBase()
{
	// use our custom PlayerController class
	PlayerControllerClass = AController::StaticClass();

	// set default pawn class to our Blueprinted character
	DefaultPawnClass = APawn::StaticClass();
	//static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Enlight/Character/BP_PlayerCharacter"));
	//if (PlayerPawnBPClass.Class != NULL)
	//{
	//	DefaultPawnClass = PlayerPawnBPClass.Class;
	//}
}