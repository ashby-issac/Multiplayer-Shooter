// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"
#include "Sound/SoundCue.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "RocketMovementComponent.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"

AProjectileRocket::AProjectileRocket()
{
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovementComponent = CreateDefaultSubobject<URocketMovementComponent>(TEXT("RocketMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true); 
}

void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority()) // for clients
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &ThisClass::OnProjectileHit);
	}

	SpawnTrialSystem();
	SpawnTrialSFX();
}

void AProjectileRocket::Destroyed()
{
	// Delaying the destruction of the rocket projectile
	// to play the smoke effects
}

void AProjectileRocket::OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor == GetOwner())
	{
		return;
	}

	ApplyProjectileRadialDamage();
	StartDestroyTimer();

	if (ImpactParticles != nullptr)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}

	if (ImpactSound != nullptr)
	{
		UGameplayStatics::SpawnSoundAtLocation(this, ImpactSound, GetActorLocation());
	}

	if (ProjectileMesh != nullptr)
	{
		ProjectileMesh->SetVisibility(false);
	}
	
	if (CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (TrailSystemComponent != nullptr && TrailSystemComponent->GetSystemInstanceController() != nullptr)
	{
		TrailSystemComponent->GetSystemInstanceController()->Deactivate();
	}

	if (TrailComponent != nullptr && TrailComponent->IsPlaying())
	{
		TrailComponent->Stop();
	}
}
