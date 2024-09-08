// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "MultiplayerShooter/Character/ShooterCharacter.h"
#include "MultiplayerShooter/PlayerStates/ShooterPlayerState.h"
#include "MultiplayerShooter/PlayerController/ShooterPlayerController.h"

namespace MatchState
{
    const FName Cooldown = FName(TEXT("Cooldown"));
}

AShooterGameMode::AShooterGameMode()
{
    PrimaryActorTick.bCanEverTick = true;

    bDelayedStart = true;
}

void AShooterGameMode::BeginPlay()
{
    Super::BeginPlay();

    LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void AShooterGameMode::OnMatchStateSet()
{
    Super::OnMatchStateSet();

    for (FConstPlayerControllerIterator it = GetWorld()->GetPlayerControllerIterator(); it; ++it)
    {
        AShooterPlayerController* ShooterController = Cast<AShooterPlayerController>(*it);
        if (ShooterController != nullptr)
        {
            ShooterController->OnMatchStateSet(MatchState);
        }
    }
}

void AShooterGameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (MatchState == MatchState::WaitingToStart)
    {
        CountdownTimer = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
        if (CountdownTimer <= 0)
        {
            StartMatch();
        }
    }
    else if (MatchState == MatchState::InProgress)
    {
        CountdownTimer = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
        if (CountdownTimer <= 0)
        {
            SetMatchState(MatchState::Cooldown);
        }
    }
}

void AShooterGameMode::OnPlayerEliminated(AShooterCharacter *ElimCharacter, AShooterPlayerController *ElimController, AShooterPlayerController *AttackerController)
{
    if (AttackerController != nullptr && AttackerController != ElimController)
    {
        AShooterPlayerState *ShooterPlayerState = Cast<AShooterPlayerState>(AttackerController->PlayerState);
        ShooterPlayerState->AddToScore(1.f);
    }

    if (ElimController != nullptr)
    {
        AShooterPlayerState *ShooterPlayerState = Cast<AShooterPlayerState>(ElimController->PlayerState);
        ShooterPlayerState->AddToDefeats(1.f);
    }

    if (ElimCharacter != nullptr)
    {
        ElimCharacter->OnEliminated();
    }
}

void AShooterGameMode::RespawnPlayer(AController *Controller, AActor *Player)
{
    if (Player != nullptr)
    {
        Player->Reset();
        Player->Destroy();
    }

    if (Controller != nullptr)
    {
        TArray<AActor *> PlayerStarts;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStarts);
        int32 PlayerStartIndex = FMath::RandRange(0, PlayerStarts.Num() - 1);
        RestartPlayerAtPlayerStart(Controller, PlayerStarts[PlayerStartIndex]);
    }
}