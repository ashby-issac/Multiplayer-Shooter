// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"

AProjectileRocket::AProjectileRocket()
{
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
	RocketMesh->SetupAttachment(RootComponent);
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AProjectileRocket::OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	APawn* FiringPawn = Cast<APawn>(GetOwner());
	if (FiringPawn != nullptr)
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
			if (GEngine != nullptr)
			{
				GEngine->AddOnScreenDebugMessage(
					-1,
					15.f,
					FColor::Green,
					FString::Printf(TEXT("Applying radial damage"))
				);
			}
		}
	}

	Super::OnProjectileHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);
}
