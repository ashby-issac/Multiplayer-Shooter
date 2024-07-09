// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectileAmmo.generated.h"

UCLASS()
class MULTIPLAYERSHOOTER_API AProjectileAmmo : public AActor
{
	GENERATED_BODY()

public:
	AProjectileAmmo();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(EditAnywhere)
	class UBoxComponent *CollisionBox;

	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* ProjectileTrace;

	UPROPERTY(EditAnywhere)
	class UParticleSystemComponent* ProjectileTraceComponent;
};
