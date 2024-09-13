// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"

void AHitScanWeapon::Fire(const FVector& HitLocation)
{
	Super::Fire(HitLocation);

	UWorld* World = GetWorld();
	APawn* ShooterPawn = Cast<APawn>(GetOwner());

	const USkeletalMeshSocket* MuzzleFlashSocket = WeaponMesh->GetSocketByName(FName("MuzzleFlash"));
	if (MuzzleFlashSocket != nullptr && ShooterPawn != nullptr)
	{
		FTransform MuzzleFlashTransform = MuzzleFlashSocket->GetSocketTransform(WeaponMesh);

		FHitResult HitResult;
		FVector Start(MuzzleFlashTransform.GetLocation());
		FVector End(Start + (HitLocation - Start) * 1.25f);

		if (World != nullptr)
		{
			World->LineTraceSingleByChannel(
				HitResult,
				Start,
				End,
				ECollisionChannel::ECC_Visibility);
			if (HitResult.bBlockingHit)
			{
				if (HasAuthority())
				{
					UGameplayStatics::ApplyDamage(
						HitResult.GetActor(),
						Damage,
						ShooterPawn->GetController(),
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
			}
		}
	}

	

	
}
