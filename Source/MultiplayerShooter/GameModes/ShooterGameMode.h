// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "ShooterGameMode.generated.h"

UCLASS()
class MULTIPLAYERSHOOTER_API AShooterGameMode : public AGameMode
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	AShooterGameMode();

	virtual void Tick(float DeltaSeconds) override;

	void RespawnPlayer(AController* Controller, AActor* Player);
	void OnPlayerEliminated(class AShooterCharacter* ElimCharacter, class AShooterPlayerController* ElimController, AShooterPlayerController* AttackerController);

	UPROPERTY(EditAnywhere, Category = "Time")
	float WarmupTime = 10.f;
	
	float LevelStartingTime;

private:
	float CountdownTimer = 0.f;
};
