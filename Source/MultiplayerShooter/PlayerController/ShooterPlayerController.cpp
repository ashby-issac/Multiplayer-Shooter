// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterPlayerController.h"
#include "MultiplayerShooter/HUD/ShooterHUD.h"
#include "MultiplayerShooter/HUD/CharacterOverlay.h"
#include "Kismet/GameplayStatics.h"

void AShooterPlayerController::BeginPlay()
{
    Super::BeginPlay();

    ShooterHUD = Cast<AShooterHUD>(GetHUD());
    auto PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

    FString PawnRoleText;
	switch (PlayerPawn->GetLocalRole())
	{
		case ENetRole::ROLE_Authority:
			PawnRoleText = FString("Authority");
			break;
		case ENetRole::ROLE_SimulatedProxy:
			PawnRoleText = FString("SimulatedProxy");
			break;
		case ENetRole::ROLE_AutonomousProxy:
			PawnRoleText = FString("AutonomousProxy");
			break;
		case ENetRole::ROLE_None:
			PawnRoleText = FString("None");
			break;
	}

    if (ShooterHUD == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s ShooterHUD is null"), *PawnRoleText);
    }
    else 
    {
        UE_LOG(LogTemp, Warning, TEXT("%s ShooterHUD is not null"), *PawnRoleText);
    }
}

void AShooterPlayerController::UpdatePlayerHUD(float Health, float MaxHealth)
{
    if (GetHUD() == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("GETHUD() is null"));
    }

    if (ShooterHUD == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("ShooterHUD is null"));
    }
    if (ShooterHUD != nullptr && ShooterHUD->CharacterOverlay == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("ShooterHUD: CharacterOverlay is null"));
    }

    if (ShooterHUD != nullptr && ShooterHUD->CharacterOverlay != nullptr)
    {
        ShooterHUD->CharacterOverlay->UpdateHealthInfo(Health, MaxHealth);
    }
}