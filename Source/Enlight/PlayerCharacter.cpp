// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "PlayerCharacter.h"
//#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Runtime/Engine/Classes/Components/SkeletalMeshComponent.h"
#include "Runtime/Engine/Classes/Animation/AnimBlueprint.h"
#include "Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h"
#include "BaseSavingPoint.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "Runtime/Engine/Public/TimerManager.h"
#include "Components/CapsuleComponent.h"
#include "FogKillArea.h"
#include "EnlightPlayerController.h"
#include "NiagaraFunctionLibrary.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "SkyMapComponent.h"
#include "Kismet/GameplayStatics.h"
#include "BaseGuardian.h"

//DEBUG PLEASE REMOVE THIS BEFORE SHIPMENT!
#include "Engine/Engine.h"

//////////////////////////////////////////////////////////////////////////
// APlayerCharacter

APlayerCharacter::APlayerCharacter()
{
	UE_LOG(LogInit, Display, TEXT("Init PlayerCharacter."));
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &APlayerCharacter::OnCapsuleBeginOverlap);
	GetCapsuleComponent()->OnComponentEndOverlap.AddDynamic(this, &APlayerCharacter::OnCapsuleEndOverlap);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 600.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
	
	UE_LOG(LogInit, Warning, TEXT("Init CurveFloat."));
	//COMBAT TIMELINE
	static ConstructorHelpers::FObjectFinder<UCurveFloat> Curvy(TEXT("CurveFloat'/Game/Enlight/Particles/MeshImitator/FC_MeshLerp.FC_MeshLerp'"));
	check(Curvy.Succeeded());//TODO: Do research whether ctor is smarter with check-macro or an if() 
	DodgeLerpCurve = Curvy.Object;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named BP_Player (to avoid direct content references in C++)

	UE_LOG(LogInit, Warning, TEXT("Init SkeletalMesh."));
	//MESH
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SkeletalMeshAsset(TEXT("SkeletalMesh'/Game/Enlight/Meshes/Character/PlayerCharacter.PlayerCharacter'"));
	if (SkeletalMeshAsset.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(SkeletalMeshAsset.Object, false);
		//always update
		GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
		GetMesh()->SetRelativeLocation(FVector(0, 0, -95));
		GetMesh()->SetRelativeRotation(FRotator(0, 270, 0));
		GetMesh()->SetVisibility(false);
	}

	UE_LOG(LogInit, Warning, TEXT("Init AnimationBlueprint."));
	/*static ConstructorHelpers::FObjectFinder<UAnimBlueprint> AnimationAsset(TEXT("AnimBlueprint'/Game/Mannequin/Animations/ThirdPerson_AnimBP.ThirdPerson_AnimBP'"));
	if (AnimationAsset.Succeeded())
	{
		GetMesh()->SetAnimInstanceClass(AnimationAsset.Object->GeneratedClass);
	}*/
	GetMesh()->SetAnimInstanceClass(AnimationBlueprintClass);
	
	UE_LOG(LogInit, Warning, TEXT("Init FogSystem."));
	//NIAGARA
	FogSystem = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FogSystem"));
	FogSystem->SetupAttachment(RootComponent);

	UE_LOG(LogInit, Warning, TEXT("Init HandParticles."));
	HandParticles = CreateDefaultSubobject<UNiagaraComponent>(TEXT("HandParticles"));
	HandParticles->SetupAttachment(RootComponent);

	UE_LOG(LogInit, Warning, TEXT("Init MeshImitator."));
	MeshImitator = CreateDefaultSubobject<UNiagaraComponent>(TEXT("MeshImitator"));
	MeshImitator->SetupAttachment(RootComponent);

	UE_LOG(LogInit, Warning, TEXT("Init WindParticles."));
	WindParticles = CreateDefaultSubobject<UNiagaraComponent>(TEXT("WindParticles"));
	WindParticles->SetupAttachment(RootComponent);

	UE_LOG(LogInit, Warning, TEXT("Init Interactor."));
	Interactor = CreateDefaultSubobject<UInteractorComponent>(TEXT("Interactor"));
	AddOwnedComponent(Interactor);
	Interactor->OnInteractionPossible.AddDynamic(this, &APlayerCharacter::InteractionPossible);
	Interactor->OnInteractionNotPossibleAnymore.AddDynamic(this, &APlayerCharacter::InteractionNotPossibleAnymore);
}

