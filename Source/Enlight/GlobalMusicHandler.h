// Enlight Game 2019

#pragma once


#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GlobalMusicHandler.generated.h"

class APlayerCharacter;

USTRUCT(BlueprintType)
struct FGlobalMusicSong {
	GENERATED_USTRUCT_BODY()
public:
	FGlobalMusicSong() {
		CalmMusic = nullptr;
		DangerMusic = nullptr;
		CombatMusic = nullptr;
	};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		USoundCue* CalmMusic;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		USoundCue* DangerMusic;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		USoundCue* CombatMusic;

	bool IsValid() { return CalmMusic != nullptr && DangerMusic != nullptr && CombatMusic != nullptr; }
};

UCLASS()
class ENLIGHT_API AGlobalMusicHandler : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGlobalMusicHandler();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY()
		UAudioComponent* AudioComponentCalm;

	UPROPERTY()
		UAudioComponent* AudioComponentDanger;

	UPROPERTY()
		UAudioComponent* AudioComponentCombat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
		float GlobalMaxMusicVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
		float UpdatesPerSecond = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
		TArray<FGlobalMusicSong> Songs;
	UPROPERTY(BlueprintReadOnly, Category = "Music")
		FGlobalMusicSong ActiveSong;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
		float CalmThreshold = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
		float DangerThreshold = 0.8f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Music")
		float VolumeMultiplier = 1.0f;

	UFUNCTION()
		void CalmAudioFinished();

	UFUNCTION(BlueprintCallable)
		void TimedMusicModification(float VolumeModifier, float Duration);

private:
	APlayerCharacter* EnlightPlayer;

	void GlobalMusicUpdate();
	
	FTimerHandle GlobalMusicTimerHandle;
	FTimerHandle SongSwapTimerHandle;

	FTimerHandle VolumeModificationTimerHandle;
};
