
#include "ShooterPlayerController.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "MultiplayerShooter/HUD/ShooterHUD.h"
#include "MultiplayerShooter/HUD/CharacterOverlay.h"
#include "MultiplayerShooter/Character/ShooterCharacter.h"
#include "MultiplayerShooter/GameModes/ShooterGameMode.h"
#include "MultiplayerShooter/HUD/Announcement.h"
#include "MultiplayerShooter/GameState/ShooterGameState.h"
#include "MultiplayerShooter/PlayerStates/ShooterPlayerState.h"
#include "MultiplayerShooter/Combat/CombatComponent.h"

void AShooterPlayerController::BeginPlay()
{
    Super::BeginPlay();

    ShooterHUD = Cast<AShooterHUD>(GetHUD());
    ServerCheckMatchState();
}

void AShooterPlayerController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    SetHUDCountdown();
    CheckTimeSync(DeltaSeconds);
    PollInit();
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

            AShooterCharacter* ShooterChar = Cast<AShooterCharacter>(GetPawn());
            if (ShooterChar && ShooterChar->GetCombatComponent())
            {
                SendGrenadesHUDUpdate(ShooterChar->GetCombatComponent()->GetGrenadesCount());
            }
        }
    }
}

void AShooterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
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

    if (IsLocalController())
    {
        ClientRequestServerTime(GetWorld()->GetTimeSeconds());
    }
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
        HandleMatchStart();
    }
    else if (MatchState == MatchState::Cooldown)
    {
        HandleCooldown();
    }
}

// Server RPC
void AShooterPlayerController::ClientRequestServerTime_Implementation(float ClientRequestTime)
{
    ServerResponseServerTime(ClientRequestTime, GetWorld()->GetTimeSeconds());
}