void APlayerCharacter::InteractionPossible(FText message) {
	// Show the interaction widget with the message
	FStringOutputDevice OutputDevice;
	const FString command = FString::Printf(TEXT("ShowMessage \"%s\""), *(message.ToString()));
	InteractionWidget->CallFunctionByNameWithArguments(*command, OutputDevice, this, true);
}

void APlayerCharacter::InteractionNotPossibleAnymore(FText message) {
	// Hide the interaction widget
	FStringOutputDevice OutputDevice;
	const FString command = FString::Printf(TEXT("HideMessage \"%s\""), *(message.ToString()));
	InteractionWidget->CallFunctionByNameWithArguments(*command, OutputDevice, this, true);
}

void APlayerCharacter::ShowMonolog(FMonologStruct Monolog)
{
	if (Monolog.ShowingDuration > 0.0f)
	{
		MonologQueue.Enqueue(Monolog);

		UWorld* World = GetWorld();
		if (World)
		{
			if (!World->GetTimerManager().IsTimerActive(MonologTimerHandle))
			{
				ShowNextMonolog();
			}
		}
	}
}

void APlayerCharacter::ShowNextMonolog()
{
	UE_LOG(LogTemp, Display, TEXT("ShowNextMonolog"));
	FMonologStruct Monolog;
	if (MonologQueue.Dequeue(Monolog))
	{
		UWorld* World = GetWorld();
		if (World)
		{
			UE_LOG(LogTemp, Display, TEXT("ShowNextMonolog - PreAdjustVolume"));
			if (GlobalMusic && Monolog.SpokenMonologCue)
			{
				GlobalMusic->TimedMusicModification(GlobalMusicVolumeModifier, Monolog.SpokenMonologCue->Duration);
			}
			UE_LOG(LogTemp, Display, TEXT("ShowNextMonolog - PostAdjustVolume"));

			UE_LOG(LogTemp, Display, TEXT("ShowNextMonolog - PrePlaySound2DMonolog"));
			if(Monolog.SpokenMonologCue)
				UGameplayStatics::PlaySound2D(World, Monolog.SpokenMonologCue);
			UE_LOG(LogTemp, Display, TEXT("ShowNextMonolog - PreMonologWidget"));

			UE_LOG(LogTemp, Display, TEXT("ShowNextMonolog - ShowNextMonolog with Message %s"), *(Monolog.MonologText.ToString()));
			FStringOutputDevice OutputDevice;
			const FString command = FString::Printf(TEXT("ShowMessage \"%s\""), *(Monolog.MonologText.ToString()));
			
			UE_LOG(LogTemp, Display, TEXT("ShowNextMonolog - PreMonologWidget"));
			MonologWidget->CallFunctionByNameWithArguments(*command, OutputDevice, this, true);
			UE_LOG(LogTemp, Display, TEXT("ShowNextMonolog - PostMonologWidget"));

			World->GetTimerManager().SetTimer(MonologTimerHandle, this, &APlayerCharacter::ShowNextMonolog, Monolog.ShowingDuration, false);
		}
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("ShowNextMonolog - HideMessage"));
		FStringOutputDevice OutputDevice;
		const FString command = FString::Printf(TEXT("HideMessage"));
		MonologWidget->CallFunctionByNameWithArguments(*command, OutputDevice, this, true);
	}
}

void APlayerCharacter::AddQuestText(FName ID, FText QuestText)
{
	FStringOutputDevice OutputDevice;
	const FString command = FString::Printf(TEXT("AddQuestLine \"%s\" \"%s\""), *(ID.ToString()),*(QuestText.ToString()));
	HUDWidget->CallFunctionByNameWithArguments(*command, OutputDevice, this, true);
}

void APlayerCharacter::UpdateQuestText(FName ID, FText QuestText)
{
	FStringOutputDevice OutputDevice;
	const FString command = FString::Printf(TEXT("UpdateQuestLine \"%s\" \"%s\""), *(ID.ToString()), *(QuestText.ToString()));
	HUDWidget->CallFunctionByNameWithArguments(*command, OutputDevice, this, true);
}

void APlayerCharacter::RemoveQuestText(FName ID)
{
	FStringOutputDevice OutputDevice;
	const FString command = FString::Printf(TEXT("RemoveQuestLine \"%s\""), *(ID.ToString()));
	HUDWidget->CallFunctionByNameWithArguments(*command, OutputDevice, this, true);
}

