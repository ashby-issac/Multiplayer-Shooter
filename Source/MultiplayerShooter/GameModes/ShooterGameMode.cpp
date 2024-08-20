// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterGameMode.h"
#include "MultiplayerShooter/Character/ShooterCharacter.h"
#include "MultiplayerShooter/PlayerController/ShooterPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

void AShooterGameMode::OnPlayerEliminated(AShooterCharacter *ElimCharacter, AShooterPlayerController *ElimController, AShooterPlayerController *AttackerController)
{
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
        TArray<AActor*> PlayerStarts;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStarts);
        int32 PlayerStartIndex = FMath::RandRange(0, PlayerStarts.Num() - 1);
        RestartPlayerAtPlayerStart(Controller, PlayerStarts[PlayerStartIndex]);
    }
}