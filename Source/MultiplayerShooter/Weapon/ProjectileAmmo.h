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
	FTimerHandle DestroyTimer;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* TrailSystem;

	UPROPERTY()
	class UNiagaraComponent* TrailSystemComponent;

	UPROPERTY(EditAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ProjectileMesh;

	UPROPERTY()
	class UAudioComponent* TrailComponent;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;

	UPROPERTY(EditAnywhere)
	float Damage;

	UPROPERTY(EditAnywhere)
	float DestroyDelay = 3.f;

	UPROPERTY(EditAnywhere)
	class USoundCue* TrailSFX;

	UPROPERTY(EditAnywhere)
	USoundCue* ImpactSound;

	UPROPERTY(EditAnywhere)
	class USoundAttenuation* TrailAttenuation;

	UPROPERTY(EditAnywhere)
	float DamageInnerRadius = 200.f;

	UPROPERTY(EditAnywhere)
	float DamageOuterRadius = 500.f;

	virtual void BeginPlay() override;

	void SpawnTrialSystem();
	void SpawnTrialSFX();
	void ApplyProjectileRadialDamage();
	void StartDestroyTimer();
	void OnDestroyProjectile();

	UFUNCTION()
	virtual void OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

public:
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

private:
	UPROPERTY(EditAnywhere)
	float MinDamage = 10.f;

	UPROPERTY(EditAnywhere)
	float DamageFalloff = 1.f;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* ProjectileTrace;

	UPROPERTY(EditAnywhere)
	class UParticleSystemComponent* ProjectileTraceComponent;

};
