// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "ShooterGameMode.generated.h"

namespace MatchState
{
	extern MULTIPLAYERSHOOTER_API const FName Cooldown;
}

UCLASS()
class MULTIPLAYERSHOOTER_API AShooterGameMode : public AGameMode
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;

public:
	AShooterGameMode();

	virtual void Tick(float DeltaSeconds) override;

	void RespawnPlayer(AController* Controller, AActor* Player);
	void OnPlayerEliminated(class AShooterCharacter* ElimCharacter, class AShooterPlayerController* ElimController, AShooterPlayerController* AttackerController);

	UPROPERTY(EditAnywhere, Category = "Time")
	float WarmupTime = 10.f;
	
	UPROPERTY(EditAnywhere, Category = "Time")
	float MatchTime = 120.f;

	UPROPERTY(EditAnywhere, Category = "Time")
	float CooldownTime = 120.f;

	float LevelStartingTime = 0.f;

private:
	float CountdownTimer = 0.f;

public:
	FORCEINLINE float GetCountdownTimer() { return CountdownTimer; }
};
