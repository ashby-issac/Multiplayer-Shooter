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

	void SendHealthHUDUpdate(float Health, float MaxHealth);
	void SendScoreHUDUpdate(float Score);
	void SendDefeatsHUDUpdate(int32 Defeat);
	void SendWeaponAmmoHUDUpdate(int32 Ammo);
	void SendCarriedAmmoHUDUpdate(int32 Ammo);
	void SendMatchCountdownHUDUpdate(float TotalSeconds);

	float GetServerTime();
	
protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* aPawn) override;

	UFUNCTION(Server, Reliable)
	void ClientRequestServerTime(float ClientRequestTime);

	UFUNCTION(Client, Reliable)
	void ServerResponseServerTime(float ClientRequestTime, float ServersTimeOfReceipt);

private:
	UPROPERTY(EditAnywhere, Category = "Time")
	float SyncTimerDelay = 5.f;

	UPROPERTY()
	class AShooterHUD* ShooterHUD;

	uint32 MatchTimer = 120;
	uint32 CountdownInt = 0;

	float ClientServerDelta;
	float SyncTimer = 0;

	void SetHUDCountdown();
	void CheckTimeSync(float DeltaSeconds);
};