// Client RPC
void AShooterPlayerController::ServerResponseServerTime_Implementation(float ClientRequestTime, float ServersTimeOfReceipt)
{
    float RoundTripTime = GetWorld()->GetTimeSeconds() - ClientRequestTime;
    float CurrentServerTime = ServersTimeOfReceipt + (0.5f * RoundTripTime);
    ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

void AShooterPlayerController::ClientJoinMidGame_Implementation(FName StateOfMatch, float RemWarmupTime, float RemMatchTime, float RemCooldownTime, float RemStartTime)
{
    MatchTime = RemMatchTime;
    WarmupTime = RemWarmupTime;
    CooldownTime = RemCooldownTime;
    LevelStartingTime = RemStartTime;
    MatchState = StateOfMatch;

    OnMatchStateSet(MatchState);

    if (ShooterHUD != nullptr && MatchState == MatchState::WaitingToStart)
    {
        ShooterHUD->AddAnnouncementOverlay();
    }
}

void AShooterPlayerController::ServerCheckMatchState_Implementation()
{
    AShooterGameMode *ShooterGameMode = Cast<AShooterGameMode>(UGameplayStatics::GetGameMode(this));
    if (ShooterGameMode != nullptr)
    {
        MatchTime = ShooterGameMode->MatchTime;
        WarmupTime = ShooterGameMode->WarmupTime;
        CooldownTime = ShooterGameMode->CooldownTime;
        LevelStartingTime = ShooterGameMode->LevelStartingTime;
        MatchState = ShooterGameMode->GetMatchState();

        ClientJoinMidGame(MatchState, WarmupTime, MatchTime, CooldownTime, LevelStartingTime);
    }
}

float AShooterPlayerController::GetServerTime()
{
    if (HasAuthority())
    {
        return GetWorld()->GetTimeSeconds();
    }
    return GetWorld()->GetTimeSeconds() + ClientServerDelta;
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

void AShooterPlayerController::SendWarmupCountdownHUDUpdate(float TotalSeconds)
{
    ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
    if (ShooterHUD != nullptr && ShooterHUD->AnnouncementOverlay != nullptr)
    {
        ShooterHUD->AnnouncementOverlay->UpdateWarmupCountdownValue(TotalSeconds);
    }
}

void AShooterPlayerController::SendCooldownCountdownHUDUpdate(float TotalSeconds)
{
    ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
    if (ShooterHUD != nullptr && ShooterHUD->AnnouncementOverlay != nullptr)
    {
        ShooterHUD->AnnouncementOverlay->UpdateCooldownAnnouncementHUD(TotalSeconds);
    }
}

void AShooterPlayerController::SendGrenadesHUDUpdate(int32 Grenades)
{
    ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
    if (ShooterHUD != nullptr && ShooterHUD->CharacterOverlay != nullptr)
    {
        ShooterHUD->CharacterOverlay->UpdateGrenadesValue(Grenades);
    }
    else
    {
        bInitializeCharacterOverlay = true;
        CachedGrenades = Grenades;
    }
}

void AShooterPlayerController::OnMatchStateSet(FName NewState)
{
    MatchState = NewState;

    if (MatchState == MatchState::InProgress)
    {
        HandleMatchStart();
    }
    else if (MatchState == MatchState::Cooldown)
    {
        HandleCooldown();
    }
}

void AShooterPlayerController::HandleMatchStart()
{
    ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
    if (ShooterHUD != nullptr)
    {
        if (ShooterHUD->AnnouncementOverlay != nullptr)
            ShooterHUD->AnnouncementOverlay->SetVisibility(ESlateVisibility::Hidden);

        if (ShooterHUD->CharacterOverlay == nullptr)
            ShooterHUD->AddCharacterOverlay();
    }
}

void AShooterPlayerController::HandleCooldown()
{
    ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
    if (ShooterHUD != nullptr && ShooterHUD->CharacterOverlay != nullptr)
    {
        ShooterHUD->CharacterOverlay->RemoveFromParent();
        if (ShooterHUD->AnnouncementOverlay != nullptr)
        {
            ShooterHUD->AnnouncementOverlay->SetVisibility(ESlateVisibility::Visible);

            AShooterGameState* ShooterGameState = Cast<AShooterGameState>(UGameplayStatics::GetGameState(this));
            AShooterPlayerState* ShooterPlayerState = GetPlayerState<AShooterPlayerState>();
            if (ShooterGameState != nullptr && ShooterPlayerState != nullptr)
            {
                FString InfoText("");
                if (ShooterGameState->TopScoringPlayers.Num() == 0)
                {
                    InfoText = FString("There is no winner");
                }
                else if (ShooterGameState->TopScoringPlayers.Num() > 1)
                {
                    InfoText = FString("Winners:\n");
                    for (AShooterPlayerState* TopScorer : ShooterGameState->TopScoringPlayers)
                    {
                        InfoText.Append(FString::Printf(TEXT("%s\n"), *TopScorer->GetPlayerName()));
                    }
                }
                else
                {
                    if (ShooterGameState->TopScoringPlayers[0] == ShooterPlayerState)
                    {
                        InfoText = FString("You're the winner");
                    }
                    else
                    {
                        InfoText = FString::Printf(TEXT("MVP\n%s"), *ShooterGameState->TopScoringPlayers[0]->GetPlayerName());
                    }
                }
                ShooterHUD->AnnouncementOverlay->UpdateWinnersText(InfoText);
            }
        }
    }

    AShooterCharacter* CharacterInstance = Cast<AShooterCharacter>(GetPawn());
    if (CharacterInstance != nullptr)
    {
        CharacterInstance->bDisableGameplay = true;
        UCombatComponent* CombatComponent = CharacterInstance->GetCombatComponent();
        if (CombatComponent != nullptr)
        {
            CombatComponent->SetFiringState(false);
            CombatComponent->SetAimingState(false);
        }
    }
}

void AShooterPlayerController::SetHUDCountdown()
{
    float TimeLeft = 0.f;

    if (HasAuthority())
    {
        MainShooterGameMode = MainShooterGameMode == nullptr ? Cast<AShooterGameMode>(UGameplayStatics::GetGameMode(this)) : MainShooterGameMode;
        if (MainShooterGameMode != nullptr)
        {
            TimeLeft = MainShooterGameMode->GetCountdownTimer();
        }
    }
    else
    {
        if (MatchState == MatchState::WaitingToStart)
            TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
        else if (MatchState == MatchState::InProgress)
            TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
        else if (MatchState == MatchState::Cooldown)
            TimeLeft = WarmupTime + MatchTime + CooldownTime - GetServerTime() + LevelStartingTime;
    }

    int32 CeiledTime = FMath::CeilToInt32(TimeLeft);

    if (CountdownInt != CeiledTime)
    {
        if (MatchState == MatchState::WaitingToStart)
            SendWarmupCountdownHUDUpdate(TimeLeft);
        else if (MatchState == MatchState::InProgress)
            SendMatchCountdownHUDUpdate(TimeLeft);
        else if (MatchState == MatchState::Cooldown)
            SendCooldownCountdownHUDUpdate(TimeLeft);
    }

    CountdownInt = CeiledTime;
}