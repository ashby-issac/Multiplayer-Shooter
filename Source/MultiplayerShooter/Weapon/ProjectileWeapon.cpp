// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectileWeapon.h"
#include "Components/BoxComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "ProjectileAmmo.h"

AProjectileWeapon::AProjectileWeapon()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AProjectileWeapon::BeginPlay()
{
    Super::BeginPlay();
}

void AProjectileWeapon::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AProjectileWeapon::Fire(const FVector &HitLocation)
{
    Super::Fire(HitLocation);

    if (!HasAuthority())
    {
        return;
    }

    USkeletalMeshComponent *WeaponSKM = GetWeaponMesh();
    const USkeletalMeshSocket *MuzzleFlashSocket = WeaponSKM->GetSocketByName(FName("MuzzleFlash"));
    APawn *InstigatorPawn = Cast<APawn>(GetOwner());

    if (MuzzleFlashSocket)
    {
        FTransform MuzzleTransform = MuzzleFlashSocket->GetSocketTransform(WeaponSKM);
        FVector SpawnDirection = HitLocation - MuzzleTransform.GetLocation();
        FRotator SpawnRotation = SpawnDirection.Rotation();

        if (ProjectileAmmo && InstigatorPawn)
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = GetOwner();
            SpawnParams.Instigator = InstigatorPawn;

            GetWorld()->SpawnActor<AProjectileAmmo>(
                ProjectileAmmo,
                MuzzleTransform.GetLocation(),
                SpawnRotation,
                SpawnParams);
        }
    }
}
