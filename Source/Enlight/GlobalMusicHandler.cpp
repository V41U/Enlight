// Enlight Game 2019


#include "GlobalMusicHandler.h"

#include "PlayerCharacter.h"
#include "Runtime/Engine/Public/TimerManager.h"

// Sets default values
AGlobalMusicHandler::AGlobalMusicHandler()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AudioComponentCalm = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponentCalm"));
	AudioComponentCalm->SetupAttachment(RootComponent);

	AudioComponentDanger = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponentDanger"));
	AudioComponentDanger->SetupAttachment(RootComponent);

	AudioComponentCombat = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponentCombat"));
	AudioComponentCombat->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AGlobalMusicHandler::BeginPlay()
{
	Super::BeginPlay();
	
	UWorld* World = GetWorld();
	if (World)
	{
		//initialize songs
		CalmAudioFinished();

		EnlightPlayer = Cast<APlayerCharacter>(World->GetFirstPlayerController()->GetPawn());
		if (EnlightPlayer)
		{
			FAttachmentTransformRules AttachmentTransformRules(EAttachmentRule::SnapToTarget, true);
			AttachToActor(EnlightPlayer, AttachmentTransformRules);
		}
		World->GetTimerManager().SetTimer(GlobalMusicTimerHandle, this, &AGlobalMusicHandler::GlobalMusicUpdate, 1 / UpdatesPerSecond, true);
	}
}

// Called every frame
void AGlobalMusicHandler::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGlobalMusicHandler::CalmAudioFinished()
{
	UE_LOG(LogInit, Display, TEXT("Restarting Calm Audio."));

	AudioComponentCalm->Stop();
	AudioComponentDanger->Stop();
	AudioComponentCombat->Stop();

	ActiveSong = Songs[FMath::RandRange(0, (Songs.Num() - 1))];
	if (ActiveSong.IsValid() && IsValid(ActiveSong.CalmMusic))
	{
		//only calm music is allowed to swap all songs
		if (ActiveSong.CalmMusic)
		{
			AudioComponentCalm->SetSound(ActiveSong.CalmMusic);
			AudioComponentCalm->Play(0.0f);
			AudioComponentCalm->AdjustVolume(0.0f, 0.001f);
		}
		if (ActiveSong.DangerMusic)
		{
			AudioComponentDanger->SetSound(ActiveSong.DangerMusic);
			AudioComponentDanger->Play(0.0f);
			AudioComponentDanger->AdjustVolume(0.0f, 0.001f);
		}
		if (ActiveSong.CombatMusic)
		{
			AudioComponentCombat->SetSound(ActiveSong.CombatMusic);
			AudioComponentCombat->Play(0.0f);
			AudioComponentCombat->AdjustVolume(0.0f, 0.001f);
		}

		UWorld* World = GetWorld();
		if (IsValid(World))
		{
			World->GetTimerManager().SetTimer(SongSwapTimerHandle, this, &AGlobalMusicHandler::CalmAudioFinished, ActiveSong.CalmMusic->GetDuration(), false);
		}
	}
}

void AGlobalMusicHandler::TimedMusicModification(float VolumeModifier, float Duration)
{
	VolumeMultiplier = VolumeModifier;
	FTimerDelegate TimerCallback;
	TimerCallback.BindLambda([this]()
	{
		VolumeMultiplier = 1.0f;
	});

	//manually call update so that volume is refreshed
	GlobalMusicUpdate();
	
	UWorld* World = GetWorld();
	if (World)
		World->GetTimerManager().SetTimer(VolumeModificationTimerHandle, TimerCallback, Duration, false);
}

void AGlobalMusicHandler::GlobalMusicUpdate()
{
	UE_LOG(LogTemp, Display, TEXT("GlobalMusicHandler - GlobalMusicUpdate - Begin"));

	UWorld* World = GetWorld();

	if (Songs.Num() > 0 && IsValid(EnlightPlayer))
	{
		if (EnlightPlayer->CurrentDangerPercentage < CalmThreshold)
		{//calm
			AudioComponentCalm->AdjustVolume(0.5f, GlobalMaxMusicVolume * VolumeMultiplier);
			AudioComponentDanger->AdjustVolume(0.5f, 0.001f);
			AudioComponentCombat->AdjustVolume(0.5f, 0.001f);
		}
		else if (EnlightPlayer->CurrentDangerPercentage < DangerThreshold)
		{//danger
			AudioComponentCalm->AdjustVolume(0.5f, GlobalMaxMusicVolume * VolumeMultiplier);
			AudioComponentDanger->AdjustVolume(0.5f, GlobalMaxMusicVolume * VolumeMultiplier);
			AudioComponentCombat->AdjustVolume(0.5f, 0.001f);
		}
		else
		{//combat
			AudioComponentCalm->AdjustVolume(0.5f, GlobalMaxMusicVolume * VolumeMultiplier);
			AudioComponentDanger->AdjustVolume(0.5f, GlobalMaxMusicVolume * VolumeMultiplier);
			AudioComponentCombat->AdjustVolume(0.5f, GlobalMaxMusicVolume * VolumeMultiplier);
		}
	}

	UE_LOG(LogTemp, Display, TEXT("GlobalMusicHandler - GlobalMusicUpdate - Finished"));
}