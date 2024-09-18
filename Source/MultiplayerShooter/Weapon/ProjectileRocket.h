// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectileAmmo.h"
#include "ProjectileRocket.generated.h"

UCLASS()
class MULTIPLAYERSHOOTER_API AProjectileRocket : public AProjectileAmmo
{
	GENERATED_BODY()
	
public:
	AProjectileRocket();
	virtual void Destroyed() override;

protected:
	virtual void BeginPlay() override;
	virtual void OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

};
