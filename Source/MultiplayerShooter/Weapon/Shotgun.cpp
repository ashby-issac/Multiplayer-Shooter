// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Particles/ParticleSystemComponent.h"

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
		FVector End;

		for (uint32 i = 0; i < PelletsCount; i++)
		{
			End = TraceEndWithScatter(Start, HitLocation);
		}
	}
}