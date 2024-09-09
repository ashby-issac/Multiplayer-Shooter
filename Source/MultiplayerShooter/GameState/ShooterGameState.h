// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "ShooterGameState.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSHOOTER_API AShooterGameState : public AGameState
{
	GENERATED_BODY()

public:
	UPROPERTY(Replicated)
	TArray<class AShooterPlayerState*> TopScoringPlayers;

	void UpdateTopScorers(AShooterPlayerState* TopScorer);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	float CurrentTopScore = 0.f;
};
