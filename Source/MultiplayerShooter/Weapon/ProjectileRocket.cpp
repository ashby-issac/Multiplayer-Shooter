// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"
#include "Sound/SoundCue.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "RocketMovementComponent.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"

AProjectileRocket::AProjectileRocket()
{
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
	RocketMesh->SetupAttachment(RootComponent);
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovementComponent = CreateDefaultSubobject<URocketMovementComponent>(TEXT("RocketMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true); // Set in BP
}

void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority()) // for clients
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &ThisClass::OnProjectileHit);
	}

	if (TrailSystem != nullptr)
	{
		TrailSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
							   TrailSystem,
							   GetRootComponent(),
							   FName(),
							   GetActorLocation(),
							   GetActorRotation(),
							   EAttachLocation::KeepWorldPosition,
							   false);
	}

	if (TrailSFX != nullptr && TrailAttenuation != nullptr)
	{
		TrailComponent = UGameplayStatics::SpawnSoundAttached(
						 TrailSFX,
						 GetRootComponent(),
						 FName(),
						 GetActorLocation(),
						 GetActorRotation(),
						 EAttachLocation::KeepWorldPosition,
						 false,
						 1.f,
						 1.f,
						 0.f,
						 TrailAttenuation,
						 (USoundConcurrency*)nullptr,
						 false);
	}
}

void AProjectileRocket::OnDestroyRocket()
{
	Destroy();
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
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Green,
				FString::Printf(TEXT("Hit Owner"))
			);
		}
		return;
	}

	APawn* FiringPawn = GetInstigator();
	if (FiringPawn != nullptr && HasAuthority())
	{
		AController* FiringController = FiringPawn->GetController();
		if (FiringController != nullptr)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this,
				Damage,
				MinDamage,
				GetActorLocation(),
				DamageInnerRadius,
				DamageOuterRadius,
				DamageFalloff,
				UDamageType::StaticClass(),
				TArray<AActor*>(),
				this,
				FiringController
			);
		}
	}

	GetWorldTimerManager().SetTimer(
		DestroyTimer,
		this,
		&ThisClass::OnDestroyRocket,
		DestroyDelay);

	if (ImpactParticles != nullptr)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}

	if (ImpactSound != nullptr)
	{
		UGameplayStatics::SpawnSoundAtLocation(this, ImpactSound, GetActorLocation());
	}

	if (RocketMesh != nullptr)
	{
		RocketMesh->SetVisibility(false);
	}
	
	if (CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (TrailSystemComponent != nullptr && TrailSystemComponent->GetSystemInstance() != nullptr)
	{
		TrailSystemComponent->GetSystemInstance()->Deactivate();
	}

	if (TrailComponent != nullptr && TrailComponent->IsPlaying())
	{
		TrailComponent->Stop();
	}
}