void APlayerCharacter::Tick(float DeltaTime)
{
	UE_LOG(LogTemp, Display, TEXT("PlayerCharacter - Tick - Begin"));

	Super::Tick(DeltaTime);

	//continuously update the camera position for fog evasion
	if (FogSystem)
		FogSystem->SetNiagaraVariableVec3("User.CameraPosition", FollowCamera->GetComponentLocation());
	
	//Update camera distance continuously so that player does not notice the slow adaptation in fog
	CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, CameraTargetLength, DeltaTime, CameraLerpSpeed);
	

	//better: change material in a way that it's using opacity oder something 
	if (MeshImitator && HandParticles)
	{
		MeshImitator->SetNiagaraVariableFloat("User.HPPercentage", (HealthPoints / MaxHealthPoints));
		float CameraAngleY = FollowCamera->GetComponentRotation().Euler().Y;

		if (CameraAngleY > 45.0f) {
			// Activate Navigation
			NavigationSpell();
		}
		else {
			// Deactivate Navigation
			NavigationSpellOver();
		}

		if (CameraAngleY > AngleTriggerForSkyMap)
		{
			//normalize between 1-0
			float NormalizedAngleValue = (CameraAngleY - AngleTriggerForSkyMap) / (57.0f - AngleTriggerForSkyMap);
			if (NormalizedAngleValue < 0.0f) {
				NormalizedAngleValue = 0.0f;
			}
			if (NormalizedAngleValue > 0.96f) {
				NormalizedAngleValue = 0.96f;
			}

			NormalizedAngleValue = 1 - NormalizedAngleValue;
			//looking up
			MeshImitator->SetNiagaraVariableFloat("User.SizeMultiplier", NormalizedAngleValue);
			MeshImitator->SetNiagaraVariableFloat("User.NoiseLerp", NormalizedAngleValue);
			HandParticles->SetNiagaraVariableFloat("User.CameraAngleEffectMultiplier", NormalizedAngleValue - 0.03f);

			if (IsValid(OwnedGloaming))
				OwnedGloaming->MeshImitator->SetNiagaraVariableFloat("User.SizeMultiplier", NormalizedAngleValue);
		}
		else
		{
			//not looking up'
			MeshImitator->SetNiagaraVariableFloat("User.SizeMultiplier", 1.0f);
			MeshImitator->SetNiagaraVariableFloat("User.NoiseLerp", 1.0f);
			HandParticles->SetNiagaraVariableFloat("User.CameraAngleEffectMultiplier", 1.0f);

			if (IsValid(OwnedGloaming))
				OwnedGloaming->MeshImitator->SetNiagaraVariableFloat("User.SizeMultiplier", 1.0f);
		}
	}

	// update materialparametercollection values so fog materials will be updated
	if (GlobalParameterCollection)
	{
		GlobalParameterCollection->SetScalarParameterValue("CurrentFogRadius", CurrentFogRadius);
		GlobalParameterCollection->SetVectorParameterValue("PlayerLocation", GetActorLocation());
	}

	// Dash
	if (bCurrentlyDashing)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			float ElapsedTime = World->GetTimerManager().GetTimerElapsed(DodgeCooldownHandle);
			float Ratio = FMath::Min(ElapsedTime / DashDuration, 1.0f);
			SetActorLocation(DashStartPosition * (1 - Ratio) + DashEndPosition * Ratio);
			if (Ratio >= 1.0f)
				bCurrentlyDashing = false;
		}
	}
	
	// If the player is dead, fog "devours" the player
	if (bPlayerIsDead)
	{
		UWorld* World = GetWorld();
		if (World)
			//CurrentFogRadius = (1 - World->GetTimerManager().GetTimerElapsed(RespawnTimerHandle) / RespawnDelay) * MinFogRadius;
			CurrentFogRadius = (1 - World->GetTimerManager().GetTimerElapsed(RespawnTimerHandle) / RespawnDelay) * MinFogRadius * 4.0f / 5.0f + MinFogRadius / 5.0f;
	}

	UE_LOG(LogTemp, Display, TEXT("PlayerCharacter - Tick - Finished"));
}

void APlayerCharacter::OnHeal(float Heal, AActor* Healer/* = nullptr*/)
{
	if (bPlayerIsDead)
	{
		HealthPoints -= Heal;
	}
	//Handle Heal of Player
	UpdateCameraArm();
}

void APlayerCharacter::OnDamage(float Damage, AActor* DamageDealer /*= nullptr*/)
{
	//Handle Damage of Player
	UpdateCameraArm();
}

