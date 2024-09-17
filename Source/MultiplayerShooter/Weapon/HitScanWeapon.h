// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitScanWeapon.generated.h"

class UParticleSystem;
class USoundCue;

UCLASS()
class MULTIPLAYERSHOOTER_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()
	
public:
	virtual void Fire(const FVector& HitLocation) override;

protected:
	UPROPERTY(EditAnywhere)
	float Damage = 7.f;

	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactVFX;

	UPROPERTY(EditAnywhere)
	USoundCue* HitSFX;

	FVector TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget);
	void WeaponTraceHit(const FVector& TraceStart, const FVector HitLocation, FHitResult& OutHit);

private:
	UPROPERTY(EditAnywhere)
	UParticleSystem* BeamVFX;

	UPROPERTY(EditAnywhere, Category = "FireAnimation Alternate")
	UParticleSystem* MuzzleVFX;

	UPROPERTY(EditAnywhere, Category = "FireAnimation Alternate")
	USoundCue* MuzzleSFX;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter Props")
	float DistanceToSphere;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter Props")
	float SphereRadius;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter Props")
	bool bUseScatter = false;
};
