// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ShooterPlayerController.generated.h"

UCLASS()
class MULTIPLAYERSHOOTER_API AShooterPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void Tick(float DeltaSeconds) override;
	virtual void ReceivedPlayer() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SendHealthHUDUpdate(float Health, float MaxHealth);
	void SendShieldHUDUpdate(float Shield, float MaxShield);

	void SendScoreHUDUpdate(float Score);
	void SendDefeatsHUDUpdate(int32 Defeat);
	
	void SendWeaponAmmoHUDUpdate(int32 Ammo);
	void SendCarriedAmmoHUDUpdate(int32 Ammo);
	void SendGrenadesHUDUpdate(int32 Grenades);
	
	void SendMatchCountdownHUDUpdate(float TotalSeconds);
	void SendWarmupCountdownHUDUpdate(float TotalSeconds);
	void SendCooldownCountdownHUDUpdate(float TotalSeconds);
	
	void OnMatchStateSet(FName NewState);
	float GetServerTime();
	
protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* aPawn) override;

	UFUNCTION()
	void OnRep_MatchState();

	UFUNCTION(Server, Reliable)
	void ClientRequestServerTime(float ClientRequestTime);

	UFUNCTION(Client, Reliable)
	void ServerResponseServerTime(float ClientRequestTime, float ServersTimeOfReceipt);

	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
	void ClientJoinMidGame(FName StateOfMatch, float RemWarmupTime, float RemMatchTime, float RemCooldownTime, float RemStartTime);

private:
	UPROPERTY(EditAnywhere, Category = "Time")
	float SyncTimerDelay = 5.f;

	UPROPERTY()
	class AShooterHUD* ShooterHUD;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

	UPROPERTY()
	class AShooterGameMode* MainShooterGameMode;

	bool bInitializeCharacterOverlay;

	float MatchTime = 0.f;
	float WarmupTime = 0.f;
	float CooldownTime = 0.f;
	float LevelStartingTime = 0.f;

	uint32 CountdownInt = 0;

	float CachedHealth, CachedMaxHealth;
	float CachedShield, CachedMaxShield;
	int32 CachedDefeats, CachedScore, CachedGrenades;
	bool bInitializeHealth = false, bInitializeShield = false;
	bool bInitializeScore = false, bInitializeDefeats = false, bInitializeGrenades = false;

	float ClientServerDelta;
	float SyncTimer = 0;

	void PollInit();
	void SetHUDCountdown();
	void CheckTimeSync(float DeltaSeconds);
	void HandleMatchStart();
	void HandleCooldown();
};