void APlayerCharacter::OnDeath(int RemainingLives, AActor* LastDamageDealer /*= nullptr*/)
{
	UE_LOG(LogTemp, Display, TEXT("PlayerCharacter - OnDeath - Begin"));

	//Handle Death of Player
	if (RemainingLives <= 0) {
		Lives = 1;
		bPlayerIsDead = true;
		bInvulnerable = true;
		HealthPoints = 0.1f;
		DeactivateBreadcrumbs();
		if (IsValid(OwnedGloaming))
		{
			OwnedGloaming->Suicide();
		}
		ReceiveOnPlayerDeath();

		UWorld* World = GetWorld();
		if (World && HomeBase)
		{
			FTimerDelegate TimerCallback;
			TimerCallback.BindLambda([this]()
			{
				// Port the player to the HomeBase and respawn
				FNavLocation NavPoint;
				UNavigationSystemV1* NavSystem = Cast<UNavigationSystemV1>(GetWorld()->GetNavigationSystem());
				//temp fix so that respawning works
				//if (NavSystem->GetRandomPointInNavigableRadius(HomeBase->GetActorLocation(), DeadRespawnDistanceToHomeBase, NavPoint))
				//	SetActorLocation(NavPoint.Location);
				//else
					SetActorLocation(HomeBase->GetActorLocation() + FVector(0.0f, 0.0f, 50.0f));
				bPlayerIsDead = false;
				bInvulnerable = false;
				HealthPoints = MaxHealthPoints;
				ReceiveOnPlayerRespawn();
			});
			World->GetTimerManager().SetTimer(RespawnTimerHandle, TimerCallback, RespawnDelay, false);
		}
	}

	UE_LOG(LogTemp, Display, TEXT("PlayerCharacter - OnDeath - Finished"));
}

void APlayerCharacter::OnSaving_Implementation(UEnlightSaveGame* SaveObject)
{
	// Store data here!
	//FTransform ControlTansform = FTransform(GetControlRotation(), GetActorLocation(), GetActorScale3D());
	//SaveObject->PlayerTransform = ControlTansform;

	SaveObject->PlayerTransform = GetActorTransform();
	SaveObject->CameraRotation = GetControlRotation();
}

