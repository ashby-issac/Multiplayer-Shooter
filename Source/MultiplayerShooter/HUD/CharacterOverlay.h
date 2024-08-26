// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSHOOTER_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()

public:
	void UpdateHealthInfo(float Health, float MaxHealth);
	void UpdateScoreValue(float Score);
	void UpdateDefeatValue(int32 Defeat);
	void UpdateWeaponAmmoValue(int32 Ammo);

private:
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthBar;
	
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HealthPercent;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ScoreValue;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DefeatsValue;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* WeaponAmmoValue;
};
