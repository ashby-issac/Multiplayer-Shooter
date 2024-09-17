// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Sound/SoundCue.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Particles/ParticleSystemComponent.h"
#include "MultiplayerShooter/Weapon/WeaponTypes.h"
#include "MultiplayerShooter/Interfaces/CrosshairsInteractor.h"

void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector HitLocation, FHitResult& OutHit)
{
	UWorld* World = GetWorld();

	FVector Start(TraceStart);
	FVector End = bUseScatter ? TraceEndWithScatter(Start, HitLocation) : Start + (HitLocation - Start) * 1.25f;
	FVector BeamEnd = End;

	if (World != nullptr)
	{
		World->LineTraceSingleByChannel(
			OutHit,
			Start,
			End,
			ECollisionChannel::ECC_Visibility);

		if (OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;
		}

		if (BeamVFX != nullptr)
		{
			UParticleSystemComponent* BeamParticle = UGameplayStatics::SpawnEmitterAtLocation(
				World,
				BeamVFX,
				Start);

			BeamParticle->SetVectorParameter(FName("Target"), BeamEnd);
		}
	}
}

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

		WeaponTraceHit(MuzzleFlashTransform.GetLocation(), HitLocation, HitResult);

		if (HitResult.bBlockingHit)
		{
			if (HasAuthority() && InstigatorController && HitResult.GetActor()->Implements<UCrosshairsInteractor>())
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

			if (HitSFX != nullptr)
			{
				UGameplayStatics::PlaySoundAtLocation(
					this,
					HitSFX,
					HitResult.ImpactPoint
				);
			}
		}

		// Fire Animation Alternate logic
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


FVector AHitScanWeapon::TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget)
{
	FVector TargetDirNorm = (HitTarget - TraceStart).GetSafeNormal();
	FVector SphereCenter = TraceStart + TargetDirNorm * DistanceToSphere;


	FVector ScatterPoint = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	FVector ScatterEndPoint = SphereCenter + ScatterPoint;
	FVector ScatterVector = (ScatterEndPoint - TraceStart);

	/*DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);
	DrawDebugSphere(GetWorld(), ScatterEndPoint, 10.f, 12, FColor::Cyan, true);
	DrawDebugLine(GetWorld(), TraceStart, TraceStart + ScatterVector.GetSafeNormal() * TRACE_LENGTH, FColor::Green, true);*/

	return  TraceStart + ScatterVector.GetSafeNormal() * TRACE_LENGTH;
}