// Called when the game starts
void APlayerCharacter::BeginPlay()
{
	UE_LOG(LogTemp, Display, TEXT("PlayerCharacter - BeginPlay - Begin"));

	Super::BeginPlay();

	CurrentFogRadius = MaxFogRadius;

	if (MeshImitator != nullptr)
	{
		//WARNING: For some reason we have to 'timely' reset the MeshImitator
		// Here we assume some small time and then reactivate the particle system
		// Don't ask why just accept that it works like this. 
		//If you have a better solution feel free to do so :)
		FTimerDelegate TimerCallback;
		TimerCallback.BindLambda([this]()
		{
			//MeshImitator->Activate(true);
			ReceiveOnInitialize();
		});
		UWorld* World = GetWorld();
		if (World)
			World->GetTimerManager().SetTimer(InitializationTimerHandle, TimerCallback, 0.1f, false);

	}

	const UMaterialParameterCollection* MaterialParameterInst = Cast<UMaterialParameterCollection>(StaticLoadObject(UMaterialParameterCollection::StaticClass(),
																													NULL,
																													TEXT("MaterialParameterCollection'/Game/Enlight/Materials/GLOBAL_MATERIAL_PARAMS.GLOBAL_MATERIAL_PARAMS'")));
	if (MaterialParameterInst)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			GlobalParameterCollection = World->GetParameterCollectionInstance(MaterialParameterInst);
		}
	}

	//define a start point for the camera 
	CameraTargetLength = CameraBoom->TargetArmLength;


	//DODGE TIMELINE SETUP
	// ref: https://wiki.unrealengine.com/Timeline_in_c%2B%2B#How_to_use_Timeline_in_c.2B.2B
	//   &: https://answers.unrealengine.com/questions/384597/link-to-timelines-doc-for-c.html
	FOnTimelineFloat onTimelineCallback;
	FOnTimelineEventStatic onTimelineFinishedCallback;
	if (DodgeLerpCurve)
	{
		DodgeTimeline = NewObject<UTimelineComponent>(this, TEXT("DodgeTimeline"));
		DodgeTimeline->CreationMethod = EComponentCreationMethod::UserConstructionScript; // Indicate it comes from a blueprint so it gets cleared when we rerun construction scripts
		DodgeTimeline->SetLooping(false); //non looping
		this->BlueprintCreatedComponents.Add(DodgeTimeline); // Add to array so it gets saved
		DodgeTimeline->SetNetAddressable(); // This component has a stable name that can be referenced for replication

		DodgeTimeline->SetLooping(false);
		DodgeTimeline->SetTimelineLength(5.0f);
		DodgeTimeline->SetTimelineLengthMode(ETimelineLengthMode::TL_LastKeyFrame);

		DodgeTimeline->SetPlaybackPosition(0.0f, false);

		onTimelineCallback.BindUFunction(this, FName{ TEXT("TimelineCallback") });
		onTimelineFinishedCallback.BindUFunction(this, FName{ TEXT("TimelineFinishedCallback") });

		DodgeTimeline->AddInterpFloat(DodgeLerpCurve, onTimelineCallback);
		DodgeTimeline->SetTimelineFinishedFunc(onTimelineFinishedCallback);

		DodgeTimeline->RegisterComponent();
	}

	CameraMaxTargetArmLength = CameraBoom->TargetArmLength;

	if (HandParticles)
	{
		int32 RBone = GetMesh()->GetBoneIndex("pointer2_R");
		int32 LBone = GetMesh()->GetBoneIndex("pointer2_L");
		if ((RBone != INDEX_NONE) && (LBone != INDEX_NONE))
		{
			HandParticles->SetNiagaraVariableInt("User.RBone", GetMesh()->GetBoneIndex("pointer2_R"));
			HandParticles->SetNiagaraVariableInt("User.LBone", GetMesh()->GetBoneIndex("pointer2_L"));
			HandParticles->SetNiagaraVariableFloat("User.Lifetime", HandParticlesDefaultLifetime);

		}
	}
	UE_LOG(LogTemp, Display, TEXT("PlayerCharacter - BeginPlay - Creating Widgets"));
	IngameMenu = CreateWidget<UUserWidget>(Cast<APlayerController>(GetController()), IngameMenuClass);
	InteractionWidget = CreateWidget<UUserWidget>(Cast<APlayerController>(GetController()), InteractionWidgetClass);
	InteractionWidget->AddToViewport();

	MonologWidget = CreateWidget<UUserWidget>(Cast<APlayerController>(GetController()), MonologWidgetClass);
	MonologWidget->AddToViewport();

	HUDWidget = CreateWidget<UUserWidget>(Cast<APlayerController>(GetController()), HUDWidgetClass);
	HUDWidget->AddToViewport();

	UEnlightGameInstance* GameInstance = Cast<UEnlightGameInstance>(GetGameInstance());
	if (GameInstance)
	{
		if (GameInstance->bNewGame)
		{
			// #####################
			// Completely Fresh Game
			// #####################

		}
		else
		{
			// ###########
			// Loaded Game
			// ###########

			//SetActorLocation(GameInstance->SaveGame->PlayerTransform.GetLocation());
			//SetActorScale3D(GameInstance->SaveGame->PlayerTransform.GetScale3D());
			//GetController()->SetControlRotation(GameInstance->SaveGame->PlayerTransform.Rotator());

			SetActorTransform(GameInstance->SaveGame->PlayerTransform);
			GetController()->SetControlRotation(GameInstance->SaveGame->CameraRotation);
		}
	}
	
	//start distance notifier for continuous help towards gloamings
	DistanceNotifier();

	if (Guardian == nullptr)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			TArray<AActor*> AllGuardians;
			UGameplayStatics::GetAllActorsOfClass(World, ABaseGuardian::StaticClass(), AllGuardians);
			check(AllGuardians.Num() == 1 && IsValid(AllGuardians[0]))
				Guardian = Cast<ABaseGuardian>(AllGuardians[0]);	
		}
	}

	if (GlobalMusic == nullptr)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			TArray<AActor*> AllMusicHandlers;
			UGameplayStatics::GetAllActorsOfClass(World, AGlobalMusicHandler::StaticClass(), AllMusicHandlers);
			check(AllMusicHandlers.Num() == 1 && IsValid(AllMusicHandlers[0]))
				GlobalMusic = Cast<AGlobalMusicHandler>(AllMusicHandlers[0]);
		}
	}


	// Reset Mouse and InputMode, so that it works after showing some UI
	Cast<APlayerController>(GetController())->bShowMouseCursor = false;
	Cast<APlayerController>(GetController())->SetInputMode(FInputModeGameOnly());

	UE_LOG(LogTemp, Display, TEXT("PlayerCharacter - BeginPlay - Finished"));
}

void APlayerCharacter::UpdateCameraArm()
{
	if (bPlayerIsDead)
		CameraTargetLength = CameraMinTargetArmLength;
	else
		CameraTargetLength = FMath::Lerp(CameraMinTargetArmLength,
										CameraMaxTargetArmLength,
										(HealthPoints / MaxHealthPoints));
}

