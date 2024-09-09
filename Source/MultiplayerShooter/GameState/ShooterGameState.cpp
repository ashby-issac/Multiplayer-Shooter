// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterGameState.h"
#include "Net/UnrealNetwork.h"
#include "MultiplayerShooter/PlayerStates/ShooterPlayerState.h"

void AShooterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShooterGameState, TopScoringPlayers);
}

void AShooterGameState::UpdateTopScorers(AShooterPlayerState* TopScorer)
{
	if (TopScoringPlayers.IsEmpty())
	{
		TopScoringPlayers.Add(TopScorer);
		CurrentTopScore = TopScorer->GetScore();
	}
	else
	{
		if (!TopScoringPlayers.Contains(TopScorer))
		{
			if (TopScorer->GetScore() > CurrentTopScore)
			{
				TopScoringPlayers.Empty();
				TopScoringPlayers.Add(TopScorer);
				CurrentTopScore = TopScorer->GetScore();
			}
			else if (TopScorer->GetScore() == CurrentTopScore)
			{
				TopScoringPlayers.Add(TopScorer);
			}
		}
	}
}