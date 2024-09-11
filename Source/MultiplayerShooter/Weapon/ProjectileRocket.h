// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectileAmmo.h"
#include "ProjectileRocket.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSHOOTER_API AProjectileRocket : public AProjectileAmmo
{
	GENERATED_BODY()
	
public:
	AProjectileRocket();

protected:
	virtual void OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

private:
	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* RocketMesh;

	UPROPERTY(EditAnywhere)
	float MinDamage = 10.f;

	UPROPERTY(EditAnywhere)
	float DamageInnerRadius = 200.f;

	UPROPERTY(EditAnywhere)
	float DamageOuterRadius = 500.f;
	
	UPROPERTY(EditAnywhere)
	float DamageFalloff = 1.f;
};