void APlayerCharacter::ActivateBreadcrumbs(float Duration, float ParticleLifetime)
{
	if (!bPlayerIsDead && HandParticles)
	{	
		UWorld* World = GetWorld();
		if (World)
		{
			if (World->GetTimerManager().IsTimerActive(BreadcrumbTimerHandle))
			{
				// Breadcrumbs are currently active - stop the current one
				DeactivateBreadcrumbs();
			}
			World->GetTimerManager().SetTimer(BreadcrumbTimerHandle, this, &APlayerCharacter::DeactivateBreadcrumbs, Duration, false);
			HandParticles->SetNiagaraVariableFloat("User.Lifetime", ParticleLifetime);
		}
	}
}

void APlayerCharacter::DeactivateBreadcrumbs() {
	if (HandParticles)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			World->GetTimerManager().ClearTimer(BreadcrumbTimerHandle);
		}

		HandParticles->SetNiagaraVariableFloat("User.Lifetime", HandParticlesDefaultLifetime);
	}
}

void APlayerCharacter::DistanceNotifier()
{
	UE_LOG(LogTemp, Display, TEXT("PlayerCharacter - DistanceNotifier - Begin"));

	UWorld* World = GetWorld();
	TArray<AActor*> AllGloamings;
	UGameplayStatics::GetAllActorsOfClass(World, ABaseGloaming::StaticClass(), AllGloamings);

	//sort ascending by distance
	FVector OwnLocation = GetActorLocation();
	AllGloamings.Sort([OwnLocation](const AActor& A, const AActor& B) {
		float DistanceToA = FVector::DistSquared(OwnLocation, A.GetActorLocation());
		float DistanceToB = FVector::DistSquared(OwnLocation, B.GetActorLocation());
		return DistanceToA < DistanceToB;
	});

	if (AllGloamings.Num() > 0)
		NotiferActor = AllGloamings[0];
	else
		NotiferActor = nullptr;

	if (World)
		World->GetTimerManager().SetTimer(DistanceNotificatierLoopTimerHandle, this, &APlayerCharacter::DistanceNotifier, 1 / DistanceNotifierLoopsPerSecond, false);

	UE_LOG(LogTemp, Display, TEXT("PlayerCharacter - DistanceNotifier - Finished"));
}


//////////////////////////////////////////////////////////////////////////
// Input

void APlayerCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &APlayerCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &APlayerCharacter::StopJumping);

	PlayerInputComponent->BindAction("Dash", IE_Pressed, this, &APlayerCharacter::Dash);
	PlayerInputComponent->BindAction("Throw", IE_Pressed, this, &APlayerCharacter::Throw);
	PlayerInputComponent->BindAction("HoldGloaming", IE_Pressed, this, &APlayerCharacter::HoldGloaming);
	PlayerInputComponent->BindAction("HoldGloaming", IE_Released, this, &APlayerCharacter::DropHoldingGloaming);
	PlayerInputComponent->BindAction("Blink", IE_Pressed, this, &APlayerCharacter::Blink);

	PlayerInputComponent->BindAction("Menu", IE_Pressed, this, &APlayerCharacter::OpenMenu);
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &APlayerCharacter::Interact);

	//PlayerInputComponent->BindAction("NavigationSpell", IE_Pressed, this, &APlayerCharacter::NavigationSpell);
	//PlayerInputComponent->BindAction("NavigationSpell", IE_Released, this, &APlayerCharacter::NavigationSpellOver);

	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APlayerCharacter::TurnAtRateMouse);
	PlayerInputComponent->BindAxis("TurnRate", this, &APlayerCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APlayerCharacter::LookUpAtRateMouse);
	PlayerInputComponent->BindAxis("LookUpRate", this, &APlayerCharacter::LookUpAtRate);
}

