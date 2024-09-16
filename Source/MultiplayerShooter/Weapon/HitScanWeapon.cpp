// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Particles/ParticleSystemComponent.h"

void AHitScanWeapon::Fire(const FVector& HitLocation)
{
	Super::Fire(HitLocation);

	UWorld* World = GetWorld();
	APawn* ShooterPawn = Cast<APawn>(GetOwner());
	AController* InstigatorController = ShooterPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = WeaponMesh->GetSocketByName(FName("MuzzleFlash"));
	if (MuzzleFlashSocket != nullptr && ShooterPawn != nullptr)
	{
		FTransform MuzzleFlashTransform = MuzzleFlashSocket->GetSocketTransform(WeaponMesh);

		FHitResult HitResult;
		FVector Start(MuzzleFlashTransform.GetLocation());
		FVector End(Start + (HitLocation - Start) * 1.25f);
		FVector BeamEnd(End);

		if (World != nullptr)
		{
			World->LineTraceSingleByChannel(
				HitResult,
				Start,
				End,
				ECollisionChannel::ECC_Visibility);

			if (HitResult.bBlockingHit)
			{
				if (HasAuthority() && InstigatorController)
				{
					UGameplayStatics::ApplyDamage(
						HitResult.GetActor(),
						Damage,
						InstigatorController,
						this,
						UDamageType::StaticClass()
					);
				}

				if (ImpactVFX != nullptr)
				{
					UGameplayStatics::SpawnEmitterAtLocation(
						World,
						ImpactVFX,
						HitResult.ImpactPoint,
						HitResult.ImpactNormal.Rotation());
				}

				if (BeamVFX != nullptr)
				{
					BeamEnd = HitResult.ImpactPoint;
					UParticleSystemComponent* BeamParticle = UGameplayStatics::SpawnEmitterAtLocation(
																World, 
																BeamVFX, 
																MuzzleFlashTransform);
					BeamParticle->SetVectorParameter(FName("Target"), BeamEnd);
				}

				if (HitSFX != nullptr)
				{
					UGameplayStatics::PlaySoundAtLocation(
						World,
						HitSFX,
						HitResult.ImpactPoint
					);
				}
			}

			if (WeaponMesh != nullptr)
			{
				if (MuzzleSFX != nullptr)
				{
					UGameplayStatics::PlaySoundAtLocation(
						World, 
						MuzzleSFX, 
						MuzzleFlashTransform.GetLocation()
					);
				}

				if (MuzzleVFX != nullptr)
				{
					UGameplayStatics::SpawnEmitterAtLocation(
						World,
						MuzzleVFX,
						MuzzleFlashTransform
					);
				}
			}
		}
	}

	

	
}
