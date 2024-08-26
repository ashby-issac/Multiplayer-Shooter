// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "ShooterPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSHOOTER_API AShooterPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	virtual void OnRep_Score() override;
	UFUNCTION()
	virtual void OnRep_Defeat();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;

	void AddToScore(float ScoreAmt);
	void AddToDefeats(int32 DefeatAmt);

private:
	UPROPERTY()
	class AShooterCharacter* ShooterCharacter;

	UPROPERTY()
	class AShooterPlayerController* ShooterPlayerController;

	UPROPERTY(ReplicatedUsing = OnRep_Defeat)
	int32 Defeats;

	void UpdateScoreHUD();
	void UpdateDefeatsHUD();
};