void APlayerCharacter::Dash()
{
	UE_LOG(LogTemp, Display, TEXT("PlayerCharacter - Dash - Begin"));

	//only allow dodge if a jump would be possible and if the player is moving
	if (!bPlayerIsDead && DodgeTimeline != nullptr
		&& MeshImitator
		//&& CanJump() debatable? should a player only dodge if they are on the ground?
		&& bMayDodge)
	{
		bCurrentlyDashing = true;
		uint8 DashLoops = 0;
		DashStartPosition = GetActorLocation();
		bool bDashOnGround = GetMovementComponent()->IsMovingOnGround();
		float DashLoopDistance = DashDistance;
		FVector DashLoopStart;

		FHitResult SweepResult;
		while (DashLoops < 3 && DashLoopDistance > 0.0f)
		{
			DashLoopStart = GetActorLocation();
			SetActorLocation(DashLoopStart + GetActorForwardVector()*DashLoopDistance, true, &SweepResult);
			if (!bDashOnGround)
				break;

			float normalDot = FVector::DotProduct(SweepResult.ImpactNormal, FVector(0.0f, 0.0f, 1.0f));
			if (normalDot > 0.6f) {
				SetActorLocation(DashLoopStart, false, nullptr, ETeleportType::TeleportPhysics);
				FVector OffsetHitPosition = SweepResult.Location + SweepResult.ImpactNormal * FMath::Min(SweepResult.Distance, 3.0f);
				FVector NewDashDirection = OffsetHitPosition - DashLoopStart;
				NewDashDirection.Normalize();
				SetActorLocation(DashLoopStart + NewDashDirection * DashLoopDistance, true);
			}
			// Set the player a bit back onto the ground
			SetActorLocation(GetActorLocation() - FVector(0.0f, 0.0f, DashLoopDistance / 2.0f), true);
			DashLoops++;
			DashLoopDistance = DashDistance - (GetActorLocation() - DashStartPosition).Size();
		}
		// UE_LOG(LogTemp, Warning, TEXT("Dash Distance: %f"), (GetActorLocation()- DashStartPosition).Size());

		// Save the final Position, Teleport back and interpolate the position based on DashDuration (in Tick)
		DashEndPosition = GetActorLocation();
		SetActorLocation(DashStartPosition, false, nullptr, ETeleportType::TeleportPhysics);

		/*
		FTimerDelegate TimerCallback;
		TimerCallback.BindLambda([this]()
		{
			if (IsValid(this))
			{
				bMayDodge = true;
				ReceiveOnDashOffCooldown();
			}
		});
		*/
		bMayDodge = false;

		UWorld* World = GetWorld();
		if (World)
			World->GetTimerManager().SetTimer(DodgeCooldownHandle, this, &APlayerCharacter::DashResetCooldown, DodgeCooldown, false);
		DodgeTimeline->PlayFromStart();
		ReceiveOnDash();
	}

	UE_LOG(LogTemp, Display, TEXT("PlayerCharacter - Dash - Finished"));
}

void APlayerCharacter::DashResetCooldown()
{
	bMayDodge = true;
	ReceiveOnDashOffCooldown();
}

void APlayerCharacter::Blink()
{
	if (!bPlayerIsDead && OwnedGloaming && IsValid(OwnedGloaming) && OwnedGloaming->bTeleportable)
	{
		SetActorLocation(OwnedGloaming->GetActorLocation());

		OwnedGloaming->OnThrowFinished();
		ReceiveOnBlink();
	}

}

void APlayerCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds() * (bInvertCameraLeftRight ? -1.0f : 1.0f));
}

void APlayerCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds() * (bInvertCameraUpDown ? -1.0f : 1.0f));
}

void APlayerCharacter::TurnAtRateMouse(float Rate)
{
	AddControllerYawInput(Rate * (bInvertCameraLeftRight ? -1.0f : 1.0f));
}

void APlayerCharacter::LookUpAtRateMouse(float Rate)
{
	AddControllerPitchInput(Rate * (bInvertCameraUpDown ? -1.0f : 1.0f));
}

void APlayerCharacter::OnCapsuleBeginOverlap(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	AFogKillArea* KillArea = Cast<AFogKillArea>(OtherActor);
	if (KillArea)
	{
		FogSystem->SetNiagaraVariableFloat("User.FogPercentage", 0.0f);
	}
}

void APlayerCharacter::OnCapsuleEndOverlap(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex)
{
	AFogKillArea* KillArea = Cast<AFogKillArea>(OtherActor);
	if (KillArea)
	{
		FogSystem->SetNiagaraVariableFloat("User.FogPercentage", 1.0f);
	}
}

void APlayerCharacter::HoldGloaming()
{
	if (IsValid(OwnedGloaming))
	{
		if (!bPlayerIsDead && !OwnedGloaming->bThrowingState)
		{
			OwnedGloaming->HoldGloaming();
		}
	}
	else
	{
		//update property 
		OwnedGloaming = nullptr;
	}
}

void APlayerCharacter::DropHoldingGloaming() {
	if (!bIsThrowingActivated)
	{
		if (IsValid(OwnedGloaming))
		{
			if (OwnedGloaming->bThrowingState)
			{
				OwnedGloaming->DropGloaming();
			}
		}
		else
		{
			//update property 
			OwnedGloaming = nullptr;
		}
	}
}

