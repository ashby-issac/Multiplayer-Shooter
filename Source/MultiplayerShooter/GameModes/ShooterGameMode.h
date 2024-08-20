// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "ShooterGameMode.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSHOOTER_API AShooterGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	void RespawnPlayer(AController* Controller, AActor* Player);
	void OnPlayerEliminated(class AShooterCharacter* ElimCharacter, class AShooterPlayerController* ElimController, AShooterPlayerController* AttackerController);
};
