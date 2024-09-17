// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Particles/ParticleSystemComponent.h"
#include "MultiplayerShooter/Character/ShooterCharacter.h"

void AShotgun::Fire(const FVector& HitLocation)
{
	AWeapon::Fire(HitLocation);

	UWorld* World = GetWorld();
	APawn* ShooterPawn = Cast<APawn>(GetOwner());
	AController* InstigatorController = ShooterPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = WeaponMesh->GetSocketByName(FName("MuzzleFlash"));
	if (MuzzleFlashSocket != nullptr && ShooterPawn != nullptr)
	{
		FTransform MuzzleFlashTransform = MuzzleFlashSocket->GetSocketTransform(WeaponMesh);

		FHitResult HitResult;
		FVector Start(MuzzleFlashTransform.GetLocation());
		TMap<AShooterCharacter*, uint32> HitsMap;
		for (uint32 i = 0; i < PelletsCount; i++) // 10
		{
			WeaponTraceHit(Start, HitLocation, HitResult);
			if (HitResult.bBlockingHit)
			{
				if (HasAuthority())
				{
					AShooterCharacter* ShooterChar = Cast<AShooterCharacter>(HitResult.GetActor());
					// Apply Damage
					if (ShooterChar != nullptr)
					{
						if (HitsMap.Contains(ShooterChar))
						{
							HitsMap[ShooterChar]++;
						}
						else 
						{
							HitsMap.Emplace(ShooterChar, 1);
						}
					}
				}

				// Spawn Impact VFX
				if (ImpactVFX != nullptr)
				{
					UGameplayStatics::SpawnEmitterAtLocation(
						World,
						ImpactVFX,
						HitResult.ImpactPoint,
						HitResult.ImpactNormal.Rotation());
				}

				if (HitSFX != nullptr)
				{
					UGameplayStatics::PlaySoundAtLocation(
						this,
						HitSFX,
						HitResult.ImpactPoint,
						.5f,
						FMath::FRandRange(-.5f, .5f)
					);
				}
			}
		}

		if (HasAuthority())
		{
			for (auto HitPair : HitsMap)
			{
				if (HitPair.Key != nullptr && InstigatorController != nullptr)
				{
					UGameplayStatics::ApplyDamage(
						HitPair.Key,
						Damage * HitPair.Value,
						InstigatorController,
						this,
						UDamageType::StaticClass()
					);
				}
			}
		}
	}
}