void APlayerCharacter::ThrowGloaming()
{
	if (IsValid(OwnedGloaming))
	{
		if (!bPlayerIsDead && OwnedGloaming->bThrowingState)
		{
			//TODO take velocity of actor into consideration so that gloamings are thrown roughly the same distance when moving or not
			FVector LaunchVector = GetActorForwardVector() * 3 + GetActorUpVector() * 2;
			LaunchVector.Normalize();
			LaunchVector = LaunchVector * LaunchForce;

			OwnedGloaming->ThrowGloaming();
			OwnedGloaming->LaunchCharacter(LaunchVector, true, true);
		}
	}
	else
	{
		//update property 
		OwnedGloaming = nullptr;
	}
	bIsThrowingActivated = false;
}

void APlayerCharacter::Throw()
{
	if (IsValid(OwnedGloaming))
	{
		if(OwnedGloaming->bThrowingState)
			bIsThrowingActivated = true;

		ReceiveOnThrow();
	}
	else
	{
		//update property 
		OwnedGloaming = nullptr;
		bIsThrowingActivated = false; //better safe than sorry
	}
}

void APlayerCharacter::MoveForward(float Value)
{
	if (!bPlayerIsDead && (Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void APlayerCharacter::MoveRight(float Value)
{
	if (!bPlayerIsDead && (Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void APlayerCharacter::Jump()
{
	if (!bPlayerIsDead && !GetCharacterMovement()->IsFalling())
	{
		ACharacter::Jump();
		ReceiveOnJump();
	}
}

void APlayerCharacter::StopJumping()
{
	ACharacter::StopJumping();
}

void APlayerCharacter::NavigationSpell()
{
	if (!bPlayerIsDead)
	{
		AEnlightPlayerController* PlayerController = Cast<AEnlightPlayerController>(GetController());

		if (PlayerController && !PlayerController->bNavigationActive)
		{
			PlayerController->ActivateNavigation();
			ReceiveOnActivateNavigation();
		}
	}
}

void APlayerCharacter::NavigationSpellOver()
{
	AEnlightPlayerController* PlayerController = Cast<AEnlightPlayerController>(GetController());

	if (PlayerController && PlayerController->bNavigationActive)
	{
		PlayerController->DeactivateNavigation();
		ReceiveOnDeactivateNavigation();
	}
}

void APlayerCharacter::TimelineCallback(float Value)
{//called every tick of the timeline
	if (MeshImitator)
	{
		MeshImitator->SetNiagaraVariableFloat("User.Noise", Value);
	}
}

void APlayerCharacter::TimelineFinishedCallback()
{//called when timeline is finished
	if (MeshImitator)
	{
		MeshImitator->SetNiagaraVariableFloat("User.Lerp", 0.3f);
		MeshImitator->SetNiagaraVariableFloat("User.Noise", 1.0f);
	}
}

void APlayerCharacter::OpenMenu()
{
	UE_LOG(LogTemp, Display, TEXT("PlayerCharacter - OpenMenu - Begin"));

	UWorld* World = GetWorld();

	if (IngameMenu != nullptr)
	{
		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
		ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldDynamic));
		ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_PhysicsBody));
		TArray<AActor*> ActorsToIgnore;
		TArray<AActor*> HitActors;

		UKismetSystemLibrary::SphereOverlapActors(World, GetActorLocation(), 400, ObjectTypes, ABaseSavingPoint::StaticClass(), ActorsToIgnore, HitActors);

		bool bIsSavingPossible = false;
		for (auto& Hit : HitActors)
		{
			bIsSavingPossible = bIsSavingPossible ||
				FVector::Distance(GetActorLocation(), Hit->GetActorLocation()) < MaxSavePointDistance;

			if (bIsSavingPossible)
				break;
		}

		FStringOutputDevice OutputDevice;
		const FString command = FString::Printf(TEXT("SetIsSavingPossible %d"), bIsSavingPossible);
		IngameMenu->CallFunctionByNameWithArguments(*command, OutputDevice, this, true);

		IngameMenu->AddToViewport();
		Cast<APlayerController>(GetController())->bShowMouseCursor = true;
		Cast<APlayerController>(GetController())->SetInputMode(FInputModeUIOnly());
		Cast<APlayerController>(GetController())->SetPause(true);

		ReceiveOnOpenMenu();
	}

	UE_LOG(LogTemp, Display, TEXT("PlayerCharacter - OpenMenu - Finished"));
}

void APlayerCharacter::Interact()
{
	if (!bPlayerIsDead)
	{
		Interactor->Interact();
	}
}