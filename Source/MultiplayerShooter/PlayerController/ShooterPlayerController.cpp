
#include "ShooterPlayerController.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "MultiplayerShooter/HUD/ShooterHUD.h"
#include "MultiplayerShooter/HUD/CharacterOverlay.h"
#include "MultiplayerShooter/Character/ShooterCharacter.h"
#include "MultiplayerShooter/GameModes/ShooterGameMode.h"
#include "MultiplayerShooter/HUD/Announcement.h"


void AShooterPlayerController::BeginPlay()
{
    Super::BeginPlay();

    ShooterHUD = Cast<AShooterHUD>(GetHUD());
    if (ShooterHUD != nullptr)
    {
        ShooterHUD->AddAnnouncementOverlay();
    }
}

void AShooterPlayerController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    PollInit();
    SetHUDCountdown();
    CheckTimeSync(DeltaSeconds);
}

void AShooterPlayerController::PollInit()
{
    if (CharacterOverlay == nullptr)
    {
        if (ShooterHUD != nullptr && ShooterHUD->CharacterOverlay != nullptr)
        {
            CharacterOverlay = ShooterHUD->CharacterOverlay;

            SendScoreHUDUpdate(CachedScore);
            SendDefeatsHUDUpdate(CachedDefeats);
            SendHealthHUDUpdate(CachedHealth, CachedMaxHealth);
        }
    }
}

void AShooterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AShooterPlayerController, MatchState);
}

void AShooterPlayerController::CheckTimeSync(float DeltaSeconds)
{
    SyncTimer += DeltaSeconds;
    if (SyncTimer >= SyncTimerDelay)
    {
        ClientRequestServerTime(GetWorld()->GetTimeSeconds());
        SyncTimer = 0;
    }
}

void AShooterPlayerController::ReceivedPlayer()
{
    Super::ReceivedPlayer();

    ClientRequestServerTime(GetWorld()->GetTimeSeconds());
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

void AShooterPlayerController::OnRep_MatchState()
{
    if (MatchState == MatchState::InProgress)
    {
        ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
        if (ShooterHUD != nullptr)
        {
            ShooterHUD->AddCharacterOverlay();
            if (ShooterHUD->AnnouncementOverlay != nullptr)
            {
                ShooterHUD->AnnouncementOverlay->SetVisibility(ESlateVisibility::Hidden);
            }
        }
    }
}

void AShooterPlayerController::ClientRequestServerTime_Implementation(float ClientRequestTime)
{
    ServerResponseServerTime(ClientRequestTime, GetWorld()->GetTimeSeconds());
}

void AShooterPlayerController::ServerResponseServerTime_Implementation(float ClientRequestTime, float ServersTimeOfReceipt)
{
    float RoundTripTime = GetWorld()->GetTimeSeconds() - ClientRequestTime;
    float CurrentServerTime = ServersTimeOfReceipt + (0.5f * RoundTripTime);
    ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

float AShooterPlayerController::GetServerTime()
{
    return HasAuthority() ? GetWorld()->GetTimeSeconds() : GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void AShooterPlayerController::SendHealthHUDUpdate(float Health, float MaxHealth)
{
    ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
    if (ShooterHUD != nullptr && ShooterHUD->CharacterOverlay != nullptr)
    {
        ShooterHUD->CharacterOverlay->UpdateHealthInfo(Health, MaxHealth);
    }
    else 
    {
        bInitializeCharacterOverlay = true;
        CachedHealth = Health;
        CachedMaxHealth = MaxHealth;
    }
}

void AShooterPlayerController::SendScoreHUDUpdate(float Score)
{
    ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
    if (ShooterHUD != nullptr && ShooterHUD->CharacterOverlay != nullptr)
    {
        ShooterHUD->CharacterOverlay->UpdateScoreValue(Score);
    }
    else 
    {
        bInitializeCharacterOverlay = true;
        CachedScore = Score;
    }
}

void AShooterPlayerController::SendDefeatsHUDUpdate(int32 Defeat)
{
    ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
    if (ShooterHUD != nullptr && ShooterHUD->CharacterOverlay != nullptr)
    {
        ShooterHUD->CharacterOverlay->UpdateDefeatValue(Defeat);
    }
    else 
    {
        bInitializeCharacterOverlay = true;
        CachedDefeats = Defeat;
    }
}

void AShooterPlayerController::SendWeaponAmmoHUDUpdate(int32 Ammo)
{
    ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
    if (ShooterHUD != nullptr && ShooterHUD->CharacterOverlay != nullptr)
    {
        ShooterHUD->CharacterOverlay->UpdateWeaponAmmoValue(Ammo);
    }
}

void AShooterPlayerController::SendCarriedAmmoHUDUpdate(int32 Ammo)
{
    ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
    if (ShooterHUD != nullptr && ShooterHUD->CharacterOverlay != nullptr)
    {
        ShooterHUD->CharacterOverlay->UpdateCarriedAmmoValue(Ammo);
    }
}

void AShooterPlayerController::SendMatchCountdownHUDUpdate(float TotalSeconds)
{
    ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
    if (ShooterHUD != nullptr && ShooterHUD->CharacterOverlay != nullptr)
    {
        ShooterHUD->CharacterOverlay->UpdateMatchCountdownValue(TotalSeconds);
    }
}

void AShooterPlayerController::OnMatchStateSet(FName NewState)
{   
    MatchState = NewState;

    if (MatchState == MatchState::InProgress)
    {
        ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
        if (ShooterHUD != nullptr)
        {
            ShooterHUD->AddCharacterOverlay();
            if (ShooterHUD->AnnouncementOverlay != nullptr)
            {
                ShooterHUD->AnnouncementOverlay->SetVisibility(ESlateVisibility::Hidden);
            }
        }
    }
}

void AShooterPlayerController::SetHUDCountdown()
{
    int32 TimeLeft = FMath::CeilToInt32(MatchTimer - GetServerTime());

    if (CountdownInt != TimeLeft)
        SendMatchCountdownHUDUpdate(TimeLeft);

    CountdownInt = TimeLeft;
}