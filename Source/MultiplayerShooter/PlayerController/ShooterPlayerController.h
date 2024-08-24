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
	void SendHealthHUDUpdate(float Health, float MaxHealth);
	void SendScoreHUDUpdate(float Score);
	void SendDefeatsHUDUpdate(int32 Defeat);

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* aPawn) override;

private:
	class AShooterHUD* ShooterHUD;
};
