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

    USkeletalMeshComponent *WeaponSKM = GetWeaponMesh();
    const USkeletalMeshSocket *MuzzleFlashSocket = WeaponSKM->GetSocketByName(FName("MuzzleFlash"));
    APawn *InstigatorPawn = Cast<APawn>(GetOwner());
    UE_LOG(LogTemp, Warning, TEXT(":: ProjectileWeapon :: Fire"));

    if (MuzzleFlashSocket)
    {
        FTransform MuzzleTransform = MuzzleFlashSocket->GetSocketTransform(WeaponSKM);
        FVector SpawnDirection = HitLocation - MuzzleTransform.GetLocation();
        FRotator SpawnRotation = SpawnDirection.Rotation();
        UE_LOG(LogTemp, Warning, TEXT(":: ProjectileWeapon :: MuzzleFlashSocket"));

        if (ProjectileAmmo && InstigatorPawn)
        {
            FActorSpawnParameters SpawnParameters;
            SpawnParameters.Owner = GetOwner();
            SpawnParameters.Instigator = InstigatorPawn;
            UE_LOG(LogTemp, Warning, TEXT(":: ProjectileWeapon :: SpawnActor"));
            GetWorld()->SpawnActor<AProjectileAmmo>(
                ProjectileAmmo,
                MuzzleTransform.GetLocation(),
                SpawnRotation,
                SpawnParameters);
        }
    }
}
