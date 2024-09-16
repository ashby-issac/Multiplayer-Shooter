// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitScanWeapon.generated.h"

class UParticleSystem;

UCLASS()
class MULTIPLAYERSHOOTER_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()
	
public:
	virtual void Fire(const FVector& HitLocation) override;

private:
	UPROPERTY(EditAnywhere)
	float Damage = 7.f;

	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactVFX;

	UPROPERTY(EditAnywhere)
	UParticleSystem* BeamVFX; // Bullet Trial
};
