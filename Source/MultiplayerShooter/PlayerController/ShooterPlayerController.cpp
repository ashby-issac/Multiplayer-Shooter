// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterPlayerController.h"
#include "MultiplayerShooter/HUD/ShooterHUD.h"
#include "MultiplayerShooter/HUD/CharacterOverlay.h"
#include "Kismet/GameplayStatics.h"
#include "MultiplayerShooter/Character/ShooterCharacter.h"

void AShooterPlayerController::BeginPlay()
{
    Super::BeginPlay();

    ShooterHUD = Cast<AShooterHUD>(GetHUD());
}

void AShooterPlayerController::OnPossess(APawn *aPawn)
{
    Super::OnPossess(aPawn);

    AShooterCharacter *ShooterCharacter = Cast<AShooterCharacter>(aPawn);
    if (ShooterCharacter != nullptr)
    {
        SendHealthHUDUpdate(ShooterCharacter->GetHealth(), ShooterCharacter->GetMaxHealth());
    }
}

void AShooterPlayerController::SendHealthHUDUpdate(float Health, float MaxHealth)
{
    ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
    if (ShooterHUD != nullptr && ShooterHUD->CharacterOverlay != nullptr)
    {
        ShooterHUD->CharacterOverlay->UpdateHealthInfo(Health, MaxHealth);
    }
}

void AShooterPlayerController::SendScoreHUDUpdate(float Score)
{
    ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
    if (ShooterHUD != nullptr && ShooterHUD->CharacterOverlay != nullptr)
    {
        ShooterHUD->CharacterOverlay->UpdateScoreValue(Score);
    }
}

void AShooterPlayerController::SendDefeatsHUDUpdate(int32 Defeat)
{
    ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
    if (ShooterHUD != nullptr && ShooterHUD->CharacterOverlay != nullptr)
    {
        ShooterHUD->CharacterOverlay->UpdateDefeatValue(Defeat);
    }
}