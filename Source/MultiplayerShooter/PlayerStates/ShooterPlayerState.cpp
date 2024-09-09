// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterPlayerState.h"
#include "MultiplayerShooter/Character/ShooterCharacter.h"
#include "MultiplayerShooter/PlayerController/ShooterPlayerController.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "MultiplayerShooter/GameState/ShooterGameState.h"
#include "MultiplayerShooter/PlayerStates/ShooterPlayerState.h"

void AShooterPlayerState::AddToScore(float ScoreAmt)
{
    float ScoreVal = GetScore() + ScoreAmt;
    SetScore(ScoreVal);
    UpdateScoreHUD();
}

void AShooterPlayerState::AddToDefeats(int32 DefeatAmt)
{
    Defeats += DefeatAmt;

    UpdateDefeatsHUD();
}

void AShooterPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AShooterPlayerState, Defeats);
}

void AShooterPlayerState::OnRep_Score()
{
    Super::OnRep_Score();

    UpdateScoreHUD();
}

void AShooterPlayerState::OnRep_Defeat()
{
    UpdateDefeatsHUD();
}

void AShooterPlayerState::UpdateScoreHUD()
{
    ShooterCharacter = ShooterCharacter == nullptr ? Cast<AShooterCharacter>(GetPawn()) : ShooterCharacter;

    if (ShooterCharacter != nullptr)
    {
        ShooterPlayerController = ShooterPlayerController == nullptr ? Cast<AShooterPlayerController>(ShooterCharacter->Controller) 
                                                                     : ShooterPlayerController;
        if (ShooterPlayerController != nullptr)
        {
            ShooterPlayerController->SendScoreHUDUpdate(GetScore());
        }
    }
}

void AShooterPlayerState::UpdateDefeatsHUD()
{
    ShooterCharacter = ShooterCharacter == nullptr ? Cast<AShooterCharacter>(GetPawn()) : ShooterCharacter;

    if (ShooterCharacter)
    {
        ShooterPlayerController = ShooterPlayerController == nullptr ? Cast<AShooterPlayerController>(ShooterCharacter->Controller) 
                                                                     : ShooterPlayerController;
        if (ShooterPlayerController)
        {
            ShooterPlayerController->SendDefeatsHUDUpdate(Defeats);
        }
    }
}

